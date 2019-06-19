#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <lstate.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>


#include "profiler.h"


typedef struct context {
	lua_Alloc alloc;
	void* ud;
	int hook_c;
	double time_start;
	kstring_t name_cached;
	struct co* co_ctx;
	lua_State* pool;
	hash_frame_t* hash;
	struct frame* freelist;
} context_t;

typedef struct co {
	frame_t* head;
	frame_t* tail;

	struct co* next;
	struct co* prev;

	int alloc_total;
	int alloc_count;
} co_t;

double
get_time() {
	struct timeval tv;

#ifdef _WIN32

#define EPOCH_BIAS (116444736000000000)
#define UNITS_PER_SEC (10000000)
#define USEC_PER_SEC (1000000)
#define UNITS_PER_USEC (10)

	union {
		FILETIME ft_ft;
		uint64_t ft_64;
	} ft;

	GetSystemTimeAsFileTime(&ft.ft_ft);

	if ( ft.ft_64 < EPOCH_BIAS ) {
		return -1;
	}
	ft.ft_64 -= EPOCH_BIAS;
	tv.tv_sec = (long)( ft.ft_64 / UNITS_PER_SEC );
	tv.tv_usec = (long)( ( ft.ft_64 / UNITS_PER_USEC ) % USEC_PER_SEC );
#else
	gettimeofday(&tv, NULL);
#endif

	return (double)tv.tv_sec * 1000  + (double)tv.tv_usec / 1000;
}


static void *
lalloc (void *ud, void *ptr, size_t osize, size_t nsize) {
	context_t* ctx = ud;
	if (nsize != 0 && nsize > osize) {
		ctx->co_ctx->alloc_count++;
		ctx->co_ctx->alloc_total += nsize - osize;
	}
	return ctx->alloc(ctx->ud, ptr, osize, nsize);
}

static const char*
create_string(context_t* profiler, const char* str) {
	if (str == NULL)
		return str;

	lua_getfield(profiler->pool, 1, str);
	if ( !lua_isnil(profiler->pool, 2) ) {
		char* result = lua_touserdata(profiler->pool, -1);
		lua_pop(profiler->pool, 1);
		return result;
	}
	lua_pop(profiler->pool, 1);

	lua_pushstring(profiler->pool, str);
	char* ptr = (char*)lua_tostring(profiler->pool, -1);
	lua_pushlightuserdata(profiler->pool, ptr);
	lua_settable(profiler->pool, 1);
	return ptr;
}

static inline frame_t*
frame_create(context_t* ctx, const char* source, const char* name, int linedefined) {
	frame_t* frame = NULL;
	if ( ctx->freelist) {
		frame = ctx->freelist;
		ctx->freelist = frame->free_next;
	} else {
		frame = malloc(sizeof( *frame ));
	}
	
	memset(frame, 0, sizeof( *frame ));

	if (name) {
		frame->name = create_string(ctx, name);
	}
	frame->source = create_string(ctx, source);
	frame->linedefined = linedefined;

	return frame;
}

static inline void
frame_delete(context_t* ctx,frame_t* frame) {
	frame->free_next = ctx->freelist;
	ctx->freelist = frame;
}

void 
frame_push(co_t* co_ctx, context_t* profiler, const char* name, const char* source, int linedefined) {
	double now = get_time();

	frame_t* frame = frame_create(profiler, source, name, linedefined);

	frame->alloc_count = co_ctx->alloc_count;
	frame->alloc_total = co_ctx->alloc_total;

	frame->invoke_diff = now;
	frame->invoke_cost = 0;
	frame->invoke_start = now;
	
	frame->next = frame->prev = NULL;

	if (co_ctx->head == NULL) {
		assert(co_ctx->head == co_ctx->tail);
		co_ctx->head = co_ctx->tail = frame;
	}
	else {
		co_ctx->tail->next = frame;
		frame->prev = co_ctx->tail;
		co_ctx->tail = frame;
	}

	frame_t* prev_frame = frame->prev;
	if (prev_frame) {
		prev_frame->invoke_cost += now - prev_frame->invoke_start;
	}
}

void
frame_pop(co_t* co_ctx, context_t* profiler) {
	assert(co_ctx->head != NULL);
	assert(co_ctx->tail != NULL);
	
	frame_t* frame = NULL;
	while (co_ctx->tail) {
		frame = co_ctx->tail;
		assert(frame != NULL);

		if ( frame == co_ctx->head ) {
			co_ctx->head = co_ctx->tail = NULL;
		}
		else {
			co_ctx->tail = frame->prev;
			co_ctx->tail->next = NULL;
		}
		break;
	}

	frame->alloc_count = co_ctx->alloc_count - frame->alloc_count;
	frame->alloc_total = co_ctx->alloc_total - frame->alloc_total;

	profiler->name_cached.l = 0;
	kputs(frame->source, &profiler->name_cached);
	kputc_(':', &profiler->name_cached);
	kputw(frame->linedefined, &profiler->name_cached);
	if (frame->name) {
		kputc_(':', &profiler->name_cached);
		kputs(frame->name, &profiler->name_cached);
	}

	frame_t* fm = hash_frame_find(profiler->hash, profiler->name_cached.s);
	if (fm == NULL) {
		fm = frame_create(profiler, frame->source, frame->name, frame->linedefined);
		hash_frame_set(profiler->hash, profiler->name_cached.s, fm);
	}

	double now = get_time();

	fm->invoke_count++;

	fm->alloc_count += frame->alloc_count;
	fm->alloc_total += frame->alloc_total;
	
	fm->invoke_cost += frame->invoke_cost;
	fm->invoke_cost += now - frame->invoke_start;
	fm->invoke_diff += now - frame->invoke_diff;

	frame_t* prev_frame = frame->prev;
	if (prev_frame) {
		prev_frame->invoke_start = now;
	}

	frame_delete(profiler, frame);
}

static void 
lhook (lua_State *L, lua_Debug *ar) {
	context_t* ctx;
	lua_getallocf(L, (void**)&ctx);
	co_t* co_ctx = ctx->co_ctx;
	if ( co_ctx->head == NULL ) {
		if ( ar->event == LUA_HOOKRET )
			return;
	}
	
	lua_getinfo(L, "nS", ar);
	if ( ctx->hook_c == 0 && strcmp(ar->what,"C") == 0) {
		return;
	}

	switch(ar->event) {
		case LUA_HOOKCALL: {
			frame_push(co_ctx, ctx, ar->name, ar->source, ar->linedefined);
			break;
		}
		case LUA_HOOKRET: {
			frame_pop(co_ctx, ctx);
			break;
		}
		default: {
			break;
		}
	}
}

static int 
lco_create(lua_State* L) {
	lua_CFunction co_create = lua_tocfunction(L, lua_upvalueindex(1));
	int status = co_create(L);
	lua_State* co = lua_tothread(L, -1);
	lua_sethook(co, lhook, LUA_MASKCALL | LUA_MASKRET, 0);

	co_t* co_ctx = malloc(sizeof( *co_ctx ));
	memset(co_ctx, 0, sizeof( *co_ctx ));
	lua_getfield(L, LUA_REGISTRYINDEX, "profiler");
	lua_pushvalue(L, -2);
	lua_pushlightuserdata(L, co_ctx);
	lua_settable(L, -3);

	lua_pop(L, 1);

	return status;
}

static int
lco_resume(lua_State* L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "profiler");

	lua_pushvalue(L, 1);
	lua_gettable(L, -2);
	co_t* next_co_ctx = lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushthread(L);
	lua_gettable(L, -2);
	co_t* current_co_ctx = lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pop(L, 1);

	current_co_ctx->next = next_co_ctx;
	next_co_ctx->prev = current_co_ctx;

	context_t* ctx;
	lua_getallocf(L, (void**)&ctx);
	ctx->co_ctx = next_co_ctx;

	lua_CFunction co_resume = lua_tocfunction(L, lua_upvalueindex(1));
	return co_resume(L);
}

static int
lco_yield(lua_State* L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "profiler");
	lua_pushthread(L);
	lua_gettable(L, -2);
	co_t* co_ctx = lua_touserdata(L, -1);
	lua_pop(L, 2);

	context_t* ctx;
	lua_getallocf(L, (void**)&ctx);

	co_t* prev_co_ctx = co_ctx->prev;
	co_ctx->prev = NULL;
	prev_co_ctx->next = NULL;
	ctx->co_ctx = prev_co_ctx;

	lua_CFunction co_yield = lua_tocfunction(L, lua_upvalueindex(1));
	return co_yield(L);
}

static int
lstop(lua_State* L) {
	context_t* ctx = lua_touserdata(L, lua_upvalueindex(1));
	lua_setallocf(L, ctx->alloc, ctx->ud);

	lua_getglobal(L, "coroutine");

	lua_pushvalue(L, lua_upvalueindex(2));
	lua_setfield(L, -2, "create");

	lua_pushvalue(L, lua_upvalueindex(3));
	lua_setfield(L, -2, "resume");

	lua_pushvalue(L, lua_upvalueindex(4));
	lua_setfield(L, -2, "yield");

	lua_getfield(L, LUA_REGISTRYINDEX, "profiler");
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)	{
		lua_State* co = lua_tothread(L, -2);
		co_t* co_ctx = lua_touserdata(L, -1);

		frame_t* cursor = co_ctx->head;
		while ( cursor ) {
			frame_t* tmp = cursor;
			cursor = cursor->free_next;
			free(tmp);
		}

		free(co_ctx);
		lua_sethook(co, NULL, 0, 0);
		lua_pop(L, 1);
	}

	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "profiler");

	frame_t* cursor = ctx->freelist;
	while ( cursor ) {
		frame_t* tmp = cursor;
		cursor = cursor->free_next;
		free(tmp);
	}
	
	lua_pushnumber(L, get_time() - ctx->time_start);

	lua_newtable(L);
	const char* name;
	frame_t* fm;
	hash_frame_foreach(ctx->hash, name, fm, {
		lua_newtable(L);

		lua_pushinteger(L, fm->invoke_count);
		lua_setfield(L, -2, "invoke_count");
		lua_pushinteger(L, fm->alloc_count);
		lua_setfield(L, -2, "alloc_count");
		lua_pushinteger(L, fm->alloc_total);
		lua_setfield(L, -2, "alloc_total");
		lua_pushnumber(L, fm->invoke_cost);
		lua_setfield(L, -2, "invoke_cost");
		lua_pushnumber(L, fm->invoke_diff);
		lua_setfield(L, -2, "invoke_diff");

		lua_setfield(L, -2, name);
	});

	hash_frame_free(ctx->hash);
	lua_close(ctx->pool);
	free(ctx->name_cached.s);
	free(ctx);
	return 2;
}

int
lprofiler_start(lua_State* L) {
	if (L != G(L)->mainthread) {
		luaL_error(L, "profiler only start in main coroutine");
	}
	int hook_c = luaL_optinteger(L, 1, 0);

	context_t* ctx = malloc(sizeof(*ctx));
	memset(ctx, 0, sizeof(*ctx));

	ctx->alloc = lua_getallocf(L, &ctx->ud);
	ctx->hook_c = hook_c;
	ctx->time_start = get_time();

	ctx->name_cached.l = 0;
	ctx->name_cached.m = 256;
	ctx->name_cached.s = malloc(ctx->name_cached.m);

	ctx->pool = luaL_newstate();
	luaL_openlibs(ctx->pool);
	lua_settop(ctx->pool, 0);
	lua_newtable(ctx->pool);

	co_t* co_ctx = malloc(sizeof( *co_ctx ));
	memset(co_ctx, 0, sizeof( *co_ctx ));

	ctx->co_ctx = co_ctx;
	lua_setallocf(L, lalloc, ctx);

	ctx->hash = hash_frame_new();

	lua_newtable(L);
	lua_pushthread(L);
	lua_pushlightuserdata(L, co_ctx);
	lua_settable(L, -3);

	lua_setfield(L, LUA_REGISTRYINDEX, "profiler");

	lua_getglobal(L, "coroutine");
	lua_getfield(L, -1, "create");
	lua_CFunction co_create = lua_tocfunction(L, -1);
	lua_pushlightuserdata(L, ctx);
	lua_pushcclosure(L, lco_create, 2);
	lua_setfield(L, -2, "create");
	lua_pop(L, 1);

	lua_getglobal(L, "coroutine");
	lua_getfield(L, -1, "resume");
	lua_CFunction co_resume = lua_tocfunction(L, -1);
	lua_pushlightuserdata(L, ctx);
	lua_pushcclosure(L, lco_resume, 2);
	lua_setfield(L, -2, "resume");
	lua_pop(L, 1);

	lua_getglobal(L, "coroutine");
	lua_getfield(L, -1, "yield");
	lua_CFunction co_yield = lua_tocfunction(L, -1);
	lua_pushlightuserdata(L, ctx);
	lua_pushcclosure(L, lco_yield, 2);
	lua_setfield(L, -2, "yield");
	lua_pop(L, 1);

	lua_pushlightuserdata(L, ctx);
	lua_pushcfunction(L, co_create);
	lua_pushcfunction(L, co_resume);
	lua_pushcfunction(L, co_yield);
	lua_pushcclosure(L, lstop, 4);

	lua_sethook(L, lhook, LUA_MASKCALL | LUA_MASKRET, 0);

	return 1;
}

__declspec( dllexport ) int
luaopen_profiler(lua_State* L) {
	lua_pushcfunction(L, lprofiler_start);
	return 1;
}