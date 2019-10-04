#ifndef PTO_DECODE_H
#define PTO_DECODE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "lua.hpp"

namespace LuaPto {
	struct Decoder {
		lua_State* L_;
		const char* ptr_;
		int offset_;
		int size_;

		Decoder(lua_State* L, const char* ptr, int size) {
			L_ = L;
			ptr_ = ptr;
			size_ = size;
			offset_ = 0;
		}

		inline void Read(uint8_t* val, int size) {
			if (size_ - offset_ < size) {
				luaL_error(L_, "invalid message");
			}
			memcpy(val, ptr_ + offset_, size);
			offset_ += size;
		}

		inline const char* Read(uint16_t* size) {
			*size = Read<uint16_t>();
			if (size_ - offset_ < (int)*size) {
				luaL_error(L_, "invalid message");
			}
			const char* result = ptr_ + offset_;
			offset_ += *size;
			return result;
		}

		template<typename T>
		inline T Read() {
			if (sizeof(T) > (size_t)(size_ - offset_)) {
				luaL_error(L_, "invalid message");
			}
			T val = *((T*)&ptr_[offset_]);
			offset_ += sizeof(T);
			return val;
		}
	};

	template<>
	inline int64_t Decoder::Read() {
		uint8_t tag = Read<uint8_t>();

		if (tag == 0) {
			return 0;
		}

		int length = tag >> 1;

		uint64_t value = 0;
		Read((uint8_t*)&value, length);

		return (tag & 0x1) == 1 ? value : -(lua_Integer)value;
	}
}
#endif