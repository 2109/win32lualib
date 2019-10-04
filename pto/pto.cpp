#include "pto.h"
#include "encode.h"
#include "decode.h"
#include "parser.h"



#define MAX_DEPTH     16
#define MAX_INT       0xffffffffffffff
#define MAX_INT16     0x7ffff
#define MIN_INT16    (-0x7ffff-1)
#define MAX_INT8      0x7f
#define MIN_INT8     (-0x7f-1)

#define PTO_MAX      0xffff

using namespace LuaPto;

char* Encoder::sBuff_ = (char*)malloc(PTO_MAX);
int Encoder::sSize_ = PTO_MAX;

static inline void BadField(lua_State* L, Field* field, const char* vt) {
	if (field->array_) {
		luaL_error(L, "%s array member expect %s,not %s", field->name_, kTYPE_NAME[field->type_], vt);
	} else {
		luaL_error(L, "%s expect %s,not %s", field->name_, kTYPE_NAME[field->type_], vt);
	}
}

static inline void BadByte(lua_State* L, Field* field, lua_Integer val) {
	if (field->array_) {
		luaL_error(L, "%s array member byte out of range:%d", field->name_, val);
	} else {
		luaL_error(L, "%s byte out of range:%d", field->name_, val);
	}
};

static inline void BadInt(lua_State* L, Field* field, lua_Integer val) {
	if (field->array_) {
		luaL_error(L, "%s array member int out of range:%d", field->name_, val);
	} else {
		luaL_error(L, "%s int out of range:%d", field->name_, val);
	}
}

static inline void BadShort(lua_State* L, Field* field, lua_Integer val) {
	if (field->array_) {
		luaL_error(L, "%s array member short out of range:%d", field->name_, val);
	} else {
		luaL_error(L, "%s short out of range:%d", field->name_, val);
	}
}

static inline void BadString(lua_State* L, Field* field, size_t size) {
	luaL_error(L, "%s string size more than 0xffff:%d", field->name_, size);
}

static void EncodeOne(lua_State* L, Field* field, Encoder* encoder, int index, int depth);
static void DecodeOne(lua_State* L, Field* field, Decoder* decoder, int index, int depth);

static inline void EncodeBool(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	int vt = lua_type(L, index);
	if (vt != LUA_TBOOLEAN) {
		BadField(L, field, lua_typename(L, vt));
	}
	encoder->Append<bool>(lua_toboolean(L, index));
}

static inline void EncodeByte(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	int vt = lua_type(L, index);
	if (vt != LUA_TNUMBER) {
		BadField(L, field, lua_typename(L, vt));
	}
	lua_Integer val = lua_tointeger(L, index);
	if (val > MAX_INT8 || val < MIN_INT8) {
		BadByte(L, field, val);
	}
	encoder->Append<int8_t>(val);
}

static inline void EncodeShort(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	int vt = lua_type(L, index);
	if (vt != LUA_TNUMBER) {
		BadField(L, field, lua_typename(L, vt));
	}
	lua_Integer val = lua_tointeger(L, index);
	if (val > MAX_INT16 || val < MIN_INT16) {
		BadShort(L, field, val);
	}
	encoder->Append<short>((short)val);
}

static inline void EncodeInt(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	int vt = lua_type(L, index);
	if (vt != LUA_TNUMBER) {
		BadField(L, field, lua_typename(L, vt));
	}
	lua_Integer val = lua_tointeger(L, index);
	if (val > MAX_INT || val < -MAX_INT) {
		BadInt(L, field, val);
	}
	encoder->Append<int64_t>(val);
}

static inline void EncodeFloat(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	int vt = lua_type(L, index);
	if (vt != LUA_TNUMBER) {
		BadField(L, field, lua_typename(L, vt));
	}
	encoder->Append<float>((float)lua_tonumber(L, index));
}

static inline void EncodeDouble(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	int vt = lua_type(L, index);
	if (vt != LUA_TNUMBER) {
		BadField(L, field, lua_typename(L, vt));
	}
	encoder->Append<double>(lua_tonumber(L, index));
}

static inline void EncodeString(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	int vt = lua_type(L, index);
	if (vt != LUA_TSTRING) {
		BadField(L, field, lua_typename(L, vt));
	}
	size_t size;
	const char* str = lua_tolstring(L, index, &size);
	if (size > 0xffff) {
		BadString(L, field, size);
	}
	encoder->Append(str, size);
}

static inline void EncodePto(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	depth++;
	if (depth > MAX_DEPTH) {
		luaL_error(L, "encode too depth");
	}

	luaL_checkstack(L, LUA_MINSTACK, NULL);

	int vt = lua_type(L, index);

	if (vt != LUA_TTABLE) {
		BadField(L, field, lua_typename(L, vt));
	}
	for (uint32_t i = 0; i < field->childs_.size(); i++) {
		Field* child = field->GetField(i);
		lua_getfield(L, index, child->name_);
		EncodeOne(L, child, encoder, index + 1, depth);
		lua_pop(L, 1);
	}
}

static void EncodeOne(lua_State* L, Field* field, Encoder* encoder, int index, int depth) {
	if (field->array_) {
		int vt = lua_type(L, index);
		if (vt != LUA_TTABLE) {
			luaL_error(L, "%s expect table,not %s", field->name_, lua_typename(L, vt));
		}

		size_t size = lua_rawlen(L, index);
		if (size > 0xff) {
			luaL_error(L, "%s array size more than 0xff:%ld", field->name_, size);
		}
		encoder->Append<uint8_t>(size);

		for (int i = 0; i < size; i++) {
			lua_rawgeti(L, index, i + 1);
			field->encodeFunc_(L, field, encoder, index + 1, depth);
			lua_pop(L, 1);
		}
	} else {
		field->encodeFunc_(L, field, encoder, index, depth);
	}
}

static inline void DecodeBool(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	bool val = decoder->Read<bool>();
	lua_pushboolean(L, val);
}

static inline void DecodeByte(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	int8_t val = decoder->Read<int8_t>();
	lua_pushinteger(L, val);
}

static inline void DecodeShort(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	short val = decoder->Read<short>();
	lua_pushinteger(L, val);
}

static inline void DecodeInt(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	int64_t val = decoder->Read<int64_t>();
	lua_pushinteger(L, val);
}

static inline void DecodeFloat(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	float val = decoder->Read<float>();
	lua_pushnumber(L, val);
}

static inline void DecodeDouble(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	double val = decoder->Read<double>();
	lua_pushnumber(L, val);
}

static inline void DecodeString(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	uint16_t size;
	const char* val = decoder->Read(&size);
	lua_pushlstring(L, val, size);
}

static void DecodePto(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	depth++;
	if (depth > MAX_DEPTH) {
		luaL_error(L, "decode too depth");
	}

	luaL_checkstack(L, LUA_MINSTACK, NULL);

	int size = field->childs_.size();
	lua_createtable(L, 0, size);
	for (int i = 0; i < size; i++) {
		Field* child = field->GetField(i);
		DecodeOne(L, child, decoder, index + 1, depth);
	}
}

static void DecodeOne(lua_State* L, Field* field, Decoder* decoder, int index, int depth) {
	if (field->array_) {
		uint8_t size = decoder->Read<uint8_t>();
		lua_createtable(L, size, 0);
		for (int i = 1; i <= size; i++) {
			field->decodeFunc_(L, field, decoder, index + 1, depth);
			lua_rawseti(L, index + 1, i);
		}
		lua_setfield(L, index, field->name_);
	} else {
		field->decodeFunc_(L, field, decoder, index, depth);
		lua_setfield(L, index, field->name_);
	}
}

static void ExportField(lua_State* L, std::vector<Field*>& fields) {
	lua_newtable(L);
	for (uint32_t i = 0; i < fields.size(); i++) {
		lua_newtable(L);
		Field* field = fields[i];

		lua_pushstring(L, field->name_);
		lua_setfield(L, -2, "name");

		lua_pushinteger(L, field->array_);
		lua_setfield(L, -2, "array");

		lua_pushinteger(L, field->type_);
		lua_setfield(L, -2, "type");

		if (field->type_ == eType::Pto) {
			ExportField(L, field->childs_);
			lua_setfield(L, -2, "pto");
		}
		lua_seti(L, -2, i + 1);
	}
}


static void ImportField(lua_State* L, Context* ctx, std::vector<Field*>& fields, int index, int depth) {
	int size = lua_rawlen(L, index);
	for (int i = 1; i <= size; i++) {
		lua_rawgeti(L, index, i);

		lua_getfield(L, -1, "type");
		eType type = (eType)lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "array");
		bool array = lua_toboolean(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, -1, "name");
		const char* name = lua_tostring(L, -1);
		lua_pop(L, 1);

		EncodeFunc encodeFunc = NULL;
		DecodeFunc decodeFunc = NULL;
		switch (type) {
			case LuaPto::Bool:
				encodeFunc = EncodeBool;
				decodeFunc = DecodeBool;
				break;
			case LuaPto::Byte:
				encodeFunc = EncodeByte;
				decodeFunc = DecodeByte;
				break;
			case LuaPto::Short:
				encodeFunc = EncodeShort;
				decodeFunc = DecodeShort;
				break;
			case LuaPto::Int:
				encodeFunc = EncodeInt;
				decodeFunc = DecodeInt;
				break;
			case LuaPto::Float:
				encodeFunc = EncodeFloat;
				decodeFunc = DecodeFloat;
				break;
			case LuaPto::Double:
				encodeFunc = EncodeDouble;
				decodeFunc = DecodeDouble;
				break;
			case LuaPto::String:
				encodeFunc = EncodeDouble;
				decodeFunc = DecodeDouble;
				break;
			case LuaPto::Pto:
				encodeFunc = EncodePto;
				decodeFunc = DecodePto;
				break;
			default:
				break;
		}
		Field* field = new Field(name, array, type, encodeFunc, decodeFunc);

		if (type == eType::Pto) {
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

static int Import(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);

	lua_pushnil(L);
	while (lua_next(L, 2) != 0) {
		int id = lua_tointeger(L, -2);
		if (id >= 0xffff) {
			luaL_error(L, "pto id must less than 0xffff");
		}

		lua_getfield(L, -1, "name");
		const char* name = lua_tostring(L, -1);

		Protocol* pto = new Protocol(id, name);
		lua_pop(L, 1);

		lua_getfield(L, -1, "pto");

		int depth = 0;
		ImportField(L, ctx, pto->fields_, lua_gettop(L), ++depth);

		ctx->AddPto(id, pto);

		lua_pop(L, 2);
	}
	return 0;
}

static int Export(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);

	lua_pushstring(L, pto->name_);

	ExportField(L, pto->fields_);
	return 2;
}

static int EncodePto(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);
	Encoder* encoder = (Encoder*)lua_touserdata(L, 3);

	int depth = 1;
	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);

	for (uint32_t i = 0; i < pto->fields_.size(); i++) {
		Field* field = pto->GetField(i);
		lua_getfield(L, 4, field->name_);
		EncodeOne(L, field, encoder, 5, depth);
		lua_pop(L, 1);
	}

	if (encoder->offset_ > PTO_MAX) {
		luaL_error(L, "pto=%s size=%d too much", pto->name_, encoder->offset_);
	}

	return 0;
}

static int Encode(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);
	int makeHeader = luaL_optinteger(L, 3, 0);
	int toStr = luaL_optinteger(L, 4, 0);

	Encoder encoder(L);
	if (makeHeader == 1) {
		encoder.Append<uint16_t>(0);
		encoder.Append<uint16_t>(pto->id_);
	}

	lua_pushcfunction(L, EncodePto);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_pushlightuserdata(L, &encoder);
	lua_pushvalue(L, 5);
	if (lua_pcall(L, 4, 0, 0) != LUA_OK) {
		lua_pushboolean(L, false);
		lua_pushvalue(L, -2);
		return 2;
	}

	if (makeHeader == 1) {
		uint16_t length = encoder.offset_;
		memcpy(encoder.ptr_, &length, sizeof(uint16_t));
	}

	if (toStr) {
		lua_pushlstring(L, encoder.ptr_, encoder.offset_);
		return 1;
	}
	lua_pushlightuserdata(L, (void*)encoder.ptr_);
	lua_pushinteger(L, encoder.offset_);
	return 2;
}

static int DecodePto(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);
	Decoder* decoder = (Decoder*)lua_touserdata(L, 3);

	int depth = 1;
	luaL_checkstack(L, MAX_DEPTH * 2 + 8, NULL);
	lua_createtable(L, 0, pto->fields_.size());
	int top = lua_gettop(L);
	for (uint32_t i = 0; i < pto->fields_.size(); i++) {
		Field* field = pto->GetField(i);
		DecodeOne(L, field, decoder, top, depth);
	}
	if (decoder->offset_ != decoder->size_) {
		luaL_error(L, "invalid pto message");
	}
	return 1;
}

static int Decode(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	Protocol* pto = GetPto(L, ctx, 2);

	size_t size = 0;
	const char* str = NULL;
	switch (lua_type(L, 3)) {
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

	Decoder decoder(L, str, size);

	lua_pushcfunction(L, DecodePto);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_pushlightuserdata(L, (void*)&decoder);
	if (lua_pcall(L, 3, 1, 0) != LUA_OK) {
		lua_pushboolean(L, false);
		lua_pushvalue(L, -2);
		return 2;
	}

	lua_pushstring(L, pto->name_);

	return 2;
}

static int Release(lua_State* L) {
	Context* ctx = (Context*)lua_touserdata(L, 1);
	ctx->~Context();
	return 0;
}

static int Create(lua_State* L) {
	void* userdata = lua_newuserdata(L, sizeof(Context));
	new (userdata)Context();

	lua_pushvalue(L, -1);
	if (luaL_newmetatable(L, "pto")) {
		const luaL_Reg meta[] = {
			{"Import", Import},
			{"Export", Export},
			{"Encode", Encode},
			{"Decode", Decode},
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

static int Parse(lua_State* L) {
	size_t sz;
	const char* path = luaL_checklstring(L, 1, &sz);
	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushnil(L);
	while (lua_next(L, 2) != 0) {
		if (lua_type(L, -1) != LUA_TSTRING) {
			luaL_error(L, "parse pto name must string");
		}
		lua_pop(L, 1);
	}

	ParserContext ctx(path);

	lua_pushnil(L);
	while (lua_next(L, 2) != 0) {
		if (ctx.Import(lua_tostring(L, -1)) == false) {
			return 0;
		}
		lua_pop(L, 1);
	}

	ctx.Export(L);
	return 1;
}

int luaopen_ptocxx(lua_State* L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{"Create", Create},
		{"Parse", Parse},
		{NULL, NULL},
	};
	luaL_newlib(L, l);
	return 1;
}
