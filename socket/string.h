#ifndef STRING_H
#define STRING_H
#include <stdlib.h>
#include <stdio.h>
#include "kstring.h"


#define DEFAULT_SIZE	64

struct string {
	kstring_t kstr;
};

typedef struct string string_t;

struct string* string_new(char* str, size_t size);
void string_init(struct string* string, char* str, size_t size);
void string_release(struct string* string);
void string_free(struct string* string);

char* string_str(struct string* string);
size_t string_length(struct string* string);

int string_append_lstr(struct string* string, const char* ptr, int size);
int string_append_str(struct string* string, const char* ptr);
int string_append_char(struct string* string, char c);
int string_append_word(struct string* string, int c);

int string_append_uword(struct string* string, unsigned c);
int string_append_long(struct string* string, long c);


char* string_concat(struct string** list, int size, int* outsize);



#endif