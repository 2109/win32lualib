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

struct ArrayException : public PtoException {
	ArrayException(const char* field, const char* vt) {
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
		if (array) {
			reason_ = fmt::format("field:{} array member int out of range,{}", field, val);
		} else {
			reason_ = fmt::format("field:{} int out of range,{}", field, val);
		}
	}
};

struct StringLimitException : public PtoException {
	StringLimitException(const char* field, size_t size) {
		reason_ = fmt::format("field:{} string size more than 0xffff:{}", field, size);
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
		ptos_.reserve(0xffff);
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

	inline void Reserve(size_t sz) {
		if ( offset_ + sz > size_ ) {
			size_t nsize = size_ * 2;
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

	inline void Append(void* data, size_t size) {
		Reserve(size);
		memcpy(ptr_ + offset_, data, size);
		offset_ += size;
	}

	inline void Append(const char* str, size_t sz) {
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
			throw ArrayException(field->name_, lua_typename(L, vt));
		}

		int size = lua_rawlen(L, index);
		if ( size > 0xffff ) {
			throw ArraySizeException(field->name_);
		}
		return size;
	}

	int EncodeOne(lua_State* L, Field* field, int index, int depth) {
		switch ( field->type_ ) {
			case FTYPE_BOOL: {
				EncodeBool(L, field, index);
				break;
			}
			case FTYPE_SHORT: {
				EncodeInt16(L, field, index);
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
				break;
			}
			default: {
				return -1;
			}
		}
		return 0;
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
				Append<bool>(lua_toboolean(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TBOOLEAN ) {
				throw FieldException(field->name_, "bool", lua_typename(L, vt));
			}
			Append<bool>(lua_toboolean(L, index));
		}
	}

	void EncodeInt16(lua_State* L, Field* field, int index) {
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
				Append<int16_t>(lua_tointeger(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TNUMBER ) {
				throw FieldException(field->name_, "short", lua_typename(L, vt));
			}
			Append<int16_t>(lua_tointeger(L, index));
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
				Append<float>(lua_tonumber(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if ( vt != LUA_TNUMBER ) {
				throw FieldException(field->name_, "float", lua_typename(L, vt));
			}
			Append<float>(lua_tonumber(L, index));
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

	template<typename T>
	inline T Read()  {
		assert(sizeof(T) <= size_ - offset_);
		T val = *((T*)&ptr_[offset_]);
		offset_ += sizeof(T);
		return val;
	}

	int DecodeOne(lua_State* L, Field* field, int index, int depth) {
		switch ( field->type_ ) {
			case FTYPE_BOOL: {
								 unpack_bool(L, field, index);
								 break;
			}
			case FTYPE_SHORT: {
								  unpack_short(L, field, index);
								  break;
			}
			case FTYPE_INT: {
								unpack_int(L, field, index);
								break;
			}
			case FTYPE_FLOAT: {
								  unpack_float(L, field, index);
								  break;
			}
			case FTYPE_DOUBLE: {
								   unpack_double(L, field, index);
								   break;
			}
			case FTYPE_STRING: {
								   unpack_string(L, field, index);
								   break;
			}
			case FTYPE_PROTOCOL: {
									 unpack_field(L, field, index);
									 break;
			}
			default: {
						 luaL_error(L, "unpack error:invalid name:%s,type:%d", field->name, f->type);
			}
		}
		return 0;
	}
};

static int ImportField(lua_State* L, ProtocolContext* ctx, Field* field, int index, int depth) {
	int size = lua_rawlen(L, index);
	for ( int i = 0; i <= size; i++ ) {
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
		field->AddField(child);

		lua_pop(L, 1);
	}
}

static int ImportField(lua_State* L, ProtocolContext* ctx, Protocol* pto, int index, int depth) {

	int size = lua_rawlen(L, index);
	for ( int i = 0; i <= size; i++ ) {
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

}

static int EncodePto(lua_State* L) {
	ProtocolContext* ctx = (ProtocolContext*)lua_touserdata(L, 1);
	uint16_t id = luaL_checkinteger(L, 2);
	Protocol* pto = ctx->ptos_[id];

	PtoWriter writer;

	int depth = 1;
	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);

	for ( int i = 0; i < pto->fields_.size(); i++ ) {
		Field* field = pto->GetField(i);
		lua_getfield(L, 3, field->name_);
		writer.EncodeOne(L, field, 4, depth);
		lua_pop(L, 1);
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

	for ( int i = 0; i < pto->fields_.size(); i++ ) {
		Field* field = pto->GetField(i);
		unpack_one(L, &reader, field, top, depth);
	}

	if ( reader.offset_ != reader.size_ ) {
		luaL_error(L, "decode protocol:%s error", pto->name_);
	}

}

static int Release(lua_State* L) {

}

static int Create(lua_State* L) {
	void* userdata = lua_newuserdata(L, sizeof(ProtocolContext));
	ProtocolContext* ptoCtx = new (userdata)ProtocolContext();

	lua_pushvalue(L, -1);
	if ( luaL_newmetatable(L, "pto") ) {
		const luaL_Reg meta[] = {
			{"Import", ImportPto},
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