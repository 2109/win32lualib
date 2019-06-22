#ifndef PTO_H
#define PTO_H
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

namespace LuaPto {
	enum eTYPE {
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
		eTYPE type_;

		std::vector<Field*> childs_;

		Field(const char* name, bool array, eTYPE type) {
			name_ = _strdup(name);
			array_ = array;
			type_ = type;
		}

		~Field() {
			free(name_);
			for ( uint32_t i = 0; i < childs_.size(); ++i ) {
				Field* field = childs_[i];
				delete field;
			}
		}

		inline Field* GetField(uint32_t index) {
			if ( index > childs_.size() ) {
				return NULL;
			}
			return childs_[index];
		}
	};

	struct Protocol {
		char* name_;
		std::vector<Field*> fields_;

		Protocol(const char* name) {
			name_ = _strdup(name);
		}

		~Protocol() {
			free(name_);
			for ( uint32_t i = 0; i < fields_.size(); ++i ) {
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
			for ( uint32_t i = 0; i < ptos_.size(); ++i ) {
				Protocol* pto = ptos_[i];
				if ( pto ) {
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
			if ( id >= 0xffff ) {
				return NULL;
			}
			return ptos_[id];
		}
	};
}
#endif