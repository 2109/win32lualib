#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "read.h"


typedef struct redis_reply {
	int type;
	long long integer;
	double dval;
	size_t len;
	char *str;
	size_t elements;
	struct redis_reply **element;
	struct redis_reply* next;
} redis_reply;

typedef struct lreader {
	redisReader* core;
	redis_reply* freelist;
} lreader;


static redis_reply *create_reply(int type, void* userdata);
static void free_reply(void *reply, void* userdata);
static void *create_string(const redisReadTask *task, char *str, size_t len);
static void *create_array(const redisReadTask *task, size_t elements);
static void *create_integer(const redisReadTask *task, long long value);
static void *create_double(const redisReadTask *task, double value, char *str, size_t len);
static void *create_nil(const redisReadTask *task);
static void *create_bool(const redisReadTask *task, int bval);

static redisReplyObjectFunctions defaultFunctions = {
	create_string,
	create_array,
	create_integer,
	create_double,
	create_nil,
	create_bool,
	free_reply
};

static redis_reply *create_reply(int type, void* userdata) {
	lreader* reader = (lreader*)userdata;
	redis_reply *r = NULL;
	if (reader->freelist) {
		r = reader->freelist;
		reader->freelist = r->next;
	} else {
		r = calloc(1, sizeof(*r));
	}
	r->next = NULL;
	r->type = type;
	return r;
}

static void free_reply(void *reply, void* userdata) {
	redis_reply *r = reply;
	size_t j;
	if (r == NULL)
		return;

	switch (r->type) {
		case REDIS_REPLY_INTEGER:
			break;
		case REDIS_REPLY_ARRAY:
		case REDIS_REPLY_MAP:
		case REDIS_REPLY_SET:
			if (r->element != NULL) {
				for (j = 0; j < r->elements; j++)
					free_reply(r->element[j], userdata);
				free(r->element);
				r->element = NULL;
			}
			break;
		case REDIS_REPLY_ERROR:
		case REDIS_REPLY_STATUS:
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_DOUBLE:
			free(r->str);
			break;
	}
	lreader* reader = (lreader*)userdata;
	r->next = reader->freelist;
	reader->freelist = r;
}

static void *create_string(const redisReadTask *task, char *str, size_t len) {
	redis_reply *r, *parent;
	char *buf;

	r = create_reply(task->type, task->privdata);
	if (r == NULL)
		return NULL;

	buf = malloc(len + 1);
	if (buf == NULL) {
		free_reply(r, task->privdata);
		return NULL;
	}

	assert(task->type == REDIS_REPLY_ERROR ||
		   task->type == REDIS_REPLY_STATUS ||
		   task->type == REDIS_REPLY_STRING);

	memcpy(buf, str, len);
	buf[len] = '\0';
	r->str = buf;
	r->len = len;

	if (task->parent) {
		parent = task->parent->obj;
		assert(parent->type == REDIS_REPLY_ARRAY ||
			   parent->type == REDIS_REPLY_MAP ||
			   parent->type == REDIS_REPLY_SET);
		parent->element[task->idx] = r;
	}
	return r;
}

static void *create_array(const redisReadTask *task, size_t elements) {
	redis_reply *r, *parent;

	r = create_reply(task->type, task->privdata);
	if (r == NULL)
		return NULL;

	if (elements > 0) {
		r->element = calloc(elements, sizeof(redis_reply*));
		if (r->element == NULL) {
			free_reply(r, task->privdata);
			return NULL;
		}
	}

	r->elements = elements;

	if (task->parent) {
		parent = task->parent->obj;
		assert(parent->type == REDIS_REPLY_ARRAY ||
			   parent->type == REDIS_REPLY_MAP ||
			   parent->type == REDIS_REPLY_SET);
		parent->element[task->idx] = r;
	}
	return r;
}

static void *create_integer(const redisReadTask *task, long long value) {
	redis_reply *r, *parent;

	r = create_reply(REDIS_REPLY_INTEGER, task->privdata);
	if (r == NULL)
		return NULL;

	r->integer = value;

	if (task->parent) {
		parent = task->parent->obj;
		assert(parent->type == REDIS_REPLY_ARRAY ||
			   parent->type == REDIS_REPLY_MAP ||
			   parent->type == REDIS_REPLY_SET);
		parent->element[task->idx] = r;
	}
	return r;
}

static void *create_double(const redisReadTask *task, double value, char *str, size_t len) {
	redis_reply *r, *parent;

	r = create_reply(REDIS_REPLY_DOUBLE, task->privdata);
	if (r == NULL)
		return NULL;

	r->dval = value;
	r->str = malloc(len + 1);
	if (r->str == NULL) {
		free_reply(r, task->privdata);
		return NULL;
	}

	memcpy(r->str, str, len);
	r->str[len] = '\0';

	if (task->parent) {
		parent = task->parent->obj;
		assert(parent->type == REDIS_REPLY_ARRAY ||
			   parent->type == REDIS_REPLY_MAP ||
			   parent->type == REDIS_REPLY_SET);
		parent->element[task->idx] = r;
	}
	return r;
}

static void *create_nil(const redisReadTask *task) {
	redis_reply *r, *parent;

	r = create_reply(REDIS_REPLY_NIL, task->privdata);
	if (r == NULL)
		return NULL;

	if (task->parent) {
		parent = task->parent->obj;
		assert(parent->type == REDIS_REPLY_ARRAY ||
			   parent->type == REDIS_REPLY_MAP ||
			   parent->type == REDIS_REPLY_SET);
		parent->element[task->idx] = r;
	}
	return r;
}

static void *create_bool(const redisReadTask *task, int bval) {
	redis_reply *r, *parent;

	r = create_reply(REDIS_REPLY_BOOL, task->privdata);
	if (r == NULL)
		return NULL;

	r->integer = bval != 0;

	if (task->parent) {
		parent = task->parent->obj;
		assert(parent->type == REDIS_REPLY_ARRAY ||
			   parent->type == REDIS_REPLY_MAP ||
			   parent->type == REDIS_REPLY_SET);
		parent->element[task->idx] = r;
	}
	return r;
}

static int reader_feed(lua_State *L) {
	lreader *reader = (lreader*)lua_touserdata(L, 1);

	const char* str = NULL;
	size_t sz = 0;
	int vt = lua_type(L, 2);
	switch (vt) {
		case LUA_TSTRING:
			str = lua_tolstring(L, 2, &sz);
			break;
		case LUA_TLIGHTUSERDATA:
			str = (const char*)lua_touserdata(L, 2);
			sz = luaL_checkinteger(L, 3);
			break;
		default:
			luaL_error(L, "unknow lua type:%s", lua_typename(L, vt));
	}

	redisReaderFeed(reader->core, str, sz);
	return 0;
}

static int push_reply(lua_State* L, redis_reply* reply) {
	int n = 0;
	switch (reply->type) {
		case REDIS_REPLY_STRING:
			lua_pushlstring(L, reply->str, reply->len);
			n = 1;
			break;
		case REDIS_REPLY_ARRAY:
			lua_createtable(L, reply->elements, 0);
			int i;
			for (i = 0; i < reply->elements; i++) {
				assert(1 == push_reply(L, reply->element[i]));
				lua_seti(L, -2, i + 1);
			}
			n = 1;
			break;
		case REDIS_REPLY_INTEGER:
			lua_pushinteger(L, reply->integer);
			n = 1;
			break;
		case REDIS_REPLY_NIL:
			lua_pushliteral(L, "");
			n = 1;
			break;
		case REDIS_REPLY_STATUS:
			lua_pushboolean(L, 1);
			lua_pushlstring(L, reply->str, reply->len);
			n = 2;
			break;
		case REDIS_REPLY_ERROR:
			lua_pushboolean(L, 0);
			lua_pushlstring(L, reply->str, reply->len);
			n = 2;
			break;
		case REDIS_REPLY_DOUBLE:
			lua_pushnumber(L, reply->dval);
			n = 1;
			break;
		case REDIS_REPLY_BOOL:
			lua_pushboolean(L, reply->integer);
			n = 1;
			break;
		case REDIS_REPLY_MAP:
		case REDIS_REPLY_SET:
		case REDIS_REPLY_ATTR:
		case REDIS_REPLY_PUSH:
		case REDIS_REPLY_BIGNUM:
			n = 0;
			break;
	}
	return n;
}

static int reader_get_reply(lua_State *L) {
	lreader *reader = (lreader*)lua_touserdata(L, 1);
	redis_reply* reply = NULL;
	int status = redisReaderGetReply(reader->core, &reply);
	if (status != REDIS_OK) {
		luaL_error(L, "redisReaderGetReply ERROR");
	}
	if (!reply) {
		return 0;
	}

	int n = push_reply(L, reply);
	free_reply(reply, reader);
	return n;
}

static int reader_release(lua_State *L) {
	lreader *reader = (lreader*)lua_touserdata(L, 1);
	redisReaderFree(reader->core);
	return 0;
}

static int reader_new(lua_State *L) {
	lreader *reader = (lreader*)lua_newuserdata(L, sizeof(*reader));
	memset(reader, 0, sizeof(*reader));
	reader->core = redisReaderCreateWithFunctions(&defaultFunctions);
	redisReaderSetPrivdata(reader->core, reader);

	if (luaL_newmetatable(L, "redis_meta")) {
		const luaL_Reg meta[] = {
			{"feed", reader_feed},
			{"get_reply", reader_get_reply},
			{NULL, NULL},
		};
		luaL_newlib(L, meta);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, reader_release);
		lua_setfield(L, -2, "__gc");
	}

	lua_setmetatable(L, -2);
	return 1;
}


__declspec(dllexport) int luaopen_redis(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{"reader", reader_new},
		{NULL, NULL},
	};

	luaL_newlib(L, l);
	return 1;
}

