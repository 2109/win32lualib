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

#ifdef _MSC_VER
#define EXPORT __declspec( dllexport )
#define inline __inline
#else
#define EXPORT
#endif

#define PHASE_SEARCH 0
#define PHASE_MATCH 1

struct word_tree;

KHASH_MAP_INIT_INT(word, struct word_tree*);

typedef khash_t(word) tree_hash_t;

typedef struct word_tree {
	tree_hash_t* hash;
	uint8_t tail;
} tree_t;

typedef struct utf8_node {
	utf8_int32_t utf8;
	struct utf8_node* next;
} utf8_node_t;

typedef struct utf8_link {
	utf8_node_t* head;
	utf8_node_t* tail;
} utf8_link_t;

typedef struct search_ctx {
	lua_State* L;
	int index;
	const char* prefix;
	size_t prefix_size;
	char* buffer;
	size_t size;
	size_t offset;
} search_ctx_t;

void
tree_set(tree_hash_t* hash, utf8_int32_t utf8, tree_t* tree) {
	int ok;
	khiter_t k = kh_put(word, hash, utf8, &ok);
	assert(ok == 1 || ok == 2);
	kh_value(hash, k) = tree;
}

tree_t*
tree_get(tree_hash_t* hash, utf8_int32_t utf8) {
	khiter_t k = kh_get(word, hash, utf8);
	if ( k == kh_end(hash) ) {
		return NULL;
	}
	return kh_value(hash, k);
}

void
word_add(tree_t* root_tree, const char* word, size_t size) {
	tree_t* tree = root_tree;
	size_t i;
	for ( i = 0; i < size; ) {
		utf8_int32_t utf8 = 0;
		word = utf8codepoint(word, &utf8);
		int length = utf8codepointsize(utf8);
		i += length;

		tree_t* child_tree = tree_get(tree->hash, utf8);
		if ( !child_tree ) {
			child_tree = malloc(sizeof( *tree ));
			child_tree->tail = 0;
			child_tree->hash = kh_init(word);

			tree_set(tree->hash, utf8, child_tree);

			tree = child_tree;
		}
		else {
			tree = child_tree;
		}

		if ( i == size ) {
			tree->tail = 1;
		}
	}
}

void
word_delete(tree_t* root_tree, const char* word, size_t size) {
	tree_t* tree = root_tree;
	size_t i;
	for ( i = 0; i < size; ) {
		utf8_int32_t utf8 = 0;
		word = utf8codepoint(word, &utf8);
		int length = utf8codepointsize(utf8);
		i += length;

		tree = tree_get(tree->hash, utf8);
		if ( !tree ) {
			return;
		}
	}
	tree->tail = 0;
}

tree_t*
word_search(tree_t* root_tree, const char* word, size_t size) {
	tree_t* tree = root_tree;
	size_t i;
	for ( i = 0; i < size; ) {
		utf8_int32_t utf8 = 0;
		word = utf8codepoint(word, &utf8);
		int length = utf8codepointsize(utf8);
		i += length;

		tree = tree_get(tree->hash, utf8);
		if ( !tree ) {
			return NULL;
		}
	}
	return tree;
}

int
word_filter(tree_t* root_tree, const char* source, size_t size, luaL_Buffer* buffer) {
	tree_t* tree = root_tree;

	int start = 0;
	int rollback = -1;
	int founded = 0;
	int filter_length = -1;
	int filter_slider = -1;

	int phase = PHASE_SEARCH;

	size_t position = 0;
	while ( position < size ) {
		utf8_int32_t utf8 = 0;
		utf8codepoint(source + position, &utf8);
		int length = utf8codepointsize(utf8);
		position += length;

		switch ( phase ) {
			case PHASE_SEARCH: {
								   tree = tree_get(tree->hash, utf8);
								   if ( tree ) {
									   phase = PHASE_MATCH;
									   start = position - length;
									   rollback = position;
									   filter_length = 1;
									   filter_slider = 1;
									   founded = 0;
									   if ( tree->tail ) {
										   if ( !buffer ) {
											   return -1;
										   }
										   founded = 1;
									   }
								   }
								   else {
									   tree = root_tree;
									   if ( buffer ) {
										   char word[8] = { 0 };
										   utf8catcodepoint(word, utf8, 8);
										   luaL_addlstring(buffer, word, length);
									   }
								   }
								   break;
			}
			case PHASE_MATCH: {
								  if ( length == 1 ) {
									  if ( isspace(utf8) || iscntrl(utf8) || ispunct(utf8) ) {
										  ++filter_slider;
										  continue;
									  }
								  }
								  tree = tree_get(tree->hash, utf8);
								  if ( tree ) {
									  ++filter_slider;
									  if ( tree->tail ) {
										  if ( !buffer ) {
											  return -1;
										  }
										  filter_length = filter_slider;
										  rollback = position;
										  founded = 1;
									  }
								  }
								  else {
									  if ( founded == 1 ) {
										  //匹配成功
										  if ( !buffer ) {
											  return -1;
										  }

										  int i;
										  for ( i = 0; i < filter_length; i++ ) {
											  luaL_addchar(buffer, '*');
										  }
									  }
									  else if ( buffer ) {
										  //匹配失败
										  luaL_addlstring(buffer, source + start, rollback - start);
									  }
									  //回滚
									  position = rollback;
									  tree = root_tree;
									  phase = PHASE_SEARCH;
								  }
								  break;
			}
		}
	}

	if ( !buffer ) {
		return 0;
	}

	if ( phase == PHASE_MATCH ) {
		if ( founded == 1 ) {
			int i;
			for ( i = 0; i < filter_length; i++ ) {
				luaL_addchar(buffer, '*');
			}
			luaL_addlstring(buffer, source + rollback, size - rollback);
		}
		else {
			luaL_addlstring(buffer, source + start, position - start);
		}
	}

	return 0;
}

static int
lcreate(lua_State* L) {
	tree_t* tree = lua_newuserdata(L, sizeof( *tree ));
	tree->hash = kh_init(word);
	luaL_newmetatable(L, "meta_trie");
	lua_setmetatable(L, -2);
	return 1;
}

void
release(utf8_int32_t utf8, tree_t* tree) {
	if ( tree->hash ) {
		tree_t* child = NULL;
		kh_foreach(tree->hash, utf8, child, release(utf8, child));
		kh_destroy(word, tree->hash);
	}
	free(tree);
}

static int
lrelease(lua_State* L) {
	tree_t* tree = lua_touserdata(L, 1);

	tree_t* child = NULL;
	utf8_int32_t utf8;
	kh_foreach(tree->hash, utf8, child, release(utf8, child));
	kh_destroy(word, tree->hash);

	return 0;
}

static int
ladd(lua_State* L) {
	tree_t* tree = lua_touserdata(L, 1);
	size_t size;
	const char* word = lua_tolstring(L, 2, &size);
	word_add(tree, word, size);
	return 0;
}

static int
ldelete(lua_State* L) {
	tree_t* tree = lua_touserdata(L, 1);
	size_t size;
	const char* word = lua_tolstring(L, 2, &size);
	word_delete(tree, word, size);
	return 0;
}

static int
lfilter(lua_State* L) {
	tree_t* tree = lua_touserdata(L, 1);
	size_t size;
	const char* word = luaL_checklstring(L, 2, &size);
	int replace = luaL_optinteger(L, 3, 1);

	if ( !replace ) {
		int ok = word_filter(tree, word, size, NULL);
		lua_pushboolean(L, ok == 0);
		return 1;
	}

	luaL_Buffer buffer;
	luaL_buffinit(L, &buffer);
	word_filter(tree, word, size, &buffer);
	luaL_pushresult(&buffer);
	return 1;
}

static void
buffer_init(search_ctx_t* search_ctx) {
	search_ctx->offset = 0;
}

static void
buffer_add(search_ctx_t* search_ctx, const char* str, size_t l) {
	if ( search_ctx->offset + l >= search_ctx->size ) {
		size_t nsize = search_ctx->size * 2;
		if ( nsize < search_ctx->offset + l ) {
			nsize = search_ctx->offset + l;
		}
		search_ctx->buffer = (char*)realloc(search_ctx->buffer, nsize);
		search_ctx->size = nsize;
	}
	memcpy(search_ctx->buffer + search_ctx->offset, str, l);
	search_ctx->offset += l;
}

static void
merge_word(search_ctx_t* search_ctx, utf8_link_t* link, tree_t* tree) {
	if ( tree->tail == 1 ) {
		buffer_init(search_ctx);
		buffer_add(search_ctx, search_ctx->prefix, search_ctx->prefix_size);
		utf8_node_t* cursor = link->head;
		while ( cursor ) {
			char word[8] = { 0 };
			utf8catcodepoint(word, cursor->utf8, 8);
			buffer_add(search_ctx, word, strlen(word));
			cursor = cursor->next;
		}
		lua_pushlstring(search_ctx->L, search_ctx->buffer, search_ctx->offset);
		lua_rawseti(search_ctx->L, -2, search_ctx->index++);
	}

	utf8_int32_t utf8;
	tree_t* child;

	kh_foreach(tree->hash, utf8, child, {
		utf8_node_t node;
		node.utf8 = utf8;
		node.next = NULL;

		utf8_node_t* tail = link->tail;

		tail->next = &node;
		link->tail = tail->next;

		merge_word(search_ctx, link, child);

		tail->next = NULL;
		link->tail = tail;
	});
}

static int
lsearch(lua_State* L) {
	tree_t* tree = lua_touserdata(L, 1);
	size_t size;
	const char* word = luaL_checklstring(L, 2, &size);

	search_ctx_t search_ctx;
	search_ctx.L = L;
	search_ctx.index = 1;
	search_ctx.prefix = word;
	search_ctx.prefix_size = size;
	search_ctx.size = 128;
	search_ctx.offset = 0;
	search_ctx.buffer = (char*)malloc(search_ctx.size);

	lua_newtable(L);

	tree = word_search(tree, word, size);
	if ( !tree ) {
		free(search_ctx.buffer);
		return 1;
	}

	utf8_int32_t utf8;
	tree_t* child;

	kh_foreach(tree->hash, utf8, child, {
		utf8_node_t node;
		node.utf8 = utf8;
		node.next = NULL;

		utf8_link_t link;
		link.head = &node;
		link.tail = &node;

		merge_word(&search_ctx, &link, child);
	});
	free(search_ctx.buffer);
	return 1;
}

static int
lsplit(lua_State* L) {
	size_t size;
	const char* word = lua_tolstring(L, 1, &size);

	lua_newtable(L);

	int index = 1;
	size_t i;
	for ( i = 0; i < size; ) {
		utf8_int32_t utf8 = 0;
		char* next = utf8codepoint(word, &utf8);
		size_t length = utf8codepointsize(utf8);
		lua_pushlstring(L, word, length);
		lua_seti(L, -2, index++);
		word = next;
		i += length;
	}

	return 1;
}

void
dump(utf8_int32_t utf8, tree_t* tree, int depth, FILE* f) {
	int i;
	for ( i = 0; i < depth; i++ ) {
		const char* t = "  ";
		printf(t);
		if ( f ) {
			fwrite(t, strlen(t), 1, f);
		}
	}

	char word[8] = { 0 };
	utf8catcodepoint(word, utf8, 8);
	word[strlen(word)] = '\n';

	printf("%s", word);
	if ( f ) {
		fwrite(word, strlen(word), 1, f);
	}

	depth++;
	if ( tree->hash ) {
		tree_t* child = NULL;
		kh_foreach(tree->hash, utf8, child, dump(utf8, child, depth, f));
	}
}

static int
ldump(lua_State* L) {
	tree_t* tree = lua_touserdata(L, 1);
	const char* file = NULL;
	if ( lua_type(L, 2) == LUA_TSTRING ) {
		file = lua_tostring(L, 2);
	}

	tree_t* child = NULL;
	utf8_int32_t utf8;
	int depth = 0;

	FILE* f = NULL;
	if ( file ) {
		f = fopen(file, "w");
	}

	kh_foreach(tree->hash, utf8, child, dump(utf8, child, depth, f));

	if ( file ) {
		fclose(f);
	}

	return 0;
}

EXPORT int
luaopen_trie(lua_State *L) {
	luaL_checkversion(L);

	luaL_newmetatable(L, "meta_trie");
	const luaL_Reg meta[] = {
		{ "add", ladd },
		{ "delete", ldelete },
		{ "filter", lfilter },
		{ "search", lsearch },
		{ "dump", ldump },
		{ NULL, NULL },
	};
	luaL_newlib(L, meta);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, lrelease);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);


	const luaL_Reg l[] = {
		{ "create", lcreate },
		{ "split_utf8", lsplit },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}
