#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "khash.h"
#include "utf8.h"

#define inline __inline

#define STACK_SIZE 64

struct length_list;

KHASH_MAP_INIT_INT(word, struct length_list*);
KHASH_SET_INIT_STR(word_set);

struct string {
	char* data;
	char init[STACK_SIZE];
	size_t size;
	size_t offset;
};

typedef struct utf8 {
	uint32_t size;
	uint32_t offset;
	utf8_int32_t* ptr;
	utf8_int32_t init[STACK_SIZE];
} utf8_t;

struct length_info {
	uint8_t length;
	uint32_t first;
};

struct length_list {
	struct length_info* slots;
	size_t offset;
	size_t size;
};

struct word_map {
	khash_t(word)* hash;
	khash_t(word_set)* set;
};

static inline void
string_init(struct string* str) {
	str->size = STACK_SIZE;
	str->offset = 0;
	str->data = str->init;
	memset(str->data, 0, str->size);
}

static inline void
string_release(struct string* str) {
	if ( str->data != str->init )
		free(str->data);
}

static inline size_t
string_length(struct string* str) {
	return str->offset;
}

static inline void
string_push(struct string* str, char* data, size_t size) {
	if ( str->offset + size + 1 >= str->size ) {
		str->size = str->size * 2;
		if ( str->size <= str->offset + size + 1 )
			str->size = str->offset + size + 1;
		char* odata = str->data;
		str->data = malloc(str->size);
		memset(str->data, 0, str->size);
		memcpy(str->data, odata, str->offset);
		if ( str->init != odata )
			free(odata);
	}
	memcpy(str->data + str->offset, data, size);
	str->offset += size;
}

static inline void
string_append_utf8(struct string* str, utf8_int32_t code) {
	char word[8] = {0};
	char* over = utf8catcodepoint(word, code, 8);
	string_push(str, word, over - word);
}

static inline void
string_append_str(struct string* str, char* data, size_t size) {
	string_push(str, data, size);
}

static inline void
string_append_one(struct string* str, const char ch) {
	string_push(str, (char*)&ch, 1);
}

static inline void
utf8_init(utf8_t* utf8) {
	utf8->ptr = utf8->init;
	utf8->size = STACK_SIZE;
	utf8->offset = 0;
}

static inline void
utf8_release(utf8_t* utf8) {
	if ( utf8->ptr != utf8->init ) {
		free(utf8->ptr);
	}
}

static inline void
utf8_append(utf8_t* utf8, uint32_t val) {
	if ( utf8->offset >= utf8->size ) {
		utf8->size = utf8->size * 2;
		uint32_t* optr = utf8->ptr;
		utf8->ptr = malloc(utf8->size * sizeof( uint32_t ));
		memset(utf8->ptr, 0, utf8->size * sizeof( uint32_t ));
		memcpy(utf8->ptr, optr, utf8->offset * sizeof( uint32_t ));
		if ( utf8->init != optr )
			free(optr);
	}
	utf8->ptr[utf8->offset++] = val;
}

static inline int
length_sort(const void * left, const void * right) {
	const struct length_info* l = left;
	const struct length_info* r = right;
	return l->length < r->length;
}


void
word_add(struct word_map* map, utf8_t* utf8) {
	utf8_int32_t last = utf8->ptr[utf8->offset - 1];
	khiter_t k = kh_get(word, map->hash, last);
	struct length_list* list = NULL;
	if ( k == kh_end(map->hash) ) {
		list = malloc(sizeof( *list ));
		list->size = 2;
		list->offset = 0;
		list->slots = malloc(sizeof( struct length_info ) * list->size);
		memset(list->slots, 0, sizeof( struct length_info ) * list->size);
		int result;
		k = kh_put(word, map->hash, last, &result);
		if ( result == 1 || result == 2 ) {
			kh_value(map->hash, k) = list;
		}
		else {
			assert(0);
		}
	}
	else {
		list = kh_value(map->hash, k);
	}

	if ( list->offset >= list->size ) {
		list->size = list->size * 2;
		list->slots = realloc(list->slots, sizeof( struct length_info ) * list->size);
	}
	struct length_info* slot = &list->slots[list->offset++];

	slot->length = utf8->offset;
	slot->first = utf8->ptr[0];

	qsort(list->slots, list->offset, sizeof( struct length_info ), length_sort);
}

int
word_filter(struct word_map* map, utf8_t* utf8, const char* word, size_t size, struct string* result) {
	size_t tIndex = 0;

	size_t index;
	for ( index = 0; index < utf8->offset; index++ ) {

		khiter_t k = kh_get(word, map->hash, utf8->ptr[index]);
		if ( k == kh_end(map->hash) ) {
			continue;
		}

		struct length_list* list = kh_value(map->hash, k);
		size_t i;
		for ( i = 0; i < list->offset; ++i ) {
			struct length_info* info = &list->slots[i];

			int start = index - ( info->length - 1 );
			if ( info->length > index + 1 ) {
				continue;
			}

			if ( utf8->ptr[start] == info->first ) {
				if ( info->length > 2 ) {
					struct string keyword;
					string_init(&keyword);
					size_t p;
					for ( p = 0; p < info->length; p++ ) {
						string_append_utf8(&keyword, utf8->ptr[start + p]);
					}

					k = kh_get(word_set, map->set, keyword.data);
					string_release(&keyword);

					if ( k == kh_end(map->set) )
						continue;
				}

				if ( !result ) {
					return -1;
				}

				size_t j;
				for ( j = tIndex; j <= index; j++ ) {
					if ( j >= start ) {
						string_append_one(result, '*');
					}
					else {
						string_append_utf8(result, utf8->ptr[j]);
					}
				}

				tIndex = index + 1;
				break;
			}
		}
	}

	if ( tIndex != 0 ) {
		if ( tIndex < utf8->offset ) {
			if ( result ) {
				size_t p;
				for ( p = tIndex; p < utf8->offset; p++ )
					string_append_utf8(result, utf8->ptr[p]);
			}
		}
	}
	else {
		if ( result )
			string_append_str(result, (char*)word, size);
	}

	if ( tIndex == 0 )
		return 0;
	return -1;
}

static int
lrelease(lua_State* L) {
	struct word_map* map = lua_touserdata(L, 1);

	khint_t i;
	for ( i = kh_begin(map->hash); i != kh_end(map->hash); ++i ) {
		if ( !kh_exist(map->hash, i) ) continue;
		struct length_list* list = kh_val(map->hash, i);
		free(list->slots);
		free(list);
	}

	kh_destroy(word, map->hash);

	for ( i = kh_begin(map->set); i != kh_end(map->set); ++i ) {
		if ( !kh_exist(map->set, i) ) continue;
		char* tmp = (char*)kh_key(map->set, i);
		free(tmp);
	}

	kh_destroy(word_set, map->set);
	return 0;
}

static int
ladd(lua_State* L) {
	struct word_map* map = lua_touserdata(L, 1);

	size_t size;
	const char* word = luaL_checklstring(L, 2, &size);
	if ( size == 0 ) {
		luaL_error(L, "error word length == 0");
	}
		
	utf8_t utf8;
	utf8_init(&utf8);
	size_t i;
	for ( i = 0; i < size; ) {
		utf8_int32_t val = 0;
		word = utf8codepoint(word, &val);
		utf8_append(&utf8, val);
		i += utf8codepointsize(val);
	}

	int result;
	kh_put(word_set, map->set, _strdup(word), &result);

	word_add(map, &utf8);

	utf8_release(&utf8);

	return 0;
}

static int
ldelete(lua_State* L) {
	struct word_map* map = lua_touserdata(L, 1);
	size_t size;
	const char* word = luaL_checklstring(L, 2, &size);
	if ( size == 0 ) {
		luaL_error(L, "error word length == 0");
	}
		
	khint_t k = kh_get(word_set, map->set, word);
	if ( k == kh_end(map->set) ) {
		lua_pushboolean(L, 0);
		return 1;
	}

	char* tmp = (char*)kh_key(map->set, k);
	free(tmp);
	kh_del(word_set, map->set, k);

	lua_pushboolean(L, 1);
	return 1;
}

static int
lfilter(lua_State* L) {
	struct word_map* map = lua_touserdata(L, 1);

	size_t size;
	const char* word = luaL_checklstring(L, 2, &size);
	if ( size == 0 )
		luaL_error(L, "error word length == 0");

	int replace = luaL_optinteger(L, 3, 1);

	const char* cursor = word;
	utf8_t utf8;
	utf8_init(&utf8);
	size_t i;
	for ( i = 0; i < size; ) {
		utf8_int32_t val = 0;
		cursor = utf8codepoint(cursor, &val);
		utf8_append(&utf8, val);
		i += utf8codepointsize(val);
	}

	if ( !replace ) {
		int ok = word_filter(map, &utf8, word, size, NULL);
		if ( ok == 0 )
			lua_pushboolean(L, 1);
		else
			lua_pushboolean(L, 0);
		utf8_release(&utf8);
		return 1;
	}

	struct string result;
	string_init(&result);

	int ok = word_filter(map, &utf8, word, size, &result);
	if ( ok == 0 ) {
		lua_pushboolean(L, 1);
		lua_pushvalue(L, 2);
	}
	else {
		lua_pushboolean(L, 0);
		lua_pushstring(L, result.data);
	}

	string_release(&result);
	utf8_release(&utf8);
	return 2;
}

static int
ldump(lua_State* L) {
	struct word_map* map = lua_touserdata(L, 1);
	khint_t i;
	for ( i = kh_begin(map->hash); i != kh_end(map->hash); ++i ) {
		if ( !kh_exist(map->hash, i) ) continue;
		utf8_int32_t code = kh_key(map->hash, i);

		char last[8] = { 0 };
		utf8catcodepoint(last, code, 8);

		struct length_list* list = kh_val(map->hash, i);
		printf("%s\n", last);

		size_t j;
		for ( j = 0; j < list->offset; j++ ) {
			struct length_info* slot = &list->slots[j];
			char first[8] = { 0 };
			utf8catcodepoint(first, slot->first, 8);
			printf("\t%s,%d\n", first, slot->length);
		}
	}
	return 0;
}

static int
lcreate(lua_State* L) {
	struct word_map* map = lua_newuserdata(L, sizeof( *map ));
	map->hash = kh_init(word);
	map->set = kh_init(word_set);

	if ( luaL_newmetatable(L, "meta_filterex") ) {
		const luaL_Reg meta[] = {
			{ "add", ladd },
			{ "delete", ldelete },
			{ "filter", lfilter },
			{ "dump", ldump },
			{ NULL, NULL },
		};
		luaL_newlib(L, meta);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, lrelease);
		lua_setfield(L, -2, "__gc");
	}

	lua_setmetatable(L, -2);
	return 1;
}

__declspec( dllexport ) int
luaopen_filter(lua_State *L) {
	luaL_checkversion(L);

	const luaL_Reg l[] = {
		{ "create", lcreate },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}
