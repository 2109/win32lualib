#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


#define BUFFER_SIZE 1024
#define SLOT_SIZE	16
#define MAX_DEPTH	32

#ifdef _WIN32
#define inline __inline
#endif
//#define ESCAPE_SPACE

struct write_buffer {
	char* ptr;
	size_t size;
	size_t offset;
	char init[BUFFER_SIZE];
};

struct array_kv {
	struct write_buffer* k;
	struct write_buffer* v;
};

struct array_context {
	int offset;
	int size;
	struct array_kv** slots;
	struct array_kv* init[SLOT_SIZE];
};

#define pack_error(L,buffer,fmt,...) \
do \
{\
	buffer_release(buffer);\
	luaL_error(L, "%s:%d pack error : "fmt" ", __FILE__, __LINE__, __VA_ARGS__);\
} while (0)\

static inline void
buffer_init(struct write_buffer* buffer) {
	buffer->ptr = buffer->init;
	buffer->size = BUFFER_SIZE;
	buffer->offset = 0;
}

static inline void
buffer_reservce(struct write_buffer* buffer, size_t len) {
	if (buffer->offset + len <= buffer->size) {
		return;
	}
	size_t nsize = buffer->size * 2;
	while (nsize < buffer->offset + len) {
		nsize = nsize * 2;
	}
	char* nptr = (char*)malloc(nsize);
	memcpy(nptr, buffer->ptr, buffer->size);
	buffer->size = nsize;

	if (buffer->ptr != buffer->init)
		free(buffer->ptr);
	buffer->ptr = nptr;
}

static inline void
buffer_addchar(struct write_buffer* buffer, char c) {
	buffer_reservce(buffer, 1);
	buffer->ptr[buffer->offset++] = c;
}

static inline void
buffer_addlstring(struct write_buffer* buffer, const char* str, size_t len) {
	buffer_reservce(buffer, len);
	memcpy(buffer->ptr + buffer->offset, str, len);
	buffer->offset += len;
}

static inline void
buffer_addstring(struct write_buffer* buffer, const char* str) {
	int len = strlen(str);
	buffer_addlstring(buffer, str, len);
}

static inline void
buffer_release(struct write_buffer* buffer) {
	if (buffer->ptr != buffer->init)
		free(buffer->ptr);
}

#ifdef ESCAPE_SPACE
#define tab(buffer,depth) ((void)0)
#else
#define tab(buffer,depth) \
do{\
\
buffer_reservce(buffer, depth); \
memset(buffer->ptr + buffer->offset, '\t', depth); \
buffer->offset += depth; \
} while (0)
#endif

#ifdef ESCAPE_SPACE
#define newline(buffer) buffer_addchar(buffer, ',')
#else
#define newline(buffer) buffer_addstring(buffer, ",\n")
#endif

#ifdef ESCAPE_SPACE
#define table_begin(buffer) buffer_addchar(buffer, '{')
#else
#define table_begin(buffer) buffer_addstring(buffer, "{\n")
#endif

static inline void
array_init(struct array_context* array) {
	array->size = SLOT_SIZE;
	array->offset = 0;
	array->slots = array->init;
}

static inline void
array_release(struct array_context* array) {
	int i;
	for (i = 0; i < array->offset; i++) {
		struct array_kv* kv = array->slots[i];
		buffer_release(kv->k);
		buffer_release(kv->v);
		free(kv->k);
		free(kv->v);
		free(kv);
	}

	if (array->slots != array->init)
		free(array->slots);
}

static inline void
array_append(struct array_context* array, struct write_buffer* k, struct write_buffer* v) {
	if (array->offset == array->size) {
		int nsize = array->size * 2;
		struct array_kv** nslots = (struct array_kv**)malloc(sizeof(struct array_kv*) * nsize);
		memcpy(nslots, array->slots, sizeof(struct array_kv*) * array->size);
		if (array->slots != array->init)
			free(array->slots);
		array->slots = nslots;
		array->size = nsize;
	}
	struct array_kv* kv = (struct array_kv*)malloc(sizeof(*kv));
	kv->k = k;
	kv->v = v;
	array->slots[array->offset++] = kv;
}

static inline int
array_compare(const void* a, const void* b) {
	struct array_kv* l = *(struct array_kv**)a;
	struct array_kv* r = *(struct array_kv**)b;
	return strcmp(l->k->ptr, r->k->ptr);
}

static inline void
array_sort(struct array_context* array) {
	qsort(array->slots, array->offset, sizeof(struct array_kv*), array_compare);
}

void pack_table(lua_State* L, struct write_buffer* buffer, int index, int depth);
void pack_table_order(lua_State* L, struct write_buffer* buffer, int index, int depth);

static inline void
append_string(lua_State* L, struct write_buffer* buffer, const char* str, size_t size) {
	buffer_addchar(buffer, '\"');
	size_t i;
	for (i = 0; i < size; i++) {
		char ch = str[i];
		switch (ch) {
		case '\'':
		case '\"':
		case '\\':
		{
					 buffer_addchar(buffer, '\\');
					 buffer_addchar(buffer, ch);
					 break;
		}
		case '\r':
		{
					 buffer_addlstring(buffer, "\\r", 2);
					 break;
		}
		case '\n':
		{
					 buffer_addlstring(buffer, "\\n", 2);
					 break;
		}
		case '\t':
		{
					 buffer_addlstring(buffer, "\\t", 2);
					 break;
		}
		case '\0':
		{
					 buffer_addlstring(buffer, "\\0", 2);
					 break;
		}
		default:
		{
				   buffer_addchar(buffer, ch);
				   break;
		}

		}
	}
	buffer_addchar(buffer, '\"');
}

static inline void
append_number(lua_State* L, struct write_buffer* buffer, int index) {
	char str[32];
	int len;
	
	if ( lua_isinteger(L, index) ) {
		lua_Integer i = lua_tointeger(L, index);
		int32_t i32 = (int32_t)i;
		char* end = NULL;
		if ( (lua_Integer)i32 == i ) {
			end = i32toa_fast(i32, str);
		}
		else {
			end = i64toa_fast(i, str);
		}
		*end = 0;
		len = end - str;
	}
	else {
		lua_Number n = lua_tonumber(L, index);
		dtoa_fast(n, str);
		len = strlen(str);
	}
	buffer_addlstring(buffer, str, len);
}

void
pack_key(lua_State* L, struct write_buffer* buffer, int index, int depth) {
	int type = lua_type(L, index);
	switch (type)
	{
	case LUA_TNUMBER:
	{
						append_number(L, buffer, index);
						break;
	}
	case LUA_TSTRING:
	{
						size_t sz = 0;
						const char *str = lua_tolstring(L, index, &sz);
						append_string(L, buffer, str, sz);
						break;
	}
	default:
		pack_error(L, buffer, "key not support type %s", lua_typename(L, type));
		break;
	}
}


void
pack_value(lua_State* L, struct write_buffer* buffer, int index, int depth, int sort) {
	int type = lua_type(L, index);
	switch (type) {
	case LUA_TNIL:
		buffer_addstring(buffer, "nil");
		break;
	case LUA_TNUMBER:
	{
						append_number(L, buffer, index);
						break;
	}
	case LUA_TBOOLEAN:
	{
						 int val = lua_toboolean(L, index);
						 if (val)
							 buffer_addstring(buffer, "true");
						 else
							 buffer_addstring(buffer, "false");
						 break;
	}
	case LUA_TSTRING:
	{
						size_t sz = 0;
						const char *str = lua_tolstring(L, index, &sz);
						append_string(L, buffer, str, sz);
						break;
	}
	case LUA_TTABLE:
	{
					   if (index < 0) {
						   index = lua_gettop(L) + index + 1;
					   }

					   if (sort) {
						   pack_table_order(L, buffer, index, ++depth);
					   }
					   else {
						   pack_table(L, buffer, index, ++depth);
					   }

					   break;
	}
	default:
		pack_error(L, buffer, "value not support type %s", lua_typename(L, type));
		break;
	}
}

void
pack_table(lua_State* L, struct write_buffer* buffer, int index, int depth) {
	if (depth > MAX_DEPTH) {
		pack_error(L, buffer, "pack table too depth:%d", depth);
	}
	table_begin(buffer);
	int array_size = lua_rawlen(L, index);
	int i;
	for (i = 1; i <= array_size; i++) {
		lua_rawgeti(L, index, i);
		tab(buffer, depth);
		pack_value(L, buffer, -1, depth, 0);
		newline(buffer);
		lua_pop(L, 1);
	}

	lua_pushnil(L);
	while (lua_next(L, index) != 0) {
		if (lua_type(L, -2) == LUA_TNUMBER) {
			if (lua_isinteger(L, -2)) {
				lua_Integer i = lua_tointeger(L, -2);
				if (i > 0 && i <= array_size) {
					lua_pop(L, 1);
					continue;
				}
			}
		}

		tab(buffer, depth);

		buffer_addchar(buffer, '[');
		pack_key(L, buffer, -2, depth);
		buffer_addstring(buffer, "] = ");

		pack_value(L, buffer, -1, depth, 0);

		newline(buffer);
		lua_pop(L, 1);
	}

	tab(buffer, depth - 1);
	buffer_addchar(buffer, '}');
}

void
pack_table_order(lua_State* L, struct write_buffer* buffer, int index, int depth) {
	if (depth > MAX_DEPTH) {
		pack_error(L, buffer, "pack table sort too depth:%d", depth);
	}
	table_begin(buffer);
	int array_size = lua_rawlen(L, index);
	int i;
	for (i = 1; i <= array_size; i++) {
		lua_rawgeti(L, index, i);
		tab(buffer, depth);
		pack_value(L, buffer, -1, depth, 1);
		newline(buffer);
		lua_pop(L, 1);
	}

	struct array_context array;
	array_init(&array);

	lua_pushnil(L);
	while (lua_next(L, index) != 0) {
		if (lua_type(L, -2) == LUA_TNUMBER) {
			lua_Number n = lua_tonumber(L, -2);
			lua_Integer i = lua_tointeger(L, -2);
			if (n == (lua_Number)i){
				if (i > 0 && i <= array_size) {
					lua_pop(L, 1);
					continue;
				}
			}
		}

		struct write_buffer* kbuffer = (struct write_buffer*)malloc(sizeof(*kbuffer));
		buffer_init(kbuffer);

		buffer_addchar(kbuffer, '[');
		pack_key(L, kbuffer, -2, depth);
		buffer_addstring(kbuffer, "] = ");

		struct write_buffer* vbuffer = (struct write_buffer*)malloc(sizeof(*vbuffer));
		buffer_init(vbuffer);

		pack_value(L, vbuffer, -1, depth, 1);

		array_append(&array, kbuffer, vbuffer);

		lua_pop(L, 1);
	}

	array_sort(&array);

	for (i = 0; i < array.offset; i++) {
		struct array_kv* kv = array.slots[i];
		tab(buffer, depth);
		buffer_addlstring(buffer, kv->k->ptr, kv->k->offset);
		buffer_addlstring(buffer, kv->v->ptr, kv->v->offset);
		newline(buffer);
	}

	array_release(&array);

	tab(buffer, depth - 1);
	buffer_addchar(buffer, '}');
}

static int
pack(lua_State* L) {
	int type = lua_type(L, 1);
	if (type != LUA_TTABLE)
		luaL_error(L, "must be table");

	luaL_checkstack(L, MAX_DEPTH * 2 + 4, NULL);

	struct write_buffer buffer;
	buffer_init(&buffer);

	pack_table(L, &buffer, 1, 1);

	lua_pushlstring(L, buffer.ptr, buffer.offset);

	buffer_release(&buffer);
	return 1;
}

static int
pack_order(lua_State* L) {
	int type = lua_type(L, 1);
	if (type != LUA_TTABLE)
		luaL_error(L, "must be table");

	luaL_checkstack(L, MAX_DEPTH * 2 + 4, NULL);

	struct write_buffer buffer;
	buffer_init(&buffer);

	pack_table_order(L, &buffer, 1, 1);

	lua_pushlstring(L, buffer.ptr, buffer.offset);

	buffer_release(&buffer);
	return 1;
}

struct parser_context {
	char* data;
	char* ptr;
	size_t size;
	char* reserve;
	char escape[256];
	int length;
};

static const char* KEY_WORD[] = { "nil", "true", "false" };
static const int KEY_WORD_SIZE[] = { 3, 4, 5 };

#define eof(parser) ((parser)->ptr >= (parser)->data + (parser)->size)
#define expect(parser,c) (*((parser)->ptr) == c)

static inline void
parser_init(struct parser_context* parser, const char* str, size_t size) {
	parser->data = (char*)str;
	parser->ptr = parser->data;
	parser->size = size;
	parser->length = 64;
	parser->reserve = malloc(parser->length);
}

static inline void
parser_release(struct parser_context* parser) {
	free(parser->reserve);
}


#define unpack_error(L,parser,fmt,...) \
do \
{\
	int offset = parser->ptr - parser->data;\
	char ch = *parser->ptr;\
	parser_release(parser);\
	luaL_error(L, "%s:%d unpack error at:%d : "fmt" ", __FILE__, __LINE__,offset, __VA_ARGS__); \
} while (0)\

static inline void
reserve_eat(struct parser_context *parser, int index, char ch) {
	if (index >= parser->length) {
		parser->reserve = realloc(parser->reserve, parser->length * 2);
		parser->length *= 2;
	}
	parser->reserve[index] = ch;
}

static inline void
eat_space(struct parser_context* parser) {
	while (isspace(*parser->ptr) && !eof(parser)) {
		parser->ptr++;
	}
	return;
}

static inline void
eat(lua_State* L, struct parser_context* parser, int offset) {
	if (eof(parser)) {
		return;
	}
	if (parser->ptr + offset > parser->data + parser->size) {
		parser_release(parser);
		luaL_error(L, "parser error:eof");
	}
	parser->ptr += offset;
}

static inline void
eat_string(lua_State* L, struct parser_context *parser) {
	char quot = *parser->ptr;
	eat(L, parser, 1);
	int index = 0;
	char ch = *parser->ptr;
	while (!eof(parser)) {
		if (ch == quot) {
			lua_pushlstring(L, parser->reserve, index);
			eat(L, parser, 1);
			eat_space(parser);
			return;
		}
		if (ch == '\\') {
			ch = *(parser->ptr + 1);
			ch = parser->escape[(unsigned char)ch];
			if (ch != -1)
				eat(L, parser, 1);
			else
				ch = *parser->ptr;
		}
		reserve_eat(parser, index, ch);
		eat(L, parser, 1);
		ch = *parser->ptr;
		index++;
	}
	unpack_error(L, parser, "unexpect eof");
}

static inline void
eat_pure_string(lua_State* L, struct parser_context *parser) {
	int index = 0;
	char ch = *parser->ptr;
	char next = *(parser->ptr + 1);
	while (!eof(parser)) {
		if (ch == ']' && next == ']') {
			lua_pushlstring(L, parser->reserve, index);
			eat(L, parser, 2);
			eat_space(parser);
			return;
		}

		reserve_eat(parser, index, ch);
		eat(L, parser, 1);
		index++;
		ch = *parser->ptr;
		next = *(parser->ptr + 1);
	}
	unpack_error(L, parser, "unexpect eof");
}

static inline void
eat_number(lua_State* L, struct parser_context *parser) {
	char* end = NULL;
	lua_Number number = strtod(parser->ptr, &end);
	lua_Integer integer = (lua_Integer)number;

	if (parser->ptr == end || end >= parser->data + parser->size) {
		unpack_error(L, parser, "parse number error");
	}

	parser->ptr = end;
	if ((lua_Number)integer == number) {
		lua_pushinteger(L, integer);
	}
	else {
		lua_pushnumber(L, number);
	}
	eat_space(parser);
}

void
unpack_table(lua_State* L, struct parser_context *parser, int depth);

void
unpack_key(lua_State* L, struct parser_context *parser, int i, int depth) {
	char ch = *parser->ptr;
	switch (ch)
	{
	case '[':
	{
				eat(L, parser, 1);
				ch = *parser->ptr;
				switch (ch)
				{
				case '\'':
				case '\"':
				{
							 eat_string(L, parser);
							 break;
				}
				case '.':
				case '-':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
							eat_number(L, parser);
							break;
				}
				default:
				{
						   unpack_error(L, parser, "unknown char:%c", ch);
				}
				}
				eat(L, parser, 1);
				eat_space(parser);
				return;
	}
	case '\'':
	case '\"':
	{
				 eat_string(L, parser);
				 return;
	}
	case '.':
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
				eat_number(L, parser);
				return;
	}
	case '{':
	{
				unpack_table(L, parser, depth);
				eat(L, parser, 1);
				eat_space(parser);
				return;
	}
	default:
	{
			   int index = 0;
			   char ch = *parser->ptr;
			   while (!eof(parser)) {
				   if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'z') || ch == '_') {
					   reserve_eat(parser, index, ch);
					   index++;
					   eat(L, parser, 1);
					   ch = *parser->ptr;
				   }
				   else {
					   eat_space(parser);
					   break;
				   }
			   }

			   int i;
			   for (i = 0; i < 3; i++) {
				   const char* kw = KEY_WORD[i];
				   const int kws = KEY_WORD_SIZE[i];
				   if (kws == index && strncmp(kw, parser->reserve, index) == 0) {
					   if (expect(parser, ',') || expect(parser, '}')) {
						   if (i == 0) {
							   lua_pushnil(L);
						   }
						   else if (i == 1) {
							   lua_pushboolean(L, 1);
						   }
						   else {
							   lua_pushboolean(L, 0);
						   }
						   return;
					   }
					   else {
						   unpack_error(L, parser, "expect ,or} after %s",kw);
					   }
				   }
			   }
			   if (!expect(parser, '=')) {
				   unpack_error(L, parser, "expect =,unknown char:%c",*parser->ptr);
			   }
			   lua_pushlstring(L, parser->reserve, index);
			   return;
	}
	}
	unpack_error(L, parser, "unknown char:%c", *parser->ptr);
	return;
}

void
unpack_value(lua_State* L, struct parser_context *parser, int depth) {
	char ch = *parser->ptr;

	switch (ch) {
	case '\'':
	case '"':
	{
				eat_string(L, parser);
				return;
	}
	case '[':
	{
				if (*(parser->ptr + 1) == '[') {
					eat(L, parser, 2);
					eat_pure_string(L, parser);
					return;
				}
				else {
					break;
				}
	}
	case '.':
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
				eat_number(L, parser);
				return;
	}
	case 'n':
	{
				if (strncmp(parser->ptr, "nil", 3) == 0) {
					lua_pushnil(L);
					eat(L, parser, 3);
					eat_space(parser);
					return;
				}
				else {
					break;
				}
	}
	case 't':
	{
				if (strncmp(parser->ptr, "true", 4) == 0) {
					lua_pushboolean(L, 1);
					eat(L, parser, 4);
					eat_space(parser);
					return;
				}
				else {
					break;
				}
	}
	case 'f':
	{
				if (strncmp(parser->ptr, "false", 5) == 0) {
					lua_pushboolean(L, 0);
					eat(L, parser, 5);
					eat_space(parser);
					return;
				}
				else {
					break;
				}
	}
	case '{':
	{
				unpack_table(L, parser, depth);
				eat(L, parser, 1);
				eat_space(parser);
				return;
	}
	default:
		break;
	}
	unpack_error(L, parser, "unknown char:%c", *parser->ptr);
}

void
unpack_table(lua_State* L, struct parser_context *parser, int depth) {
	++depth;
	if (depth > MAX_DEPTH) {
		unpack_error(L, parser, "unpack table too depth:%d", depth);
	}

	lua_newtable(L);

	eat(L, parser, 1);

	eat_space(parser);

	int i = 1;
	while (!expect(parser, '}')) {
		if (expect(parser, ',')) {
			eat(L, parser, 1);
			eat_space(parser);
			continue;
		}
		else {
			unpack_key(L, parser, i, depth);
		}

		if (expect(parser, ',')) {
			lua_rawseti(L, -2, i);
			i++;
			continue;
		}
		if (expect(parser, '}')) {
			lua_rawseti(L, -2, i);
			i++;
			break;
		}
		if (!expect(parser, '=')) {
			unpack_error(L, parser, "expect =,unknown char:%c", *parser->ptr);
		}

		eat(L, parser, 1);
		eat_space(parser);
		unpack_value(L, parser, depth);

		lua_rawset(L, -3);
	}
}

int
unpack(lua_State* L) {
	size_t size;
	const char* str = luaL_checklstring(L, 1, &size);
	struct parser_context parser;
	parser_init(&parser, str, size);

	memset(parser.escape, -1, 256);
	parser.escape['\''] = '\'';
	parser.escape['\"'] = '\"';
	parser.escape['\\'] = '\\';
	parser.escape['t'] = '\t';
	parser.escape['n'] = '\n';
	parser.escape['r'] = '\r';
	parser.escape['0'] = '\0';

	luaL_checkstack(L, MAX_DEPTH * 2 + 4, NULL);

	eat_space(&parser);
	if (!expect(&parser, '{')) {
		unpack_error(L, (&parser), "expect {,unknown char:%c", *parser.ptr);
	}
	int depth = 0;
	unpack_table(L, &parser, depth);

	free(parser.reserve);

	return 1;
}

__declspec(dllexport) int
luaopen_serialize(lua_State* L){
	luaL_Reg l[] = {
		{ "pack", pack },
		{ "pack_order", pack_order },
		{ "unpack", unpack },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}