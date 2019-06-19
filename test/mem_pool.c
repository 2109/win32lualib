#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef struct pool_node {
	struct pool_node* next;
} pool_node_t;

typedef struct free_node {
	struct free_node* next;
	int used;
} free_node_t;

typedef struct mem_pool {
	free_node_t* freelist;
	pool_node_t* pool;
	int pool_size;
	int node_size;
	int node_count;
	int node_used;
} mem_pool_t;

void
create_pool(mem_pool_t* mem_pool);

mem_pool_t*
pool_create(int size,int count) {
	if (size < 1) {
		size = 1;
	}
	if (count < 64) {
		count = 64;
	}
	mem_pool_t* mem_pool = malloc(sizeof(*mem_pool));
	memset(mem_pool,0,sizeof(*mem_pool));
	mem_pool->node_count = count;
	mem_pool->node_size = size;
	mem_pool->pool_size = sizeof(pool_node_t) + (size + sizeof(free_node_t)) * count;

	create_pool(mem_pool);
	return mem_pool;
}

void
pool_delete(mem_pool_t* mem_pool) {
	pool_node_t* cursor = mem_pool->pool;
	while(cursor) {
		pool_node_t* tmp = cursor;
		cursor = cursor->next;
		free(tmp);
	}
	free(mem_pool);
}

void
create_pool(mem_pool_t* mem_pool) {
	pool_node_t* pool_node = malloc(mem_pool->pool_size);
	memset(pool_node,0,mem_pool->pool_size);
	pool_node->next = mem_pool->pool;
	mem_pool->pool = pool_node;

	mem_pool->freelist = (free_node_t*)((pool_node_t*)pool_node + 1);

	int i;
	for(i=1;i<mem_pool->node_count;i++) {
		free_node_t* fn = (free_node_t*)((char*)mem_pool->freelist+mem_pool->node_size + sizeof(free_node_t));
		fn->next = mem_pool->freelist;
		mem_pool->freelist = fn;
	}
}

void*
pool_pop(mem_pool_t* mem_pool) {
	if (!mem_pool->freelist) {
		create_pool(mem_pool);
	}
	mem_pool->node_used++;
	free_node_t* fn = mem_pool->freelist;
	assert(fn->used == 0);
	mem_pool->freelist = fn->next;
	fn->used = 1;
	return (void*)((char*)fn+sizeof(free_node_t));
}

void
pool_push(mem_pool_t* mem_pool,void* ptr) {
	free_node_t* fn = (free_node_t*)((char*)ptr - sizeof(free_node_t));
	assert(fn->used == 1);
	fn->used = 0;
	fn->next = mem_pool->freelist;
	mem_pool->freelist = fn;
	mem_pool->node_used--;
}

void
pool_dump(mem_pool_t* mem_pool) {
	int pool_used = 0;
	pool_node_t* cursor = mem_pool->pool;
	while(cursor) {
		pool_used++;
		cursor = cursor->next;
	}

	int pool_total = pool_used * mem_pool->pool_size;
	int node_used = mem_pool->node_used * (mem_pool->node_size + sizeof(free_node_t));

	if (pool_total <= 1024) {
		fprintf(stderr,"total:%db,used:%db,node size:%d\n",pool_total,node_used,mem_pool->node_size);	
	} else {
		fprintf(stderr,"total:%dkb,used:%dkb,node size:%d\n",pool_total/1024,node_used/1024,mem_pool->node_size);
	}
	
}