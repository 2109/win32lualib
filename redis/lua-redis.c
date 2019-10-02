#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define inline _inline

#define BUFFER_SIZE 1024

#define PBEGIN    1
#define PSTRING   2
#define PARRAY    3

struct write_buffer {
	char *ptr;
	size_t size;
	size_t offset;
	char init[BUFFER_SIZE];
};

struct parser_context {
	char* buff;
	int total;
	int head;
	int tail;

	int phase;
	union {
		struct {
			int size;
		} string;
		struct {
			int count;
			int index;
			union {
				int length;
				int value;
			} reserve;
			short type;
			int ref;
		} array;
	} reply;

	char *tmp;
	int size;
};

static inline lua_Integer tonumber(lua_State *L, char *line) {
	lua_Number i = 0;
	char* ptr = line;
	int negative = 0;
	while (*ptr != 0) {
		if (*ptr == '-') {
			negative = 1;
		}
		else {
			i = (i * 10) + (*ptr - '0');
		}
		ptr++;
	}
	if (negative == 1) {
		i = -i;
	}
	if (line == ptr) {
		luaL_error(L, "parse number error:%s", line);
	}
	return i;
}

static inline void
wb_init(struct write_buffer *buffer) {
	buffer->ptr = buffer->init;
	buffer->size = BUFFER_SIZE;
	buffer->offset = 0;
}

static inline void
wb_reservce(struct write_buffer *buffer, size_t len) {
	if (buffer->offset + len <= buffer->size) {
		return;
	}
	size_t nsize = buffer->size * 2;
	while (nsize < buffer->offset + len) {
		nsize = nsize * 2;
	}
	char *nptr = (char *)malloc(nsize);
	memcpy(nptr, buffer->ptr, buffer->size);
	buffer->size = nsize;

	if (buffer->ptr != buffer->init)
		free(buffer->ptr);
	buffer->ptr = nptr;
}

static inline void
wb_addchar(struct write_buffer *buffer, char c) {
	wb_reservce(buffer, 1);
	buffer->ptr[buffer->offset++] = c;
}

static inline void
wb_addlstring(struct write_buffer *buffer, const char *str, size_t len) {
	wb_reservce(buffer, len);
	memcpy(buffer->ptr + buffer->offset, str, len);
	buffer->offset += len;
}

static inline void
wb_addstring(struct write_buffer *buffer, const char *str) {
	int len = strlen(str);
	wb_addlstring(buffer, str, len);
}

static inline void
wb_addnumber(struct write_buffer *buffer, double d) {
	char str[64] = {0};
	int len = sprintf(str, "%.16g", d);
	wb_addlstring(buffer, str, len);
}

static inline void
wb_release(struct write_buffer *buffer) {
	if (buffer->ptr != buffer->init)
		free(buffer->ptr);
}

static inline int
get_length(struct parser_context* parser) {
	if (parser->head <= parser->tail) {
		return parser->tail - parser->head;
	}
	return parser->tail + parser->total - parser->head;
}

static inline void parser_skip(struct parser_context *parser, int sz) {
	assert(get_length(parser) >= sz);

	parser->head += sz;
	if (parser->head >= parser->total) {
		parser->head -= parser->total;
	}
}

static inline char * parser_read(struct parser_context *parser, int sz) {
	assert(get_length(parser) >= sz);

	if (parser->size < sz + 1) {
		size_t nsize = parser->size * 2;
		while (nsize < sz + 1) {
			nsize = nsize * 2;
		}
		parser->tmp = (char*)realloc(parser->tmp, nsize);
		parser->size = nsize;
	}

	char* tmp = parser->tmp;
	int i;
	for (i = 0; i < sz; i++) {
		int index = parser->head + i;
		if (index >= parser->total) {
			index -= parser->total;
		}
		*tmp++ = parser->buff[index];
	}

	parser->head += sz;
	if (parser->head >= parser->total) {
		parser->head -= parser->total;
	}

	*tmp++ = 0;
	return parser->tmp;
}

static inline int check_eol(struct parser_context *parser, int from, const char *sep, size_t sep_len) {
	int i;
	for (i = 0; i < sep_len; i++) {
		int index = from + i;
		if (index >= parser->total) {
			index -= parser->total;
		}
		if (parser->buff[index] != sep[i]) {
			return -1;
		}
	}
	return 0;
}

static inline char * parse_until_eol(struct parser_context *parser, const char *sep, size_t sep_len) {
	int length = get_length(parser);
	if (length == 0) {
		return NULL;
	}

	int offset;
	for (offset = 0; offset < length; offset++) {
		int index = parser->head + offset;
		if (index >= parser->total) {
			index -= parser->total;
		}
		if (parser->buff[index] == sep[0]) {
			if (length - offset >= sep_len) {
				if (check_eol(parser, index, sep, sep_len) == 0) {
					break;
				}
			}
			else {
				return NULL;
			}
		}
	}
	if (offset >= length) {
		return NULL;
	}

	char *line = parser_read(parser, offset);
	parser_skip(parser, sep_len);
	return line;
}

static inline char* parse_line(struct parser_context* parser) {
	return parse_until_eol(parser, "\r\n", 2);
}

static inline int parse_string(lua_State *L, struct parser_context *parser) {
	if (get_length(parser) < parser->reply.string.size + 2) {
		return 0;
	}
	char *str = parser_read(parser, parser->reply.string.size);
	lua_pushlstring(L, str, parser->reply.string.size);
	parser_skip(parser, 2);
	parser->phase = PBEGIN;
	return 1;
}

static inline int parse_array(lua_State *L, struct parser_context *parser) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, parser->reply.array.ref);

	while (parser->reply.array.index < parser->reply.array.count) {

		if (parser->reply.array.type == -1) {
			char *line = parse_line(parser);
			if (!line) {
				lua_pop(L, 1);
				return 0;
			}
			int val = tonumber(L, &line[1]);
			if (line[0] == '$') {
				parser->reply.array.type = 0;
				parser->reply.array.reserve.length = val;
			}
			else if (line[0] == ':') {
				parser->reply.array.type = 1;
				parser->reply.array.reserve.value = val;
			}
			else {
				luaL_unref(L, LUA_REGISTRYINDEX, parser->reply.array.ref);
				luaL_error(L, "error array member type:%s", line[0]);
			}
		}

		if (parser->reply.array.type == 0) {
			if (parser->reply.array.reserve.length == -1) {
				lua_pushstring(L, "");
			}
			else {
				if (get_length(parser) < parser->reply.array.reserve.length + 2) {
					lua_pop(L, 1);
					return 0;
				}
				char *str = parser_read(parser, parser->reply.array.reserve.length);
				lua_pushlstring(L, str, parser->reply.array.reserve.length);
			}
			parser_skip(parser, 2);
		}
		else {
			lua_pushinteger(L, parser->reply.array.reserve.value);
		}

		lua_rawseti(L, -2, parser->reply.array.index + 1);

		parser->reply.array.index++;
		parser->reply.array.type = -1;
	}

	luaL_unref(L, LUA_REGISTRYINDEX, parser->reply.array.ref);
	parser->phase = PBEGIN;
	return 1;
}

static int parser_loop(lua_State *L) {
	struct parser_context *parser = (struct parser_context*)lua_touserdata(L, 1);
	for (;;) {
		switch (parser->phase) {
			case PBEGIN: {
				 if (get_length(parser) <= 0) {
					 return 0;
				 }
				 switch (parser->buff[parser->head]) {
					 case '+': {
						char* line = parse_line(parser);
						if (!line) {
							return 0;
						}
						lua_pushboolean(L, 1);
						lua_pushstring(L, line);
						return 2;
					 }
					 case '-': {
						char* line = parse_line(parser);
						if (!line) {
							return 0;
						}
						lua_pushboolean(L, 0);
						lua_pushstring(L, line);
						return 2;
					 }
					 case ':': {
						char* line = parse_line(parser);
						if (!line) {
							return 0;
						}
						lua_Number integer = tonumber(L, line);
						lua_pushinteger(L, integer);
						return 1;
					 }
					 case '$': {
						char *line = parse_line(parser);
						if (!line) {
							return 0;
						}
						parser->reply.string.size = tonumber(L, line);
						if (parser->reply.string.size < 0) {
							lua_pushstring(L, "");
							return 1;
						}
						parser->phase = PSTRING;
						break;
					 }
					 case '*': {
						char *line = parse_line(parser);
						if (!line) {
							return 0;
						}
						parser->reply.array.count = tonumber(L, line);
						parser->reply.array.index = 0;
						parser->reply.array.type = -1;
						lua_newtable(L);
						parser->reply.array.ref = luaL_ref(L, LUA_REGISTRYINDEX);
						parser->phase = PARRAY;
						break;
					 }
					 default: {
						luaL_error(L, "error char:%c", parser->buff[parser->head]);
					 }
				 }
				 break;
			}
			case PSTRING: {
				return parse_string(L, parser);
			}
			case PARRAY: {
				return parse_array(L, parser);
			}
			default: {
				luaL_error(L, "unknown phase:%d", parser->phase);
			}
		}
	}
}

static void parser_realloc(struct parser_context* parser) {
	size_t nsize = parser->total * 2;

	char* nbuff = (char*)malloc(nsize);

	int use = get_length(parser);
	if (use > parser->total - parser->head) {
		int len = parser->total - parser->head;
		memcpy(nbuff, parser->buff + parser->head, len);
		memcpy(nbuff + len, parser->buff, use - len);
	}
	else {
		if (use > 0) {
			memcpy(nbuff, parser->buff + parser->head, use);
		}
	}

	free(parser->buff);
	parser->buff = nbuff;
	parser->total = nsize;
	parser->head = 0;
	parser->tail = use;
}

static int parser_push(lua_State *L) {
	struct parser_context *parser = (struct parser_context*)lua_touserdata(L, 1);

	char* str = NULL;
	size_t sz = 0;
	int vt = lua_type(L, 2);
	switch (vt) {
		case LUA_TSTRING: {
							  str = (char*)lua_tolstring(L, 2, &sz);
							  break;
		}
		case LUA_TLIGHTUSERDATA: {
									 str = (char*)lua_touserdata(L, 2);
									 sz = luaL_checkinteger(L, 3);
									 break;
		}
		default: {
					 luaL_error(L, "unknow lua type:%s", lua_typename(L, vt));
		}
	}

	int size = parser->total - get_length(parser);
	if (size < (int)sz) {
		parser_realloc(parser);
		size = parser->total - get_length(parser);
	}

	if (parser->tail >= parser->head) {
		if (parser->total - parser->tail >= (int)sz) {
			memcpy(parser->buff + parser->tail, str, sz);
			parser->tail += sz;
		}
		else {
			int left = parser->total - parser->tail;
			memcpy(parser->buff + parser->tail, str, left);
			parser->tail = 0;
			memcpy(parser->buff + parser->tail, str + left, sz - left);
			parser->tail += sz - left;
		}

	}
	else {
		memcpy(parser->buff + parser->tail, str, sz);
		parser->tail += sz;
	}

	return 0;
}

static int parser_release(lua_State *L) {
	struct parser_context *parser = (struct parser_context*)lua_touserdata(L, 1);
	free(parser->tmp);
	free(parser->buff);
	return 0;
}

static int parser_new(lua_State *L) {
	struct parser_context *parser = (struct parser_context*)lua_newuserdata(L, sizeof(*parser));
	memset(parser, 0, sizeof(*parser));
	parser->phase = PBEGIN;
	parser->size = 64;
	parser->tmp = (char*)malloc(parser->size);
	parser->total = 64;
	parser->buff = (char*)malloc(parser->total);
	parser->head = 0;
	parser->tail = 0;
	if (luaL_newmetatable(L, "redis_meta")) {
		const luaL_Reg meta[] = {
			{"push", parser_push},
			{"loop", parser_loop},
			{NULL, NULL},
		};
		luaL_newlib(L, meta);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, parser_release);
		lua_setfield(L, -2, "__gc");
	}

	lua_setmetatable(L, -2);
	return 1;
}

static int cmd(lua_State *L) {
	struct write_buffer wb;
	wb_init(&wb);

	int top = lua_gettop(L);

	wb_addchar(&wb, '*');
	wb_addnumber(&wb, top);
	wb_addstring(&wb, "\r\n");

	int i = 1;
	for (; i <= top; i++) {
		wb_addchar(&wb, '$');

		int type = lua_type(L, i);
		switch (type) {
			case LUA_TNUMBER: {
								  char str[64] = {0};
								  size_t len = sprintf(str, "%.16g", lua_tonumber(L, i));
								  wb_addnumber(&wb, len);
								  wb_addstring(&wb, "\r\n");
								  wb_addlstring(&wb, str, len);
								  break;
			}
			case LUA_TBOOLEAN: {
								   wb_addchar(&wb, '1');
								   wb_addstring(&wb, "\r\n");
								   int val = lua_toboolean(L, i);
								   if (val)
									   wb_addnumber(&wb, 1);
								   else
									   wb_addnumber(&wb, 0);
								   break;
			}
			case LUA_TSTRING: {
								  size_t sz = 0;
								  const char *str = lua_tolstring(L, i, &sz);
								  wb_addnumber(&wb, sz);
								  wb_addstring(&wb, "\r\n");
								  wb_addlstring(&wb, str, sz);
								  break;
			}
			default:
				luaL_error(L, "value not support type %s", lua_typename(L, type));
				break;
		}
		wb_addstring(&wb, "\r\n");
	}

	lua_pushlstring(L, wb.ptr, wb.offset);
	wb_release(&wb);
	return 1;
}

int luaopen_redis(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{"cmd", cmd},
		{"parser", parser_new},
		{NULL, NULL},
	};

	luaL_newlib(L, l);
	return 1;
}

