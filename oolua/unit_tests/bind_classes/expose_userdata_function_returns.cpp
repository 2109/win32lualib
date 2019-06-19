
#include "oolua_dsl_export.h"
#include "expose_userdata_function_returns.h"

OOLUA_EXPORT_FUNCTIONS(UserDataFunctionDefaultReturnTraits
						, ref
						, refConst
						, ptr
						, ptrConst
						, refPtrConst
						, refConstPtrConst
						, value)

OOLUA_EXPORT_FUNCTIONS_CONST(UserDataFunctionDefaultReturnTraits)

OOLUA_EXPORT_FUNCTIONS(UserDataFunctionReturnTraits
						, ptr
						, ptrConst
						, refPtrConst
						, refConstPtrConst)

OOLUA_EXPORT_FUNCTIONS_CONST(UserDataFunctionReturnTraits)

OOLUA_EXPORT_FUNCTIONS(UserDataFunctionReturnMaybeNullTraits
						, ptr
						, constPtr
						, ptrConst
						, constPtrConst)
OOLUA_EXPORT_FUNCTIONS_CONST(UserDataFunctionReturnMaybeNullTraits)

int lua_proxy_returns_null_ptr(lua_State* vm)
{
	OOLUA_C_FUNCTION(OOLUA::maybe_null<Stub1*>, MaybeNullStaticFuncs::returns_null_ptr)
}

int lua_proxy_returns_valid_ptr(lua_State* vm)
{
	OOLUA_C_FUNCTION(OOLUA::maybe_null<Stub1*>, MaybeNullStaticFuncs::valid_ptr)
}

int lua_proxy_returns_lua_null_ptr(lua_State* vm)
{
	OOLUA_C_FUNCTION(OOLUA::lua_maybe_null<Stub1*>, MaybeNullStaticFuncs::returns_null_ptr)
}

int lua_proxy_returns_lua_valid_ptr(lua_State* vm)
{
	OOLUA_C_FUNCTION(OOLUA::lua_maybe_null<Stub1*>, MaybeNullStaticFuncs::valid_ptr)
}

OOLUA_EXPORT_FUNCTIONS(UserDataFunctionReturnLuaMaybeNullTraits
						, ptr
						, constPtr
						, ptrConst
						, constPtrConst)
OOLUA_EXPORT_FUNCTIONS_CONST(UserDataFunctionReturnLuaMaybeNullTraits)

OOLUA_EXPORT_FUNCTIONS(LightUserDataFunctionReturnTraits
						, value)
OOLUA_EXPORT_FUNCTIONS_CONST(LightUserDataFunctionReturnTraits)

OOLUA_EXPORT_FUNCTIONS(CastLightUserDataFunctionReturnTraits
						, ptr)
OOLUA_EXPORT_FUNCTIONS_CONST(CastLightUserDataFunctionReturnTraits)
