#ifndef PTO_DECODE_H
#define PTO_DECODE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "lua.hpp"
#include "error.h"

namespace LuaPto {
	struct Decoder {
		const char* ptr_;
		int offset_;
		int size_;

		Decoder(const char* ptr, int size) {
			ptr_ = ptr;
			size_ = size;
			offset_ = 0;
		}

		inline void Read(uint8_t* val, int size) {
			if ( size_ - offset_ < size ) {
				throw new BadPto("invalid message");
			}
			memcpy(val, ptr_ + offset_, size);
			offset_ += size;
		}

		inline const char* Read(uint16_t* size) {
			*size = Read<uint16_t>();
			if ( size_ - offset_ < *size ) {
				throw new BadPto("invalid message");
			}
			const char* result = ptr_ + offset_;
			offset_ += *size;
			return result;
		}

		template<typename T>
		inline T Read()  {
			if ( sizeof(T) > size_ - offset_ ) {
				throw new BadPto("invalid message");
			}
			T val = *((T*)&ptr_[offset_]);
			offset_ += sizeof(T);
			return val;
		}
	};

	template<>
	inline int64_t Decoder::Read() {
		uint8_t tag = Read<uint8_t>();

		if ( tag == 0 ) {
			return 0;
		}

		int length = tag >> 1;

		uint64_t value = 0;
		Read((uint8_t*)&value, length);

		return (tag & 0x1) == 1 ? value : -(lua_Integer)value;
	}
}
#endif