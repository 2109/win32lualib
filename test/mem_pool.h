#ifndef MEM_POOL_H
#define MEM_POOL_H



struct mem_pool;

struct mem_pool* pool_create(int size,int count);
void pool_delete(struct mem_pool* mem_pool);

void* pool_pop(struct mem_pool* mem_pool);
void pool_push(struct mem_pool* mem_pool,void* ptr);

void pool_dump(struct mem_pool* mem_pool);




#endif