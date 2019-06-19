
#include "string.h"


#define roundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))

string_t*
string_new(char* str, size_t size) {
	string_t* string = malloc(sizeof(*string));
	memset(string, 0, sizeof(*string));
	string_init(string, str, size);
	return string;
}

void
string_init(string_t* string, char* str, size_t size) {
	size_t rsize = roundup32(size);
	char* raw = malloc(rsize);
	memset(raw, 0, rsize);

	if (str) {
		string->kstr.l = size;
		memcpy(raw, str, size);
	} else {
		string->kstr.l = 0;
	}
	string->kstr.m = rsize;
	string->kstr.s = raw;
}

void
string_release(string_t* string) {
	char* str = ks_release(&string->kstr);
	free(str);
}
void
string_free(string_t* string) {
	string_release(string);
	free(string);
}

char*
string_str(string_t* string) {
	return ks_str(&string->kstr);
}

size_t
string_length(string_t* string) {
	return ks_len(&string->kstr);
}

int
string_append_lstr(string_t* string, const char* ptr, int size) {
	return kputsn(ptr, size, &string->kstr);
}

int
string_append_str(string_t* string, const char* ptr) {
	return string_append_lstr(string, ptr, strlen(ptr));
}

int
string_append_char(string_t* string, char c) {
	return kputc((int)c, &string->kstr);
}

int
string_append_word(string_t* string, int c) {
	return kputw(c, &string->kstr);
}

int
string_append_uword(string_t* string, unsigned c) {
	return kputuw(c, &string->kstr);
}

int
string_append_long(string_t* string, long c) {
	return kputl(c, &string->kstr);
}


char*
string_concat(string_t** list, int size, int* outsize) {
	string_t result;
	string_init(&result, NULL, 0);

	int i;
	for(i = 0; i < size;i++) {
		string_t* str = list[i];
		string_append_lstr(&result, str->kstr.s, str->kstr.l);
	}

	char* rstr = malloc(result.kstr.l);
	memcpy(rstr, result.kstr.s, result.kstr.l);
	*outsize = result.kstr.l;

	string_release(&result);
	return rstr;
}