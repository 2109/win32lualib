#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


#define BUFFER_SIZE 1024
#define SLOT_SIZE	16
#define MAX_DEPTH	32

#ifdef WIN32
#define inline __inline
#define snprintf _snprintf
#endif

extern char* i32toa_fast(int32_t value, char* buffer);

extern char* i64toa_fast(int64_t value, char* buffer);

extern char* dtoa_fast(double value, char* buffer);

typedef struct dump_buffer {
	bool stringify;
	char* ptr;
	size_t size;
	size_t offset;
	char init[BUFFER_SIZE];
} buffer_t;

typedef struct array_pair {
	buffer_t* k;
	buffer_t* v;
} pair_t;

typedef struct array_context {
	int offset;
	int size;
	pair_t** slots;
	pair_t* init[SLOT_SIZE];
} array_t;

#ifdef WIN32
#define dump_error(L,buffer,fmt,...) \
do \
{\
	buffer_release(buffer); \
	luaL_error(L, "%s:%d pack error : "fmt" ", __FILE__, __LINE__, __VA_ARGS__); \
} while ( 0 )\

#else

#define dump_error(L,buffer,fmt,args...) \
do {\
	buffer_release(buffer);\
	luaL_error(L, "%s:%d pack error : "fmt" ", __FILE__, __LINE__, ##args);\
} while (0)\

#endif

static inline void
buffer_init(buffer_t* buffer, bool stringify) {
	buffer->stringify = stringify;
	buffer->ptr = buffer->init;
	buffer->size = BUFFER_SIZE;
	buffer->offset = 0;
}

static inline void
buffer_release(buffer_t* buffer) {
	if (buffer->ptr != buffer->init)
		free(buffer->ptr);
}

static inline void
buffer_reservce(buffer_t* buffer, size_t len) {
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
addchar(buffer_t* buffer, char c) {
	buffer_reservce(buffer, 1);
	buffer->ptr[buffer->offset++] = c;
}

static inline void
addlstring(buffer_t* buffer, const char* str, size_t len) {
	buffer_reservce(buffer, len);
	memcpy(buffer->ptr + buffer->offset, str, len);
	buffer->offset += len;
}

static inline void
addstring(buffer_t* buffer, const char* str) {
	int len = strlen(str);
	addlstring(buffer, str, len);
}

static inline void
begin_table(buffer_t* buffer) {
	if (buffer->stringify) {
		addstring(buffer, "{\n");
	}
	else {
		addchar(buffer, '{');
	}
}

static inline void
tab(buffer_t* buffer, int depth) {
	if (buffer->stringify) {
		buffer_reservce(buffer, depth);
		memset(buffer->ptr + buffer->offset, '\t', depth);
		buffer->offset += depth;
	}
}

static inline void
newline(buffer_t* buffer) {
	if (buffer->stringify) {
		addstring(buffer, ",\n");
	}
	else {
		addchar(buffer, ',');
	}
}

static inline void
array_init(array_t* array) {
	array->size = SLOT_SIZE;
	array->offset = 0;
	array->slots = array->init;
}

static inline void
array_release(array_t* array) {
	int i;
	for (i = 0; i < array->offset; i++) {
		pair_t* pair = array->slots[i];
		buffer_release(pair->k);
		buffer_release(pair->v);
		free(pair->k);
		free(pair->v);
		free(pair);
	}

	if (array->slots != array->init)
		free(array->slots);
}

static inline void
array_add(array_t* array, buffer_t* k, buffer_t* v) {
	if (array->offset == array->size) {
		int nsize = array->size * 2;
		pair_t** nslots = (pair_t**)malloc(sizeof(pair_t*) * nsize);
		memcpy(nslots, array->slots, sizeof(pair_t*) * array->size);
		if (array->slots != array->init)
			free(array->slots);
		array->slots = nslots;
		array->size = nsize;
	}
	pair_t* pair = (pair_t*)malloc(sizeof(*pair));
	pair->k = k;
	pair->v = v;
	array->slots[array->offset++] = pair;
}

static inline int
array_compare(const void* lhs, const void* rhs) {
	pair_t* l = *(pair_t**)lhs;
	pair_t* r = *(pair_t**)rhs;
	return strcmp(l->k->ptr, r->k->ptr);
}

static inline void
array_sort(array_t* array) {
	qsort(array->slots, array->offset, sizeof(pair_t*), array_compare);
}

void dump_table(lua_State* L, buffer_t* buffer, int index, int depth);
void dump_table_order(lua_State* L, buffer_t* buffer, int index, int depth);

static inline void
dump_string(lua_State* L, buffer_t* buffer, const char* str, size_t size) {
	addchar(buffer, '\"');
	size_t i;
	for (i = 0; i < size; i++) {
		char ch = str[i];
		switch (ch) {
			case '\'': case '\"': case '\\': {
				addchar(buffer, '\\');
				addchar(buffer, ch);
				break;
			}
			case '\r': {
				addlstring(buffer, "\\r", 2);
				break;
			}
			case '\n': {
				addlstring(buffer, "\\n", 2);
				break;
			}
			case '\t': {
				addlstring(buffer, "\\t", 2);
				break;
			}
			case '\0': {
				addlstring(buffer, "\\0", 2);
				break;
			}
			default: {
				addchar(buffer, ch);
				break;
			}
		}
	}
	addchar(buffer, '\"');
}

static inline void
dump_number(lua_State* L, buffer_t* buffer, int index) {
	char str[32];
	size_t len;

	if (lua_isinteger(L, index)) {
		lua_Integer val = lua_tointeger(L, index);
		int32_t i32 = (int32_t)val;
		char* end = NULL;
		if ((lua_Integer)i32 == val) {
			end = i32toa_fast(i32, str);
		}
		else {
			end = i64toa_fast(val, str);
		}
		*end = 0;
		len = end - str;
	}
	else {
		lua_Number n = lua_tonumber(L, index);
		dtoa_fast(n, str);
		len = strlen(str);
	}
	addlstring(buffer, str, len);
}

static void
dump_one(lua_State* L, buffer_t* buffer, int index, int depth, bool order, bool iskey) {
	int type = lua_type(L, index);
	switch (type) {
		case LUA_TNIL: {
			addstring(buffer, "nil");
			break;
		}
		case LUA_TBOOLEAN: {
			if (lua_toboolean(L, index)) {
				addstring(buffer, "true");
			}
			else {
				addstring(buffer, "false");
			}
			break;
		}
		case LUA_TNUMBER: {
			dump_number(L, buffer, index);
			break;
		}
		case LUA_TSTRING: {
			size_t sz = 0;
			const char *str = lua_tolstring(L, index, &sz);
			dump_string(L, buffer, str, sz);
			break;
		}
		case LUA_TTABLE: {
			if (index < 0) {
				index = lua_gettop(L) + index + 1;
			}

			if (iskey) {
				const void* pointer = lua_topointer(L, index);
				char str[64] = { 0 };
				snprintf(str, 64, "\"<table:0x%x>\"", (uint32_t)(uintptr_t)pointer);
				append_lstring(L, buffer, str);
			}
			else {
				if (order) {
					dump_table_order(L, buffer, index, ++depth);
				}
				else {
					dump_table(L, buffer, index, ++depth);
				}
			}
			break;
		}
		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA: {
			const void* pointer = lua_topointer(L, index);
			char str[64] = { 0 };
			snprintf(str, 64, "\"<userdata:0x%x>\"", (uint32_t)(uintptr_t)pointer);
			append_lstring(L, buffer, str);
			break;
		}
		case LUA_TFUNCTION:
		{
			const void* pointer = lua_topointer(L, index);
			char str[64] = { 0 };
			snprintf(str, 64, "\"<function:0x%x>\"", (uint32_t)(uintptr_t)pointer);
			append_lstring(L, buffer, str);
			break;
		}
		case LUA_TTHREAD:
		{
			const void* pointer = lua_topointer(L, index);
			char str[64] = { 0 };
			snprintf(str, 64, "\"<thread:0x%x>\"", (uint32_t)(uintptr_t)pointer);
			append_lstring(L, buffer, str);
			break;
		}
		default: {
			dump_error(L, buffer, "not support type %s", lua_typename(L, type));
			break;
		}
	}
}

static inline int
dump_table_array(lua_State* L, buffer_t* buffer, int index, int depth, bool order) {
	int size = lua_rawlen(L, index);
	int i;
	for (i = 1; i <= size; i++) {
		lua_rawgeti(L, index, i);
		tab(buffer, depth);
		dump_one(L, buffer, -1, depth, order, false);
		newline(buffer);
		lua_pop(L, 1);
	}
	return size;
}

void
dump_table(lua_State* L, buffer_t* buffer, int index, int depth) {
	if (depth > MAX_DEPTH) {
		dump_error(L, buffer, "pack table too depth:%d", depth);
	}

	begin_table(buffer);

	int size = dump_table_array(L, buffer, index, depth, false);

	lua_pushnil(L);
	while (lua_next(L, index) != 0) {
		if (lua_type(L, -2) == LUA_TNUMBER) {
			if (lua_isinteger(L, -2)) {
				lua_Integer i = lua_tointeger(L, -2);
				if (i > 0 && i <= size) {
					lua_pop(L, 1);
					continue;
				}
			}
		}

		tab(buffer, depth);

		addchar(buffer, '[');
		dump_one(L, buffer, -2, depth, false, true);
		addstring(buffer, "]=");
		dump_one(L, buffer, -1, depth, false, false);
		newline(buffer);
		
		lua_pop(L, 1);
	}

	tab(buffer, depth - 1);
	addchar(buffer, '}');
}

void
dump_table_order(lua_State* L, buffer_t* buffer, int index, int depth) {
	if (depth > MAX_DEPTH) {
		dump_error(L, buffer, "pack table sort too depth:%d", depth);
	}
	begin_table(buffer);

	int size = dump_table_array(L, buffer, index, depth, true);

	array_t array;
	array_init(&array);

	lua_pushnil(L);
	while (lua_next(L, index) != 0) {
		if (lua_type(L, -2) == LUA_TNUMBER) {
			lua_Number n = lua_tonumber(L, -2);
			lua_Integer i = lua_tointeger(L, -2);
			if (n == (lua_Number)i){
				if (i > 0 && i <= size) {
					lua_pop(L, 1);
					continue;
				}
			}
		}

		buffer_t* lhs = (buffer_t*)malloc(sizeof(*lhs));
		buffer_init(lhs, buffer->stringify);
		dump_one(L, lhs, -2, depth, true, true);

		buffer_t* rhs = (buffer_t*)malloc(sizeof(*rhs));
		buffer_init(rhs, buffer->stringify);
		dump_one(L, rhs, -1, depth, true, false);

		array_add(&array, lhs, rhs);

		lua_pop(L, 1);
	}

	array_sort(&array);

	int i;
	for (i = 0; i < array.offset; i++) {
		pair_t* pair = array.slots[i];
		tab(buffer, depth);

		addchar(buffer, '[');
		addlstring(buffer, pair->k->ptr, pair->k->offset);
		addstring(buffer, "]=");
		addlstring(buffer, pair->v->ptr, pair->v->offset);
		newline(buffer);
	}

	array_release(&array);

	tab(buffer, depth - 1);
	addchar(buffer, '}');
}

static int
dump(lua_State* L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	bool stringify = luaL_optinteger(L, 2, 0);
	bool order = luaL_optinteger(L, 3, 0);
	bool tostr = luaL_optinteger(L, 4, 0);
		
	luaL_checkstack(L, MAX_DEPTH * 2 + 4, NULL);

	buffer_t buffer;
	buffer_init(&buffer, stringify);

	if (order) {
		dump_table_order(L, &buffer, 1, 1);
	}
	else {
		dump_table(L, &buffer, 1, 1);
	}

	if (tostr) {
		lua_pushlstring(L, buffer.ptr, buffer.offset);
		lua_pushinteger(L, buffer.offset);
	}
	else {
		void* data = malloc(buffer.offset);
		memcpy(data, buffer.ptr, buffer.offset);
		lua_pushlightuserdata(L, data);
		lua_pushinteger(L, buffer.offset);
	}

	buffer_release(&buffer);
	
	return 2;
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

#ifdef WIN32

#define parse_error(L,parser,fmt,...) \
do \
{\
	int offset = parser->ptr - parser->data;\
	char ch = *parser->ptr;\
	parser_release(parser);\
	luaL_error(L, "%s:%d parse error at:%d : "fmt" ", __FILE__, __LINE__,offset, __VA_ARGS__); \
} while (0)\

#else

#define parse_error(L,parser,fmt,args...) \
do {\
	int offset = (parser)->ptr - (parser)->data;\
	parser_release((parser));\
	luaL_error(L, "%s:%d parse error at:%d : "fmt" ", __FILE__, __LINE__,offset, ##args); \
} while (0)

#endif

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
		parse_error(L, parser, "unexpect eof");
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
	parse_error(L, parser, "unexpect eof");
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
	parse_error(L, parser, "unexpect eof");
}

extern double fpconv_strtod(const char *s00, char **se);

static inline void
eat_number(lua_State* L, struct parser_context *parser) {
	char* end = NULL;
	lua_Number number = strtod(parser->ptr, &end);
	// lua_Number number = fpconv_strtod(parser->ptr, &end);
	lua_Integer integer = number;

	if (parser->ptr == end || end >= parser->data + parser->size) {
		parse_error(L, parser, "parse number error");
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
parse_table(lua_State* L, struct parser_context *parser, int depth);

void
parse_key(lua_State* L, struct parser_context *parser, int i, int depth) {
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
			parse_error(L, parser, "unknown char:%c", ch);
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
		parse_table(L, parser, depth);
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
					parse_error(L, parser, "expect ,or} after %s", kw);
				}
			}
		}
		if (!expect(parser, '=')) {
			parse_error(L, parser, "expect =,unknown char:%c", *parser->ptr);
		}
		lua_pushlstring(L, parser->reserve, index);
		return;
	}
	}
	parse_error(L, parser, "unknown char:%c", *parser->ptr);
	return;
}

void
parse_value(lua_State* L, struct parser_context *parser, int depth) {
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
		parse_table(L, parser, depth);
		eat(L, parser, 1);
		eat_space(parser);
		return;
	}
	default:
		break;
	}
	parse_error(L, parser, "unknown char:%c", *parser->ptr);
}

void
parse_table(lua_State* L, struct parser_context *parser, int depth) {
	++depth;
	if (depth > MAX_DEPTH) {
		parse_error(L, parser, "parse table too depth:%d", depth);
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
			parse_key(L, parser, i, depth);
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
			parse_error(L, parser, "expect =,unknown char:%c", *parser->ptr);
		}

		eat(L, parser, 1);
		eat_space(parser);
		parse_value(L, parser, depth);

		lua_rawset(L, -3);
	}
}

int
parse(lua_State* L) {
	size_t size;
	const char* str;
	if (lua_type(L, 1) == LUA_TLIGHTUSERDATA) {
		str = lua_touserdata(L, 1);
		size = lua_tointeger(L, 2);
	}
	else {
		str = luaL_checklstring(L, 1, &size);
	}

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
		parse_error(L, (&parser), "expect {,unknown char:%c", *parser.ptr);
	}
	int depth = 0;
	parse_table(L, &parser, depth);

	parser_release(&parser);

	return 1;
}

__declspec(dllexport) int
luaopen_serialize(lua_State* L) {
	luaL_Reg l[] = {
		{ "dump", dump },
		{ "parse", parse },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}
