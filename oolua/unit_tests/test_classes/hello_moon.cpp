#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include <cstdio>

/** [HelloMoonCFunc]*/
void say(char const* input)
{
	printf("%s from a standalone function\n", input);
}
/** [HelloMoonCFunc]*/

/** [HelloMoonCFuncMinimalistProxy]*/
OOLUA_CFUNC(say, l_say)
/** [HelloMoonCFuncMinimalistProxy]*/

/** [HelloMoonCFuncOverloaded]*/
void expressive_say(char const* input)
{
	printf("%s from a expressive function\n", input);
}
void expressive_say(int input)
{
	printf("Huh %d\n", input);
	CPPUNIT_ASSERT(0);
}
/** [HelloMoonCFuncOverloaded]*/

/** [HelloMoonCFuncOverloadedCast]*/
OOLUA_CFUNC( (( void(*)(char const*))expressive_say), cast_expressive_say)
/** [HelloMoonCFuncOverloadedCast]*/

/** [HelloMoonCFuncExpressiveProxy]*/
int expressive_lsay(lua_State* vm)
{
	OOLUA_C_FUNCTION(void, expressive_say, char const*)
}
/** [HelloMoonCFuncExpressiveProxy]*/


/** [HelloMoonClass]*/
struct Say
{
	void message(char const* input)
	{ printf("%s from a Class member function\n", input); }
	static void static_message(char const* input)
	{ printf("%s from a static Class function\n", input); }
};

OOLUA_PROXY(Say)
	OOLUA_MFUNC(message)
	OOLUA_SFUNC(static_message)
OOLUA_PROXY_END

OOLUA_EXPORT_FUNCTIONS(Say, message)
OOLUA_EXPORT_FUNCTIONS_CONST(Say)
/** [HelloMoonClass]*/


class Hello_moon : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Hello_moon);
	CPPUNIT_TEST(hello_minimalist_function);
	CPPUNIT_TEST(hello_expressive_function);
	CPPUNIT_TEST(hello_cast_minimalist_function);
	CPPUNIT_TEST(hello_function_no_registration);
	CPPUNIT_TEST(hello_class_function);
	CPPUNIT_TEST(hello_static_class_function);
	CPPUNIT_TEST_SUITE_END();
public:
	/** [HelloMoonCFuncMinimalistUsage]*/
	void hello_minimalist_function()
	{
		using namespace OOLUA; //NOLINT(build/namespaces)
		Script vm;
		set_global(vm, "say", l_say);
		run_chunk(vm, "say('Hello Lua')");
	}
	/** [HelloMoonCFuncMinimalistUsage]*/

	/** [HelloMoonCFuncExpressiveUsage]*/
	void hello_expressive_function()
	{
		using namespace OOLUA; //NOLINT(build/namespaces)
		Script vm;
		set_global(vm, "say", expressive_lsay);
		vm.run_chunk("say('Hello Lua')");
	}
	/** [HelloMoonCFuncExpressiveUsage]*/

	/** [HelloMoonCFuncCastUsage]*/
	void hello_cast_minimalist_function()
	{
		using namespace OOLUA; //NOLINT(build/namespaces)
		Script vm;
		set_global(vm, "say", cast_expressive_say);
		vm.run_chunk("say('Hello Lua, we are a cast function not')");
	}
	/** [HelloMoonCFuncCastUsage]*/

	/** [HelloMoonCFuncAndProxyUsageLua]*/
	void hello_function_no_registration()
	{
		using namespace OOLUA; //NOLINT(build/namespaces)
		lua_State* vm = luaL_newstate();
		set_global(vm, "say", l_say);
		run_chunk(vm, "say('Hello Lua')");
	}
	/** [HelloMoonCFuncAndProxyUsageLua]*/

	void hello_class_function()
	{
		using namespace OOLUA; //NOLINT(build/namespaces)
		Script vm;
		vm.register_class<Say>();
		run_chunk(vm, "Say.new():message('Hello Lua')");
	}
	void hello_static_class_function()
	{
		using namespace OOLUA; //NOLINT(build/namespaces)
		Script vm;
		vm.register_class<Say>();
		vm.register_class_static<Say>("static_message", Proxy_class<Say>::static_message);
		vm.run_chunk("Say.static_message('Hello Lua')");
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Hello_moon);
