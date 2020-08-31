#ifndef PTO_H
#define PTO_H
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "lua.hpp"

#define strdup _strdup

namespace LuaPto {

	struct Field;
	struct Encoder;
	struct Decoder;

	typedef void(*EncodeFunc)(lua_State* L, Field* field, Encoder* encoder, int index, int depth);
	typedef void(*DecodeFunc)(lua_State* L, Field* field, Decoder* decoder, int index, int depth);

	enum eType {
		Bool = 0,
		Byte,
		Short,
		Int,
		Float,
		Double,
		String,
		Pto
	};

	static const char* kTYPE_NAME[] = {"bool", "byte", "short", "int", "float", "double", "string", "table"};

	struct Field {
		char* name_;
		bool array_;
		int type_;

		EncodeFunc encodeFunc_;
		DecodeFunc decodeFunc_;

		std::vector<Field*> childs_;

		Field(const char* name, bool array, int type, EncodeFunc encodeFunc, DecodeFunc decodeFunc) {
			name_ = strdup(name);
			array_ = array;
			type_ = type;
			encodeFunc_ = encodeFunc;
			decodeFunc_ = decodeFunc;
		}

		~Field() {
			free(name_);
			for (uint32_t i = 0; i < childs_.size(); ++i) {
				Field* field = childs_[i];
				delete field;
			}
		}

		inline Field* GetField(uint32_t index) {
			if (index > childs_.size()) {
				return NULL;
			}
			return childs_[index];
		}
	};

	struct Protocol {
		uint16_t id_;
		char* name_;
		std::vector<Field*> fields_;

		Protocol(uint16_t id, const char* name) {
			id_ = id;
			name_ = strdup(name);
		}

		~Protocol() {
			free(name_);
			for (uint32_t i = 0; i < fields_.size(); ++i) {
				Field* field = fields_[i];
				delete field;
			}
		}

		void AddField(struct Field* field) {
			fields_.push_back(field);
		}

		inline Field* GetField(int index) {
			return fields_[index];
		}
	};

	struct Context {
		std::vector<Protocol*> ptos_;

		Context() {
			ptos_.resize(0xffff);
		}

		~Context() {
			for (uint32_t i = 0; i < ptos_.size(); ++i) {
				Protocol* pto = ptos_[i];
				if (pto) {
					delete pto;
				}
			}
		}

		void AddPto(uint16_t id, Protocol* pto) {
			if (ptos_[id]) {
				Protocol* opto = ptos_[id];
				delete opto;
			}
			ptos_[id] = pto;
		}

		inline Protocol* GetPto(uint16_t id) {
			if (id >= 0xffff) {
				return NULL;
			}
			return ptos_[id];
		}
	};
}
#endif