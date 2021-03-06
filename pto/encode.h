﻿#ifndef PTO_ENCODE_H
#define PTO_ENCODE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "lua.hpp"



namespace LuaPto {
	// #define BUFFER_SIZE 128

	struct Encoder {
		lua_State* L_;
		char* ptr_;
		int offset_;
		int size_;

		static char* sBuff_;
		static int sSize_;

		Encoder(lua_State* L) : L_(L) {
			ptr_ = sBuff_;
			offset_ = 0;
			size_ = sSize_;
		}

		~Encoder() {
		}

		inline void Reserve(int sz) {
			if (offset_ + sz <= size_) {
				return;
			}
			luaL_error(L_, "encode pto size limited!");
		}

		inline void Append(void* data, int size) {
			Reserve(size);
			memcpy(ptr_ + offset_, data, size);
			offset_ += size;
		}

		inline void Append(const char* str, int sz) {
			Append<uint16_t>(sz);
			Append((void*)str, sz);
		}

		template<typename T>
		inline void Append(T val) {
			Append(&val, sizeof(T));
		}
	};

	template<>
	inline void Encoder::Append(int64_t val) {
		if (val == 0) {
			Append((uint8_t)0);
			return;
		}
		uint64_t value;
		uint8_t positive = 0;
		if (val < 0) {
			positive = 0;
			value = -val;
		} else {
			positive = 1;
			value = val;
		}

		int length;
		if (value <= 0xff) {
			length = 1;
		} else if (value <= 0xffff) {
			length = 2;
		} else if (value <= 0xffffff) {
			length = 3;
		} else if (value <= 0xffffffff) {
			length = 4;
		} else if (value <= 0xffffffffff) {
			length = 5;
		} else if (value <= 0xffffffffffff) {
			length = 6;
		} else {
			length = 7;
		}

		uint8_t tag = length;
		tag = (tag << 1) | positive;

		uint8_t data[8] = { 0 };
		data[0] = tag;
		memcpy(&data[1], &value, length);

		Append(data, length + 1);
	}
}

#endif