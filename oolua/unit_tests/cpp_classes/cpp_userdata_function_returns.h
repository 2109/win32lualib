#ifndef CPP_USERDATA_RETURNS_H_
#	define CPP_USERDATA_RETURNS_H_

#	include "cpp_function_returns.h"
#	include "cpp_stub_classes.h"

struct oolua_test_userdata_returns_default_traits{};
struct oolua_test_userdata_return_traits{};
struct oolua_test_userdata_maybe_null_traits{};
struct oolua_test_userdata_lua_maybe_null_traits{};
struct oolua_test_light_userdata_traits{};

typedef FunctionReturnType<oolua_test_userdata_returns_default_traits, Stub1> UserDataFunctionDefaultReturnTraits;
typedef MockFunctionReturnType<oolua_test_userdata_returns_default_traits, Stub1> UserDataFunctionDefaultReturnTraitsMock;

typedef FunctionReturnType<oolua_test_userdata_return_traits, Stub1> UserDataFunctionReturnTraits;
typedef MockFunctionReturnType<oolua_test_userdata_return_traits, Stub1> UserDataFunctionReturnTraitsMock;

typedef FunctionReturnType<oolua_test_userdata_maybe_null_traits, Stub1> UserDataFunctionReturnMaybeNullTraits;
typedef MockFunctionReturnType<oolua_test_userdata_maybe_null_traits, Stub1> UserDataFunctionReturnMaybeNullTraitsMock;

typedef FunctionReturnType<oolua_test_userdata_lua_maybe_null_traits, Stub1> UserDataFunctionReturnLuaMaybeNullTraits;
typedef MockFunctionReturnType<oolua_test_userdata_lua_maybe_null_traits, Stub1> UserDataFunctionReturnLuaMaybeNullTraitsMock;

typedef FunctionReturnType<oolua_test_light_userdata_traits, void*> LightUserDataFunctionReturnTraits;
typedef MockFunctionReturnType<oolua_test_light_userdata_traits, void*> LightUserDataFunctionReturnTraitsMock;



typedef FunctionReturnType<oolua_test_light_userdata_traits, InvalidStub> CastLightUserDataFunctionReturnTraits;
typedef MockFunctionReturnType<oolua_test_light_userdata_traits, InvalidStub> CastLightUserDataFunctionReturnTraitsMock;

struct MaybeNullStaticFuncs
{
	static Stub1* returns_null_ptr(){return NULL;}
	static Stub1* valid_ptr(){return &return_value;}
	static Stub1 return_value;
};

#endif
