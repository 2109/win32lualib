#include <vector>
#include <stdint.h>
#include "format.h"
#include "lua.hpp"

#define MAX_DEPTH 128
#define BUFFER_SIZE 128
#define MAX_INT 	0xffffffffffffff

#define FTYPE_BOOL 		0
#define FTYPE_SHORT     1
#define FTYPE_INT 		2
#define FTYPE_FLOAT 	3
#define FTYPE_DOUBLE 	4
#define FTYPE_STRING 	5
#define FTYPE_PROTOCOL 	6

struct PtoException : std::exception {
	std::string reason_;
};

struct ArrayMemberException : public PtoException {
	ArrayMemberException(const char* field, const char* vt) {
		reason_ = fmt::format("field:{} expect table,not {}", field, vt);
	}
};

struct ArraySizeException : public PtoException {
	ArraySizeException(const char* field) {
		reason_ = fmt::format("field:{} array size more than 0xffff", field);
	}
};

struct FieldException : public PtoException {
	FieldException(const char* field, const char* expect, const char* vt) {
		reason_ = fmt::format("field:{} expect {},not {}", field, expect, vt);
	}
};

struct ArrayFieldException : public PtoException {
	ArrayFieldException(const char* field, const char* expect, const char* vt) {
		reason_ = fmt::format("field:{} array member expect {},not {}", field, expect, vt);
	}
};

struct IntLimitException : public PtoException {
	IntLimitException(const char* field, lua_Integer val, bool array) {
		if ( array ) {
			reason_ = fmt::format("field:{} array member int out of range,{}", field, val);
		}
		else {
			reason_ = fmt::format("field:{} int out of range,{}", field, val);
		}
	}
};

struct StringLimitException : public PtoException {
	StringLimitException(const char* field, size_t size) {
		reason_ = fmt::format("field:{} string size more than 0xffff:{}", field, size);
	}
};

struct TypeException : public PtoException {
	TypeException(const char* field, int type) {
		reason_ = fmt::format("unknown field:{},type:{}", field, type);
	}
};

struct ErrorDecode : public PtoException {
	ErrorDecode() {
		reason_ = "invalid message";
	}
};

struct TooDepthException : public PtoException {
	TooDepthException(bool encode) {
		if ( encode ) {
			reason_ = "pto encode too depth";
		}
		else {
			reason_ = "pto decode too depth";
		}
	}
};

struct Field {
	char* name_;
	bool array_;
	int type_;

	std::vector<Field*>* childs_;

	Field(const char* name, bool array, int type) {
		name_ = _strdup(name);
		array_ = array;
		type_ = type;
		childs_ = NULL;
	}

	~Field() {
		free(name_);
		if ( childs_ ) {
			for ( int i = 0; i < childs_->size(); ++i ) {
				Field* field = (*childs_)[i];
				delete field;
			}
			delete childs_;
		}
	}

	void AddField(struct Field* field) {
		if ( !childs_ ) {
			childs_ = new std::vector<Field*>();
		}
		childs_->push_back(field);
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
		for ( int i = 0; i < fields_.size(); ++i ) {
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

struct ProtocolContext {
	std::vector<Protocol*> ptos_;

	ProtocolContext() {
		ptos_.resize(0xffff);
	}

	~ProtocolContext() {
		for ( int i = 0; i < ptos_.size(); ++i ) {
			Protocol* pto = ptos_[i];
			delete pto;
		}
	}

	void AddPto(uint16_t id, Protocol* pto) {
		ptos_[id] = pto;
	}
};

struct PtoWriter {
	char* ptr_;
	int offset_;
	int size_;
	char init_[BUFFER_SIZE];

	PtoWriter() {
		ptr_ = init_;
		offset_ = 0;
		size_ = BUFFER_SIZE;
	}

	~PtoWriter() {
		if ( ptr_ != init_ ) {
			free(ptr_);
		}
	}

	inline void Reserve(int sz) {
		if ( offset_ + sz > size_ ) {
			int nsize = size_ * 2;
			while ( nsize < offset_ + sz )
				nsize = nsize * 2;

			char* nptr = (char*)malloc(nsize);
			memcpy(nptr, ptr_, size_);
			size_ = nsize;

			if ( ptr_ != init_ )
				free(ptr_);
			ptr_ = nptr;
		}
	}

	inline void Append(void* data, int size) {
		Reserve(size);
		memcpy(ptr_ + offset_, data, size);
		offset_ += size;
	}

	inline void Append(const char* str, int sz) {
		Append((uint16_t)sz);
		Append((uint8_t*)str, sz);
	}

	template<typename T>
	inline void Append(T val) {
		Append(&val, sizeof(T));
	}

	template<>
	inline void Append(int64_t val) {
		if ( val == 0 ) {
			Append((uint8_t)0);
			return;
		}
		uint64_t value;
		uint8_t positive = 0;
		if ( val < 0 ) {
			positive = 0;
			value = -val;
		}
		else {
			positive = 1;
			value = val;
		}

		int length;
		if ( value <= 0xff ) {
			length = 1;
		}
		else if ( value <= 0xffff ) {
			length = 2;
		}
		else if ( value <= 0xffffff ) {
			length = 3;
		}
		else if ( value <= 0xffffffff ) {
			length = 4;
		}
		else if ( value <= 0xffffffffff ) {
			length = 5;
		}
		else if ( value <= 0xffffffffffff ) {
			length = 6;
		}
		else {
			length = 7;
		}

		uint8_t tag = length;
		tag = (tag << 1) | positive;

		uint8_t data[8] = {0};
		data[0] = tag;
		memcpy(&data[1], &value, length);

		Append(data, length + 1);
	}

	inline int CheckArray(lua_State* L, Field* field, int index, int vt) {
		if ( vt != LUA_TTABLE ) {
			throw ArrayMemberException(field->name_, lua_typename(L, vt));
		}

		int size = lua_rawlen(L, index);
		if ( size > 0xffff ) {
			throw ArraySizeException(field->name_);
		}
		return size;
	}

	void EncodeOne(lua_State* L, Field* field, int index, int depth) {
		switch ( field->type_ ) {
			case FTYPE_BOOL: {
								 EncodeBool(L, field, index);
								 break;
			}
			case FTYPE_SHORT: {
								  EncodeShort(L, field, index);
								  break;
			}
			case FTYPE_INT: {
								EncodeInt(L, field, index);
								break;
			}
			case FTYPE_FLOAT: {
								  EncodeFloat(L, field, index);
								  break;
			}
			case FTYPE_DOUBLE: {
								   EncodeDouble(L, field, index);
								   break;
			}
			case FTYPE_STRING: {
								   EncodeString(L, field, index);
								   break;
			}
			case FTYPE_PROTOCOL: {
									 EncodePto(L, field, index, depth);
									 break;
			}
			default: {
						 throw TypeException(field->name_, field->type_);
						 break;
			}
		}
	}

	void EncodeBool(lua_State* L, Field* field, int index) {
		int vt = lua_type(L, index);
		if ( field->array_ ) {
			int size = CheckArray(L, field, index, vt);
			Append<uint16_t>(size);
			for ( int i = 1; i <= size; i++ ) {
				lua_rawgeti(L, index, i);
				vt = lua_type(L, -1);
				if ( vt != LUA_TBOOLEAN ) {
					throw ArrayFieldException(field->name_, "bool", lua_typename(L, vt));
				}
				Append<bool>((bool)lua_toboolean(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TBOOLEAN ) {
				throw FieldException(field->name_, "bool", lua_typename(L, vt));
			}
			Append<bool>((bool)lua_toboolean(L, index));
		}
	}

	void EncodeShort(lua_State* L, Field* field, int index) {
		int vt = lua_type(L, index);
		if ( field->array_ ) {
			int size = CheckArray(L, field, index, vt);
			Append<uint16_t>(size);
			for ( int i = 1; i <= size; i++ ) {
				lua_rawgeti(L, index, i);
				vt = lua_type(L, -1);
				if ( vt != LUA_TNUMBER ) {
					throw ArrayFieldException(field->name_, "short", lua_typename(L, vt));
				}
				Append<short>((short)lua_tointeger(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TNUMBER ) {
				throw FieldException(field->name_, "short", lua_typename(L, vt));
			}
			Append<short>((short)lua_tointeger(L, index));
		}
	}

	inline void EncodeInt(lua_State* L, Field* field, int index) {
		int vt = lua_type(L, index);

		if ( field->array_ ) {
			int size = CheckArray(L, field, index, vt);
			Append<uint16_t>(size);
			for ( int i = 1; i <= size; i++ ) {
				lua_rawgeti(L, index, i);
				vt = lua_type(L, -1);
				if ( vt != LUA_TNUMBER ) {
					throw ArrayFieldException(field->name_, "int", lua_typename(L, vt));
				}
				lua_Integer val = lua_tointeger(L, -1);
				if ( val > MAX_INT || val < -MAX_INT ) {
					throw IntLimitException(field->name_, val, true);
				}
				Append<int64_t>(val);
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TNUMBER ) {
				throw FieldException(field->name_, "int", lua_typename(L, vt));
			}
			lua_Integer val = lua_tointeger(L, index);
			if ( val > MAX_INT || val < -MAX_INT ) {
				throw IntLimitException(field->name_, val, false);
			}
			Append<int64_t>(val);
		}
	}

	inline void EncodeFloat(lua_State* L, Field* field, int index) {
		int vt = lua_type(L, index);

		if ( field->array_ ) {
			int size = CheckArray(L, field, index, vt);
			Append<uint16_t>(size);
			for ( int i = 1; i <= size; i++ ) {
				lua_rawgeti(L, index, i);
				vt = lua_type(L, -1);
				if ( vt != LUA_TNUMBER ) {
					throw ArrayFieldException(field->name_, "float", lua_typename(L, vt));
				}
				Append<float>((float)lua_tonumber(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TNUMBER ) {
				throw FieldException(field->name_, "float", lua_typename(L, vt));
			}
			Append<float>((float)lua_tonumber(L, index));
		}
	}

	inline void EncodeDouble(lua_State* L, Field* field, int index) {
		int vt = lua_type(L, index);
		if ( field->array_ ) {
			int size = CheckArray(L, field, index, vt);
			Append<uint16_t>(size);
			for ( int i = 1; i <= size; i++ ) {
				lua_rawgeti(L, index, i);
				vt = lua_type(L, -1);
				if ( vt != LUA_TNUMBER ) {
					throw ArrayFieldException(field->name_, "double", lua_typename(L, vt));
				}
				Append<double>(lua_tonumber(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TNUMBER ) {
				throw FieldException(field->name_, "double", lua_typename(L, vt));
			}
			Append<double>(lua_tonumber(L, index));
		}
	}

	inline void EncodeString(lua_State* L, Field* field, int index) {
		int vt = lua_type(L, index);
		if ( field->array_ ) {
			int size = CheckArray(L, field, index, vt);
			Append<uint16_t>(size);
			for ( int i = 1; i <= size; i++ ) {
				lua_rawgeti(L, index, i);
				vt = lua_type(L, -1);
				if ( vt != LUA_TSTRING ) {
					throw ArrayFieldException(field->name_, "string", lua_typename(L, vt));
				}
				size_t size;
				const char* str = lua_tolstring(L, -1, &size);
				if ( size > 0xffff ) {
					throw StringLimitException(field->name_, size);
				}
				Append(str, size);
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TSTRING ) {
				throw FieldException(field->name_, "string", lua_typename(L, vt));
			}
			size_t size;
			const char* str = lua_tolstring(L, index, &size);
			if ( size > 0xffff ) {
				throw StringLimitException(field->name_, size);
			}
			Append(str, size);
		}
	}


	inline void EncodePto(lua_State* L, Field* field, int index, int depth) {
		depth++;
		if ( depth > MAX_DEPTH ) {
			throw TooDepthException(true);
		}

		int vt = lua_type(L, index);
		if ( vt != LUA_TTABLE ) {
			throw FieldException(field->name_, "table", lua_typename(L, vt));
		}

		if ( field->array_ ) {
			int size = CheckArray(L, field, index, vt);
			Append<uint16_t>(size);
			for ( int i = 0; i < size; i++ ) {
				lua_rawgeti(L, index, i + 1);
				vt = lua_type(L, -1);
				if ( vt != LUA_TTABLE ) {
					throw ArrayFieldException(field->name_, "table", lua_typename(L, vt));
				}

				for ( int j = 0; j < field->childs_->size(); j++ ) {
					Field* child = (*field->childs_)[j];
					lua_getfield(L, -1, child->name_);
					EncodeOne(L, child, index + 2, depth);
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
			}
		}
		else {
			for ( int i = 0; i < field->childs_->size(); i++ ) {
				Field* child = (*field->childs_)[i];
				lua_getfield(L, index, child->name_);
				EncodeOne(L, child, index + 1, depth);
				lua_pop(L, 1);
			}
		}
	}

};

struct PtoReader {
	char* ptr_;
	int offset_;
	int size_;

	PtoReader(char* ptr, int size) {
		ptr_ = ptr;
		size_ = size;
		offset_ = 0;
	}

	inline void Read(uint8_t* val, size_t size) {
		if ( size_ - offset_ < size ) {
			throw ErrorDecode();
		}
		memcpy(val, ptr_ + offset_, size);
		offset_ += size;
	}

	template<typename T>
	inline T Read()  {
		if ( sizeof(T) > size_ - offset_ ) {
			throw ErrorDecode();
		}
		T val = *((T*)&ptr_[offset_]);
		offset_ += sizeof(T);
		return val;
	}

	template<>
	inline int64_t Read() {
		uint8_t tag = Read<uint8_t>();

		if ( tag == 0 ) {
			return 0;
		}

		int length = tag >> 1;

		uint64_t value = 0;
		Read((uint8_t*)&value, length);

		return (tag & 0x1) == 1 ? value : -(lua_Integer)value;
	}

	inline char* Read(uint16_t* size) {
		*size = Read<uint16_t>();
		if ( size_ - offset_ < *size ) {
			throw ErrorDecode();
		}
		char* result = ptr_ + offset_;
		offset_ += *size;
		return result;
	}

	void DecodeOne(lua_State* L, Field* field, int index, int depth) {
		switch ( field->type_ ) {
			case FTYPE_BOOL: {
								 DecodeBool(L, field, index);
								 break;
			}
			case FTYPE_SHORT: {
								  DecodeShort(L, field, index);
								  break;
			}
			case FTYPE_INT: {
								DecodeInt(L, field, index);
								break;
			}
			case FTYPE_FLOAT: {
								  DecodeFloat(L, field, index);
								  break;
			}
			case FTYPE_DOUBLE: {
								   DecodeDouble(L, field, index);
								   break;
			}
			case FTYPE_STRING: {
								   DecodeString(L, field, index);
								   break;
			}
			case FTYPE_PROTOCOL: {
									 DecodePto(L, field, index, depth);
									 break;
			}
			default: {
						 throw TypeException(field->name_, field->type_);
			}
		}
	}

	inline void DecodeBool(lua_State* L, Field* field, int index) {
		if ( field->array_ ) {
			uint16_t size = Read<uint16_t>();
			lua_createtable(L, 0, 0);
			for ( int i = 1; i <= size; i++ ) {
				bool val = Read<bool>();
				lua_pushboolean(L, val);
				lua_rawseti(L, -2, i);
			}
			lua_setfield(L, index, field->name_);
		}
		else {
			bool val = Read<bool>();
			lua_pushboolean(L, val);
			lua_setfield(L, index, field->name_);
		}
	}

	inline void DecodeShort(lua_State* L, Field* field, int index) {
		if ( field->array_ ) {
			uint16_t size = Read<uint16_t>();
			lua_createtable(L, 0, 0);
			for ( int i = 1; i <= size; i++ ) {
				short val = Read<short>();
				lua_pushinteger(L, val);
				lua_rawseti(L, -2, i);
			}
			lua_setfield(L, index, field->name_);
		}
		else {
			short val = Read<short>();
			lua_pushinteger(L, val);
			lua_setfield(L, index, field->name_);
		}
	}

	inline void DecodeInt(lua_State* L, Field* field, int index) {
		if ( field->array_ ) {
			uint16_t size = Read<uint16_t>();
			lua_createtable(L, 0, 0);
			for ( int i = 1; i <= size; i++ ) {
				lua_Integer val = Read<int64_t>();
				lua_pushinteger(L, val);
				lua_rawseti(L, -2, i);
			}
			lua_setfield(L, index, field->name_);
		}
		else {
			lua_Integer val = Read<int64_t>();
			lua_pushinteger(L, val);
			lua_setfield(L, index, field->name_);
		}
	}

	inline void DecodeFloat(lua_State* L, Field* field, int index) {
		if ( field->array_ ) {
			uint16_t size = Read<uint16_t>();
			lua_createtable(L, 0, 0);
			for ( int i = 1; i <= size; i++ ) {
				float val = Read<float>();
				lua_pushnumber(L, val);
				lua_rawseti(L, -2, i);
			}
			lua_setfield(L, index, field->name_);
		}
		else {
			float val = Read<float>();
			lua_pushnumber(L, val);
			lua_setfield(L, index, field->name_);
		}
	}

	inline void DecodeDouble(lua_State* L, Field* field, int index) {
		if ( field->array_ ) {
			uint16_t size = Read<uint16_t>();
			lua_createtable(L, 0, 0);
			for ( int i = 1; i <= size; i++ ) {
				double val = Read<double>();
				lua_pushnumber(L, val);
				lua_rawseti(L, -2, i);
			}
			lua_setfield(L, index, field->name_);
		}
		else {
			double val = Read<double>();
			lua_pushnumber(L, val);
			lua_setfield(L, index, field->name_);
		}
	}

	inline void DecodeString(lua_State* L, Field* field, int index) {
		if ( field->array_ ) {
			uint16_t size = Read<uint16_t>();
			lua_createtable(L, 0, 0);
			for ( int i = 1; i <= size; i++ ) {
				uint16_t size;
				char* val = Read(&size);
				lua_pushlstring(L, val, size);
				lua_rawseti(L, -2, i);
			}
			lua_setfield(L, index, field->name_);
		}
		else {
			uint16_t size;
			char* val = Read(&size);
			lua_pushlstring(L, val, size);
			lua_setfield(L, index, field->name_);
		}
	}

	inline void DecodePto(lua_State* L, Field* field, int index, int depth) {
		depth++;
		if ( depth > MAX_DEPTH ) {
			throw TooDepthException(false);
		}

		if ( field->array_ ) {
			uint16_t size = Read<uint16_t>();
			lua_createtable(L, 0, 0);
			for ( int i = 1; i <= size; i++ ) {
				lua_createtable(L, 0, field->childs_->size());
				for ( int j = 0; j < field->childs_->size(); j++ ) {
					Field* child = (*field->childs_)[j];
					DecodeOne(L, child, index + 2, depth);
				}
				lua_seti(L, -2, i);
			}
			lua_setfield(L, index, field->name_);
		}
		else {
			lua_createtable(L, 0, field->childs_->size());
			for ( int i = 0; i < field->childs_->size(); i++ ) {
				Field* child = (*field->childs_)[i];
				DecodeOne(L, child, index + 1, depth);
			}
			lua_setfield(L, index, field->name_);
		}
	}

};

static void ImportField(lua_State* L, ProtocolContext* ctx, Field* field, int index, int depth) {
	int size = lua_rawlen(L, index);
	for ( int i = 1; i <= size; i++ ) {
		lua_rawgeti(L, index, i);

		lua_getfield(L, -1, "type");
		int type = lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "array");
		int array = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "name");
		const char* name = lua_tostring(L, -1);
		lua_pop(L, 1);

		Field* child = new Field(name, array, type);
		if ( type == FTYPE_PROTOCOL ) {
			lua_getfield(L, -1, "pto");
			ImportField(L, ctx, child, lua_gettop(L), ++depth);
			lua_pop(L, 1);
		}
		field->AddField(child);

		lua_pop(L, 1);
	}
}

static void ImportField(lua_State* L, ProtocolContext* ctx, Protocol* pto, int index, int depth) {
	int size = lua_rawlen(L, index);
	for ( int i = 1; i <= size; i++ ) {
		lua_rawgeti(L, index, i);

		lua_getfield(L, -1, "type");
		int type = lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "array");
		int array = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "name");
		const char* name = lua_tostring(L, -1);
		lua_pop(L, 1);

		Field* field = new Field(name, array, type);

		if ( type == FTYPE_PROTOCOL ) {
			lua_getfield(L, -1, "pto");
			ImportField(L, ctx, field, lua_gettop(L), ++depth);
			lua_pop(L, 1);
		}

		pto->AddField(field);

		lua_pop(L, 1);
	}
}

static int ImportPto(lua_State* L) {
	ProtocolContext* ctx = (ProtocolContext*)lua_touserdata(L, 1);
	uint16_t id = luaL_checkinteger(L, 2);
	size_t size;
	const char* name = luaL_checklstring(L, 3, &size);

	luaL_checktype(L, 4, LUA_TTABLE);
	if ( id > 0xffff ) {
		luaL_error(L, "id must less than 0xffff");
	}

	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);

	Protocol* pto = new Protocol(name);

	int depth = 0;
	ImportField(L, ctx, pto, lua_gettop(L), ++depth);

	ctx->AddPto(id, pto);

	return 0;
}

static int EncodePto(lua_State* L) {
	ProtocolContext* ctx = (ProtocolContext*)lua_touserdata(L, 1);
	uint16_t id = luaL_checkinteger(L, 2);
	Protocol* pto = ctx->ptos_[id];

	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);

	PtoWriter writer;
	try {
		int depth = 1;
		for ( int i = 0; i < pto->fields_.size(); i++ ) {
			Field* field = pto->GetField(i);
			lua_getfield(L, 3, field->name_);
			writer.EncodeOne(L, field, 4, depth);
			lua_pop(L, 1);
		}
	}
	catch ( PtoException e ) {
		luaL_error(L, e.reason_.c_str());
	}

	lua_pushlstring(L, writer.ptr_, writer.offset_);
	return 1;
}

static int DecodePto(lua_State* L) {
	ProtocolContext* ctx = (ProtocolContext*)lua_touserdata(L, 1);
	uint16_t id = luaL_checkinteger(L, 2);
	Protocol* pto = ctx->ptos_[id];

	size_t size;
	const char* str = NULL;
	switch ( lua_type(L, 3) ) {
		case LUA_TSTRING: {
							  str = lua_tolstring(L, 3, &size);
							  break;
		}
		case LUA_TLIGHTUSERDATA:{
									str = (const char*)lua_touserdata(L, 3);
									size = lua_tointeger(L, 4);
									break;
		}
		default:
			luaL_error(L, "decode protocol:%s error,unkown type:%s", pto->name_, lua_typename(L, lua_type(L, 3)));
	}

	PtoReader reader((char*)str, size);

	int depth = 1;
	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);

	lua_createtable(L, 0, pto->fields_.size());
	int top = lua_gettop(L);

	try {
		for ( int i = 0; i < pto->fields_.size(); i++ ) {
			Field* field = pto->GetField(i);
			reader.DecodeOne(L, field, top, depth);
		}
	}
	catch ( PtoException* e ) {
		luaL_error(L, e->reason_.c_str());
	}

	if ( reader.offset_ != reader.size_ ) {
		luaL_error(L, "decode protocol:%s error", pto->name_);
	}

	return 1;
}

static int Release(lua_State* L) {
	return 0;
}

static int Create(lua_State* L) {
	void* userdata = lua_newuserdata(L, sizeof(ProtocolContext));
	ProtocolContext* ptoCtx = new (userdata)ProtocolContext();

	lua_pushvalue(L, -1);
	if ( luaL_newmetatable(L, "pto") ) {
		const luaL_Reg meta[] = {
			{"Import", ImportPto},
			{"Encode", EncodePto},
			{"Decode", DecodePto},
			{NULL, NULL},
		};

		luaL_newlib(L, meta);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, Release);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);

	return 1;
}

int luaopen_pto(lua_State* L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{"Create", Create},
		{NULL, NULL},
	};
	luaL_newlib(L, l);

	lua_pushinteger(L, FTYPE_BOOL);
	lua_setfield(L, -2, "BOOL");

	lua_pushinteger(L, FTYPE_SHORT);
	lua_setfield(L, -2, "SHORT");

	lua_pushinteger(L, FTYPE_INT);
	lua_setfield(L, -2, "INT");

	lua_pushinteger(L, FTYPE_FLOAT);
	lua_setfield(L, -2, "FLOAT");

	lua_pushinteger(L, FTYPE_DOUBLE);
	lua_setfield(L, -2, "DOUBLE");

	lua_pushinteger(L, FTYPE_STRING);
	lua_setfield(L, -2, "STRING");

	lua_pushinteger(L, FTYPE_PROTOCOL);
	lua_setfield(L, -2, "PROTOCOL");

	return 1;
}
