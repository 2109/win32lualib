#ifndef CONVERT_H
#define CONVERT_H

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdint.h>



#ifdef __cplusplus
extern "C" {
#endif
	char* i64toa_fast(int64_t value, char* buffer);
	char* i32toa_fast(int32_t value, char* buffer);
	void dtoa_fast(double value, char* buffer);

#ifdef __cplusplus
};
#endif
#endif 