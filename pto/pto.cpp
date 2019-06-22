#include "pto.h"
#include "error.h"
#include "encode.h"
#include "decode.h"

#include "lua.hpp"

#define MAX_DEPTH 32
#define MAX_INT 	0xffffffffffffff
#define MAX_INT16 	(0x7ffff)
#define MIN_INT16	(-0x7ffff-1)
#define MAX_INT8 	(0x7f)
#define MIN_INT8 	(-0x7f-1)

using namespace LuaPto;

static inline uint8_t BeginArray(lua_State* L, Field* field, Encoder& encoder, int index, int vt) {
	if ( vt != LUA_TTABLE ) {
		throw new BadArray(field, lua_typename(L, vt));
	}

	size_t size = lua_rawlen(L, index);
	if ( size > 0xff ) {
		throw new BadSize(field, size);
	}
	encoder.Append<uint8_t>(size);
	return size;
}

static inline void EncodeBool(lua_State* L, Field* field, Encoder& encoder, int index) {
	int vt = lua_type(L, index);
	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 1; i <= size; i++ ) {
			lua_rawgeti(L, index, i);
			vt = lua_type(L, -1);
			if ( vt != LUA_TBOOLEAN ) {
				throw new BadField(field, lua_typename(L, vt));
			}
			encoder.Append<bool>(lua_toboolean(L, -1));
			lua_pop(L, 1);
		}
	}
	else {
		if ( vt != LUA_TBOOLEAN ) {
			throw new BadField(field, lua_typename(L, vt));
		}
		encoder.Append<bool>(lua_toboolean(L, index));
	}
}

static inline void EncodeByte(lua_State* L, Field* field, Encoder& encoder, int index) {
	int vt = lua_type(L, index);
	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 1; i <= size; i++ ) {
			lua_rawgeti(L, index, i);
			vt = lua_type(L, -1);
			if ( vt != LUA_TNUMBER ) {
				throw new BadField(field, lua_typename(L, vt));
			}
			lua_Integer val = lua_tointeger(L, -1);
			if ( val > MAX_INT8 || val < MIN_INT8 ) {
				throw new BadByte(field, val);
			}
			encoder.Append<int8_t>(val);
			lua_pop(L, 1);
		}
	}
	else {
		if ( vt != LUA_TNUMBER ) {
			throw new BadField(field, lua_typename(L, vt));
		}
		lua_Integer val = lua_tointeger(L, index);
		if ( val > MAX_INT8 || val < MIN_INT8 ) {
			throw new BadByte(field, val);
		}
		encoder.Append<int8_t>(val);
	}
}

static inline void EncodeShort(lua_State* L, Field* field, Encoder& encoder, int index) {
	int vt = lua_type(L, index);
	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 1; i <= size; i++ ) {
			lua_rawgeti(L, index, i);
			vt = lua_type(L, -1);
			if ( vt != LUA_TNUMBER ) {
				throw new BadField(field, lua_typename(L, vt));
			}
			lua_Integer val = lua_tointeger(L, -1);
			if ( val > MAX_INT16 || val < MIN_INT16 ) {
				throw new BadShort(field, val);
			}
			encoder.Append<short>((short)val);
			lua_pop(L, 1);
		}
	}
	else {
		if ( vt != LUA_TNUMBER ) {
			throw new BadField(field, lua_typename(L, vt));
		}
		lua_Integer val = lua_tointeger(L, index);
		if ( val > MAX_INT16 || val < MIN_INT16 ) {
			throw new BadShort(field, val);
		}
		encoder.Append<short>((short)val);
	}
}

static inline void EncodeInt(lua_State* L, Field* field, Encoder& encoder, int index) {
	int vt = lua_type(L, index);

	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 1; i <= size; i++ ) {
			lua_rawgeti(L, index, i);
			vt = lua_type(L, -1);
			if ( vt != LUA_TNUMBER ) {
				throw new BadField(field, lua_typename(L, vt));
			}
			lua_Integer val = lua_tointeger(L, -1);
			if ( val > MAX_INT || val < -MAX_INT ) {
				throw new BadInt(field, val);
			}
			encoder.Append<int64_t>(val);
			lua_pop(L, 1);
		}
	}
	else {
		if ( vt != LUA_TNUMBER ) {
			throw new BadField(field, lua_typename(L, vt));
		}
		lua_Integer val = lua_tointeger(L, index);
		if ( val > MAX_INT || val < -MAX_INT ) {
			throw new BadInt(field, val);
		}
		encoder.Append<int64_t>(val);
	}
}

static inline void EncodeFloat(lua_State* L, Field* field, Encoder& encoder, int index) {
	int vt = lua_type(L, index);

	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 1; i <= size; i++ ) {
			lua_rawgeti(L, index, i);
			vt = lua_type(L, -1);
			if ( vt != LUA_TNUMBER ) {
				throw new BadField(field, lua_typename(L, vt));
			}
			encoder.Append<float>((float)lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
	}
	else {
		if ( vt != LUA_TNUMBER ) {
			throw new BadField(field, lua_typename(L, vt));
		}
		encoder.Append<float>((float)lua_tonumber(L, index));
	}
}

static inline void EncodeDouble(lua_State* L, Field* field, Encoder& encoder, int index) {
	int vt = lua_type(L, index);
	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 1; i <= size; i++ ) {
			lua_rawgeti(L, index, i);
			vt = lua_type(L, -1);
			if ( vt != LUA_TNUMBER ) {
				throw new BadField(field, lua_typename(L, vt));
			}
			encoder.Append<double>(lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
	}
	else {
		if ( vt != LUA_TNUMBER ) {
			throw new BadField(field, lua_typename(L, vt));
		}
		encoder.Append<double>(lua_tonumber(L, index));
	}
}

static inline void EncodeString(lua_State* L, Field* field, Encoder& encoder, int index) {
	int vt = lua_type(L, index);
	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 1; i <= size; i++ ) {
			lua_rawgeti(L, index, i);
			vt = lua_type(L, -1);
			if ( vt != LUA_TSTRING ) {
				throw new BadField(field, lua_typename(L, vt));
			}
			size_t size;
			const char* str = lua_tolstring(L, -1, &size);
			if ( size > 0xffff ) {
				throw new BadString(field, size);
			}
			encoder.Append(str, size);
			lua_pop(L, 1);
		}
	}
	else {
		if ( vt != LUA_TSTRING ) {
			throw new BadField(field, lua_typename(L, vt));
		}
		size_t size;
		const char* str = lua_tolstring(L, index, &size);
		if ( size > 0xffff ) {
			throw new BadString(field, size);
		}
		encoder.Append(str, size);
	}
}

static void EncodeOne(lua_State* L, Field* field, Encoder& encoder, int index, int depth);

static inline void EncodePto(lua_State* L, Field* field, Encoder& encoder, int index, int depth) {
	depth++;
	if ( depth > MAX_DEPTH ) {
		throw new BadPto("encode too depth");
	}

	int vt = lua_type(L, index);
	if ( field->array_ ) {
		uint8_t size = BeginArray(L, field, encoder, index, vt);
		for ( int i = 0; i < size; i++ ) {
			lua_rawgeti(L, index, i + 1);
			vt = lua_type(L, -1);
			if ( vt != LUA_TTABLE ) {
				throw new BadField(field, lua_typename(L, vt));
			}

			for ( uint32_t j = 0; j < field->childs_.size(); j++ ) {
				Field* child = field->GetField(j);
				lua_getfield(L, -1, child->name_);
				EncodeOne(L, child, encoder, index + 2, depth);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
	}
	else {
		for ( uint32_t i = 0; i < field->childs_.size(); i++ ) {
			Field* child = field->GetField(i);
			lua_getfield(L, index, child->name_);
			EncodeOne(L, child, encoder, index + 1, depth);
			lua_pop(L, 1);
		}
	}
}

static void EncodeOne(lua_State* L, Field* field, Encoder& encoder, int index, int depth) {
	switch ( field->type_ ) {
		case eTYPE::Bool:
			EncodeBool(L, field, encoder, index);
			break;
		case eTYPE::Short:
			EncodeShort(L, field, encoder, index);
			break;
		case eTYPE::Int:
			EncodeInt(L, field, encoder, index);
			break;
		case eTYPE::Float:
			EncodeFloat(L, field, encoder, index);
			break;
		case eTYPE::Double:
			EncodeDouble(L, field, encoder, index);
			break;
		case eTYPE::String:
			EncodeString(L, field, encoder, index);
			break;
		case eTYPE::Pto:
			EncodePto(L, field, encoder, index, depth);
			break;
		default:
			throw new BadType(field);
			break;
	}
}

static inline void DecodeBool(lua_State* L, Field* field, Decoder& decoder, int index) {
	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			bool val = decoder.Read<bool>();
			lua_pushboolean(L, val);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		bool val = decoder.Read<bool>();
		lua_pushboolean(L, val);
		lua_setfield(L, index, field->name_);
	}
}

static inline void DecodeByte(lua_State* L, Field* field, Decoder& decoder, int index) {
	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			int8_t val = decoder.Read<int8_t>();
			lua_pushinteger(L, val);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		int8_t val = decoder.Read<int8_t>();
		lua_pushinteger(L, val);
		lua_setfield(L, index, field->name_);
	}
}

static inline void DecodeShort(lua_State* L, Field* field, Decoder& decoder, int index) {
	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			short val = decoder.Read<short>();
			lua_pushinteger(L, val);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		short val = decoder.Read<short>();
		lua_pushinteger(L, val);
		lua_setfield(L, index, field->name_);
	}
}

static inline void DecodeInt(lua_State* L, Field* field, Decoder& decoder, int index) {
	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			int64_t val = decoder.Read<int64_t>();
			lua_pushinteger(L, val);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		int64_t val = decoder.Read<int64_t>();
		lua_pushinteger(L, val);
		lua_setfield(L, index, field->name_);
	}
}

static inline void DecodeFloat(lua_State* L, Field* field, Decoder& decoder, int index) {
	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			float val = decoder.Read<float>();
			lua_pushnumber(L, val);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		float val = decoder.Read<float>();
		lua_pushnumber(L, val);
		lua_setfield(L, index, field->name_);
	}
}

static inline void DecodeDouble(lua_State* L, Field* field, Decoder& decoder, int index) {
	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			double val = decoder.Read<double>();
			lua_pushnumber(L, val);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		double val = decoder.Read<double>();
		lua_pushnumber(L, val);
		lua_setfield(L, index, field->name_);
	}
}

static inline void DecodeString(lua_State* L, Field* field, Decoder& decoder, int index) {
	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			uint16_t size;
			const char* val = decoder.Read(&size);
			lua_pushlstring(L, val, size);
			lua_rawseti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		uint16_t size;
		const char* val = decoder.Read(&size);
		lua_pushlstring(L, val, size);
		lua_setfield(L, index, field->name_);
	}
}

static void DecodeOne(lua_State* L, Field* field, Decoder& decoder, int index, int depth);

static void DecodePto(lua_State* L, Field* field, Decoder& decoder, int index, int depth) {
	depth++;
	if ( depth > MAX_DEPTH ) {
		throw new BadPto("decode too depth");
	}

	if ( field->array_ ) {
		uint8_t size = decoder.Read<uint8_t>();
		lua_createtable(L, size, 0);
		for ( int i = 1; i <= size; i++ ) {
			int size = field->childs_.size();
			lua_createtable(L, 0, size);
			for ( int j = 0; j < size; j++ ) {
				Field* child = field->GetField(j);
				DecodeOne(L, child, decoder, index + 2, depth);
			}
			lua_seti(L, -2, i);
		}
		lua_setfield(L, index, field->name_);
	}
	else {
		int size = field->childs_.size();
		lua_createtable(L, 0, size);
		for ( int i = 0; i < size; i++ ) {
			Field* child = field->GetField(i);
			DecodeOne(L, child, decoder, index + 1, depth);
		}
		lua_setfield(L, index, field->name_);
	}
}

static void DecodeOne(lua_State* L, Field* field, Decoder& decoder, int index, int depth) {
	switch ( field->type_ ) {
		case eTYPE::Bool:
			DecodeBool(L, field, decoder, index);
			break;
		case eTYPE::Short:
			DecodeShort(L, field, decoder, index);
			break;
		case eTYPE::Int:
			DecodeInt(L, field, decoder, index);
			break;
		case eTYPE::Float:
			DecodeFloat(L, field, decoder, index);
			break;
		case eTYPE::Double:
			DecodeDouble(L, field, decoder, index);
			break;
		case eTYPE::String:
			DecodeString(L, field, decoder, index);
			break;
		case eTYPE::Pto:
			DecodePto(L, field, decoder, index, depth);
			break;
		default:
			throw new BadType(field);
	}
}

static void ExportField(lua_State* L, std::vector<Field*>& fields) {
	lua_newtable(L);
	for ( uint32_t i = 0; i < fields.size(); i++ ) {
		lua_newtable(L);
		Field* field = fields[i];

		lua_pushstring(L, field->name_);
		lua_setfield(L, -2, "name");

		lua_pushinteger(L, field->array_);
		lua_setfield(L, -2, "array");

		lua_pushinteger(L, field->type_);
		lua_setfield(L, -2, "type");

		if ( field->type_ == eTYPE::Pto ) {
			ExportField(L, field->childs_);
			lua_setfield(L, -2, "pto");
		}
		lua_seti(L, -2, i + 1);
	}
}


static void ImportField(lua_State* L, Context* ctx, std::vector<Field*>& fields, int index, int depth) {
	int size = lua_rawlen(L, index);
	for ( int i = 1; i <= size; i++ ) {
		lua_rawgeti(L, index, i);

		lua_getfield(L, -1, "type");
		eTYPE type = (eTYPE)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "array");
		bool array = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "name");
		const char* name = lua_tostring(L, -1);
		lua_pop(L, 1);

		Field* field = new Field(name, array, type);

		if ( type == eTYPE::Pto ) {
			lua_getfield(L, -1, "pto");
			ImportField(L, ctx, field->childs_, lua_gettop(L), ++depth);
			lua_pop(L, 1);
		}

		fields.push_back(field);

		lua_pop(L, 1);
	}
}

static inline Protocol* GetPto(lua_State* L, Context* ctx, int index) {
	uint16_t id = (uint16_t)luaL_checkinteger(L, index);
	Protocol* pto = ctx->GetPto(id);
	if (!pto) {
		luaL_error(L, "no such pto:%d", id);
	}
	return pto;
}

static int ImportPto(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	int id = luaL_checkinteger(L, 2);
	if ( id < 0 || id > 0xffff ) {
		luaL_error(L, "pto id must less than 0xffff");
	}

	size_t size;
	const char* name = luaL_checklstring(L, 3, &size);

	luaL_checktype(L, 4, LUA_TTABLE);
	
	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);

	Protocol* pto = new Protocol(name);

	int depth = 0;
	ImportField(L, ctx, pto->fields_, lua_gettop(L), ++depth);

	ctx->AddPto(id, pto);

	return 0;
}

static int ExportPto(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);

	lua_pushstring(L, pto->name_);

	ExportField(L, pto->fields_);
	return 2;
}

static int EncodePto(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);

	try {
		int depth = 1;
		luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);
		Encoder encoder;
		for ( uint32_t i = 0; i < pto->fields_.size(); i++ ) {
			Field* field = pto->GetField(i);
			lua_getfield(L, 3, field->name_);
			EncodeOne(L, field, encoder, 4, depth);
			lua_pop(L, 1);
		}
		lua_pushlstring(L, encoder.ptr_, encoder.offset_);
	}
	catch ( BadPto* e ) {
		lua_pushstring(L, e->what());
		delete e;
		luaL_error(L, "encode:%s error:%s", pto->name_, lua_tostring(L, -1));
	}

	return 1;
}

static int DecodePto(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);

	size_t size;
	const char* str = NULL;
	switch ( lua_type(L, 3) ) {
		case LUA_TSTRING:
			str = lua_tolstring(L, 3, &size);
			break;

		case LUA_TLIGHTUSERDATA:
			str = (const char*)lua_touserdata(L, 3);
			size = (size_t)lua_tointeger(L, 4);
			break;

		default:
			luaL_error(L, "decode protocol:%s error,unkown type:%s", pto->name_, lua_typename(L, lua_type(L, 3)));
	}

	try {
		int depth = 1;
		luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);
		lua_createtable(L, 0, pto->fields_.size());
		int top = lua_gettop(L);
		Decoder decoder(str, size);
		for ( uint32_t i = 0; i < pto->fields_.size(); i++ ) {
			Field* field = pto->GetField(i);
			DecodeOne(L, field, decoder, top, depth);
		}
		if ( decoder.offset_ != decoder.size_ ) {
			throw new BadPto("invalid message");
		}
	}
	catch ( BadPto* e ) {
		lua_pushstring(L, e->what());
		delete e;
		luaL_error(L, "decode:%s error:%s", pto->name_, lua_tostring(L, -1));
	}

	return 1;
}

static int Release(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	ctx->~Context();
	return 0;
}

static int Create(lua_State* L) {
	void* userdata = lua_newuserdata(L, sizeof(Context));
	Context* ctx = new (userdata)Context();

	lua_pushvalue(L, -1);
	if ( luaL_newmetatable(L, "pto") ) {
		const luaL_Reg meta[] = {
			{"Import", ImportPto},
			{"Export", ExportPto},
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

int luaopen_ptocxx(lua_State* L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{"Create", Create},
		{NULL, NULL},
	};
	luaL_newlib(L, l);

	lua_pushinteger(L, eTYPE::Bool);
	lua_setfield(L, -2, "BOOL");

	lua_pushinteger(L, eTYPE::Byte);
	lua_setfield(L, -2, "Byte");

	lua_pushinteger(L, eTYPE::Short);
	lua_setfield(L, -2, "SHORT");

	lua_pushinteger(L, eTYPE::Int);
	lua_setfield(L, -2, "INT");

	lua_pushinteger(L, eTYPE::Float);
	lua_setfield(L, -2, "FLOAT");

	lua_pushinteger(L, eTYPE::Double);
	lua_setfield(L, -2, "DOUBLE");

	lua_pushinteger(L, eTYPE::String);
	lua_setfield(L, -2, "STRING");

	lua_pushinteger(L, eTYPE::Pto);
	lua_setfield(L, -2, "PROTOCOL");

	return 1;
}
