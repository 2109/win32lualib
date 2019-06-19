

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "mem_pool.h"


#define inline __inline

struct lua_allocator {
	struct mem_pool* freelist[4];
};

static int where[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0,//8
	1, 1, 1, 1, 1, 1, 1, 1,//16
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,//32
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,//64
};

struct lua_allocator*
lua_allocator_create() {
	struct lua_allocator* la = malloc(sizeof(*la));
	memset(la,0,sizeof(*la));
	return la;
}

void
lua_allocator_delete(struct lua_allocator* la) {
	int i ;
	for(i = 0;i < 4 ;i++) {
		struct mem_pool* pool = la->freelist[i];
		if (pool) {
			pool_delete(pool);
		}
	}
}

void
lua_allocator_dump(struct lua_allocator* la) {
	int i ;
	for(i = 0;i < 4 ;i++) {
		struct mem_pool* pool = la->freelist[i];
		if (pool) {
			pool_dump(pool);
		}
	}
}

inline void*
malloc_small(struct lua_allocator* la,int index) {
	size_t size = 1 << (index + 3);
	struct mem_pool* pool = la->freelist[index];
	if (!pool) {
		pool = pool_create(size,128);
		la->freelist[index] = pool;
	}
	return pool_pop(pool);
}

inline void
free_small(struct lua_allocator* la,void* ptr,int index) {
	struct mem_pool* pool = la->freelist[index];
	assert(pool != NULL);
	pool_push(pool,ptr);
}

inline void*
extend_small(struct lua_allocator* la,void* ptr,size_t osize,size_t nsize) {
	int oidx = where[osize];
	int nidx = where[nsize];
	if (oidx == nidx)
		return ptr;

	void* result = malloc_small(la,nidx);
	memcpy(result,ptr,osize < nsize ? osize:nsize);
	free_small(la,ptr,oidx);
	return result;
}

void*
lua_alloc(void* ud,void* ptr,size_t osize,size_t nsize) {
	if (!ptr) {
		if (nsize > 64) {
			return malloc(nsize);
		}
		if (nsize == 0) {
			return NULL;
		}
		return malloc_small(ud,where[nsize]);
	}
	else if(nsize == 0) {
		if (osize > 64) {
			free(ptr);
			return NULL;
		}
		free_small(ud,ptr,where[osize]);
		return NULL;
	}
	else {
		if (osize > 64) {
			if (nsize > 64) {
				return realloc(ptr,nsize);
			} else {
				void* result = malloc_small(ud,where[nsize]);
				memcpy(result,ptr,nsize);
				free(ptr);
				return result;
			}
		}

		if (nsize > 64) {
			void* result = malloc(nsize);
			memcpy(result,ptr,osize);
			free_small(ud,ptr,where[osize]);
			return result;
		}
		else
			return extend_small(ud,ptr,osize,nsize);
	}
	assert(0);
	return NULL;
}