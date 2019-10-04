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


typedef struct redisReply {
	int type;
	long long integer;
	double dval;
	size_t len;
	char *str;
	size_t elements;
	struct redisReply **element;
	struct redisReply* next;
} redisReply;

typedef struct lreader {
	redisReader* core;
	redisReply* freelist;
} lreader;


static redisReply *createReplyObject(int type);
static void freeReplyObject(void *reply);
static void *createStringObject(const redisReadTask *task, char *str, size_t len);
static void *createArrayObject(const redisReadTask *task, size_t elements);
static void *createIntegerObject(const redisReadTask *task, long long value);
static void *createDoubleObject(const redisReadTask *task, double value, char *str, size_t len);
static void *createNilObject(const redisReadTask *task);
static void *createBoolObject(const redisReadTask *task, int bval);

static redisReplyObjectFunctions defaultFunctions = {
	createStringObject,
	createArrayObject,
	createIntegerObject,
	createDoubleObject,
	createNilObject,
	createBoolObject,
	freeReplyObject
};

static redisReply *createReplyObject(int type) {
	redisReply *r = calloc(1, sizeof(*r));

	if (r == NULL)
		return NULL;

	r->type = type;
	return r;
}

static void freeReplyObject(void *reply) {
	redisReply *r = reply;
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
					freeReplyObject(r->element[j]);
				free(r->element);
			}
			break;
		case REDIS_REPLY_ERROR:
		case REDIS_REPLY_STATUS:
		case REDIS_REPLY_STRING:
		case REDIS_REPLY_DOUBLE:
			free(r->str);
			break;
	}
	free(r);
}

static void *createStringObject(const redisReadTask *task, char *str, size_t len) {
	redisReply *r, *parent;
	char *buf;

	r = createReplyObject(task->type);
	if (r == NULL)
		return NULL;

	buf = malloc(len + 1);
	if (buf == NULL) {
		freeReplyObject(r);
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

static void *createArrayObject(const redisReadTask *task, size_t elements) {
	redisReply *r, *parent;

	r = createReplyObject(task->type);
	if (r == NULL)
		return NULL;

	if (elements > 0) {
		r->element = calloc(elements, sizeof(redisReply*));
		if (r->element == NULL) {
			freeReplyObject(r);
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

static void *createIntegerObject(const redisReadTask *task, long long value) {
	redisReply *r, *parent;

	r = createReplyObject(REDIS_REPLY_INTEGER);
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

static void *createDoubleObject(const redisReadTask *task, double value, char *str, size_t len) {
	redisReply *r, *parent;

	r = createReplyObject(REDIS_REPLY_DOUBLE);
	if (r == NULL)
		return NULL;

	r->dval = value;
	r->str = malloc(len + 1);
	if (r->str == NULL) {
		freeReplyObject(r);
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

static void *createNilObject(const redisReadTask *task) {
	redisReply *r, *parent;

	r = createReplyObject(REDIS_REPLY_NIL);
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

static void *createBoolObject(const redisReadTask *task, int bval) {
	redisReply *r, *parent;

	r = createReplyObject(REDIS_REPLY_BOOL);
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

static int push_reply(lua_State* L, redisReply* reply) {
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
			lua_pushnil(L);
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
	redisReply* reply = NULL;
	int status = redisReaderGetReply(reader->core, &reply);
	if (status != REDIS_OK) {
		luaL_error(L, "redisReaderGetReply ERROR");
	}
	if (!reply) {
		return 0;
	}

	int n = push_reply(L, reply);
	freeReplyObject(reply);
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
	reader->core->privdata = reader;

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

