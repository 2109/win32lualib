#ifndef EXPOSE_CLASS_CONSTRUCTORS_H_
#	define EXPOSE_CLASS_CONSTRUCTORS_H_

#include "oolua_dsl.h"
#include "expose_stub_classes.h"
#include "cpp_constructor_mock.h"

/**[ExposeConstructors]*/
OOLUA_PROXY(ParamConstructor)
	OOLUA_TAGS(
		No_default_constructor
	)
	OOLUA_CTORS(
		OOLUA_CTOR(bool) /*NOLINT(readability/casting)*/
		OOLUA_CTOR(int) /*NOLINT(readability/casting)*/
		OOLUA_CTOR(char const*)
		OOLUA_CTOR(int, bool)
		OOLUA_CTOR(Stub1 *) /*NOLINT(readability/casting)*/
		OOLUA_CTOR(Stub1 *, Stub2*)
		OOLUA_CTOR(Stub2)
		OOLUA_CTOR(Stub3*) /*NOLINT(readability/casting)*/
		OOLUA_CTOR(Stub3 const *)
		OOLUA_CTOR(OOLUA::Lua_func_ref)
		OOLUA_CTOR(OOLUA::Table)
	)
OOLUA_PROXY_END
/**[ExposeConstructors]*/

OOLUA_PROXY(TableRefConstructor)
	OOLUA_TAGS(
		No_default_constructor
	)
	OOLUA_CTORS(
		OOLUA_CTOR(OOLUA::Lua_table_ref)
	)
OOLUA_PROXY_END


OOLUA_PROXY(WithOutConstructors)
	OOLUA_TAGS(
		No_public_constructors
	)
OOLUA_PROXY_END


OOLUA_PROXY(LuaStateConstructors)
	OOLUA_TAGS(
		No_default_constructor
	)
	OOLUA_CTORS(
		OOLUA_CTOR(calling_lua_state)
		OOLUA_CTOR(int, calling_lua_state)
	)
OOLUA_PROXY_END

OOLUA_PROXY(DefaultConstructorThrowsStdException)
OOLUA_PROXY_END

OOLUA_PROXY(ConstructorThrowsStdException)
	OOLUA_CTORS(
		OOLUA_CTOR(int) /*NOLINT(readability/casting)*/
	)
OOLUA_PROXY_END

OOLUA_PROXY(LightVoidUserDataConstructor)
	OOLUA_TAGS(
		No_default_constructor
	)
	OOLUA_CTORS(
		OOLUA_CTOR(light_p<void*>)
	)
	OOLUA_MEM_FUNC_CONST(light_return<void*>, get_ptr)
OOLUA_PROXY_END

OOLUA_PROXY(LightNoneVoidUserDataConstructor)
	OOLUA_TAGS(
		No_default_constructor
	)
	OOLUA_CTORS(
		OOLUA_CTOR(light_p<InvalidStub*>)
	)
	OOLUA_MEM_FUNC_CONST(light_return<InvalidStub*>, get_ptr)
OOLUA_PROXY_END

#endif
