#ifndef LUA_ALLOCATOR_H
#define LUA_ALLOCATOR_H





struct lua_allocator;

struct lua_allocator* lua_allocator_create();
void lua_allocator_delete(struct lua_allocator*);
void lua_allocator_dump(struct lua_allocator*);

void* lua_alloc(void*,void*,size_t,size_t);


#endif