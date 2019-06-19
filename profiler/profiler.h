
#ifndef PROFILER_MEMORY_H
#define PROFILER_MEMORY_H

#ifdef _WIN32
#include <WinSock2.h>
#define inline __inline
#else
#include <sys/time.h>
#endif


#include "khash.h"
#include "kstring.h"

typedef struct frame {
	const char* name;
	const char* source;
	int linedefined;
	
	int invoke_count;
	double invoke_diff;
	double invoke_start;
	double invoke_cost;

	int alloc_total;
	int alloc_count;
	struct frame* next;
	struct frame* prev;

	struct frame* free_next;
} frame_t;


KHASH_MAP_INIT_STR(frame, struct frame*);

typedef khash_t(frame) hash_frame_t;

#define hash_frame_new() kh_init(frame)

#define hash_frame_foreach(self, k, v, code) kh_foreach(self, k, v, code)

void hash_frame_set(hash_frame_t *self, const char*, struct frame*);
void hash_frame_del(hash_frame_t *self, const char*);
struct frame* hash_frame_find(hash_frame_t *self, const char*);
void hash_frame_free(hash_frame_t *self);


#endif