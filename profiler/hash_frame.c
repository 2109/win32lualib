#include "profiler.h"
#include <assert.h>

void 
hash_frame_set(hash_frame_t* self, const char* name, frame_t* frame) {
	int ok;
	khiter_t k = kh_put(frame, self, _strdup(name), &ok);
	assert(ok == 1 || ok == 2);
	kh_value(self, k) = frame;
}

void 
hash_frame_del(hash_frame_t* self, const char* name) {
	khiter_t k = kh_get(frame, self, name);
	assert(k != kh_end(self));
	kh_del(frame, self, k);
}

frame_t*
hash_frame_find(hash_frame_t* self, const char* name) {
	khiter_t k = kh_get(frame, self, name);
	if (k == kh_end(self)) {
		return NULL;
	}
	return kh_value(self, k);
}

void 
hash_frame_free(hash_frame_t *self) {
	const char* name;
	frame_t* fm;
	hash_frame_foreach(self, name, fm, {
		free((void*)name);
		free(fm);
	});
	kh_destroy(frame, self);
}