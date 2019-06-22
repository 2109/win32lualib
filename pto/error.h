#ifndef PTO_ERROR_H
#define PTO_ERROR_H
#include <exception>
#include <string>
#include "lua.hpp"
#include "format.h"
#include "pto.h"

namespace LuaPto {
	struct BadPto : std::exception {
		std::string reason_;
		BadPto() : reason_("") {
		}

		BadPto(const char* str) : reason_(str) {
		}

		virtual const char* what() const {
			return reason_.c_str();
		}
	};

	struct BadArray : public BadPto {
		BadArray(Field* field, const char* vt) {
			reason_ = fmt::format("{} expect table,not {}", field->name_, vt);
		}
	};

	struct BadSize : public BadPto {
		BadSize(Field* field, size_t size) {
			reason_ = fmt::format("{} array size more than 0xff:{}", field->name_, size);
		}
	};

	struct BadField : public BadPto {
		BadField(Field* field, const char* vt) {
			if ( field->array_ ) {
				reason_ = fmt::format("{} array member expect {},not {}", field->name_, kTYPE_NAME[field->type_], vt);
			}
			else {
				reason_ = fmt::format("{} expect {},not {}", field->name_, kTYPE_NAME[field->type_], vt);
			}
		}
	};

	struct BadByte : public BadPto {
		BadByte(Field* field, lua_Integer val) {
			if ( field->array_ ) {
				reason_ = fmt::format("{} array member byte out of range,{}", field->name_, val);
			}
			else {
				reason_ = fmt::format("{} byte out of range,{}", field->name_, val);
			}
		}
	};

	struct BadInt : public BadPto {
		BadInt(Field* field, lua_Integer val) {
			if ( field->array_ ) {
				reason_ = fmt::format("{} array member int out of range,{}", field->name_, val);
			}
			else {
				reason_ = fmt::format("{} int out of range,{}", field->name_, val);
			}
		}
	};

	struct BadShort : public BadPto {
		BadShort(Field* field, lua_Integer val) {
			if ( field->array_ ) {
				reason_ = fmt::format("{} array member short out of range,{}", field->name_, val);
			}
			else {
				reason_ = fmt::format("{} short out of range,{}", field->name_, val);
			}
		}
	};

	struct BadString : public BadPto {
		BadString(Field* field, size_t size) {
			reason_ = fmt::format("{} string size more than 0xffff:{}", field->name_, size);
		}
	};

	struct BadType : public BadPto {
		BadType(Field* field) {
			reason_ = fmt::format("{} unknown type:{}", field->name_, field->type_);
		}
	};
}
#endif