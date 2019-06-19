#ifndef EXPOSE_METHOD_RETURNS_CLASS_H_
#	define EXPOSE_METHOD_RETURNS_CLASS_H_

#	include "oolua_dsl.h"
#	include "expose_stub_classes.h"
#	include "cpp_userdata_function_returns.h"


//type qualifiers ignored on function return type [-Wignored-qualifiers]
#	if defined __GNUC__ && defined __STRICT_ANSI__
#		pragma GCC system_header
#	endif

OOLUA_PROXY(UserDataFunctionDefaultReturnTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(refConst)
	OOLUA_MFUNC(ptr)
	OOLUA_MFUNC(ptrConst)
	OOLUA_MFUNC(refPtrConst)
	OOLUA_MFUNC(refConstPtrConst)
OOLUA_PROXY_END

OOLUA_PROXY(UserDataFunctionReturnTraits)
	OOLUA_TAGS(Abstract)
	/**[UserDataProxyLuaReturnTrait]*/
	OOLUA_MEM_FUNC(lua_return<Stub1*>, ptr)
	/**[UserDataProxyLuaReturnTrait]*/
	OOLUA_MEM_FUNC(lua_return<Stub1 const*>, ptrConst)
	OOLUA_MEM_FUNC(lua_return<Stub1 const*&>, refPtrConst)
	OOLUA_MEM_FUNC(lua_return<Stub1 const*const&>, refConstPtrConst)
OOLUA_PROXY_END

OOLUA_PROXY(UserDataFunctionReturnMaybeNullTraits)
	OOLUA_TAGS(Abstract)
	/**[UserDataProxyMaybeNullTrait]*/
	OOLUA_MEM_FUNC(maybe_null<Stub1*>, ptr)
	/**[UserDataProxyMaybeNullTrait]*/
	OOLUA_MEM_FUNC(maybe_null<Stub1 *const>, constPtr)
	OOLUA_MEM_FUNC(maybe_null<Stub1 const*>, ptrConst)
	OOLUA_MEM_FUNC(maybe_null<Stub1 const*const>, constPtrConst)
OOLUA_PROXY_END

int lua_proxy_returns_null_ptr(lua_State* vm);
int lua_proxy_returns_valid_ptr(lua_State* vm);
int lua_proxy_returns_lua_null_ptr(lua_State* vm);
int lua_proxy_returns_lua_valid_ptr(lua_State* vm);

OOLUA_PROXY(UserDataFunctionReturnLuaMaybeNullTraits)
	OOLUA_TAGS(Abstract)
	/**[UserDataProxyLuaMaybeNullTrait]*/
	OOLUA_MEM_FUNC(lua_maybe_null<Stub1*>, ptr)
	/**[UserDataProxyLuaMaybeNullTrait]*/
	OOLUA_MEM_FUNC(lua_maybe_null<Stub1 *const>, constPtr)
	OOLUA_MEM_FUNC(lua_maybe_null<Stub1 const*>, ptrConst)
	OOLUA_MEM_FUNC(lua_maybe_null<Stub1 const*const>, constPtrConst)
OOLUA_PROXY_END


OOLUA_PROXY(LightUserDataFunctionReturnTraits)
	OOLUA_TAGS(Abstract)
	/**[lightReturnVoidProxy]*/
	OOLUA_MEM_FUNC(light_return<void*>, value)
	/**[lightReturnVoidProxy]*/
OOLUA_PROXY_END

OOLUA_PROXY(CastLightUserDataFunctionReturnTraits)
	OOLUA_TAGS(Abstract)
	/**[lightReturnNoneVoidProxy]*/
	OOLUA_MEM_FUNC(light_return<InvalidStub*>, ptr)
	/**[lightReturnNoneVoidProxy]*/
OOLUA_PROXY_END

#endif
