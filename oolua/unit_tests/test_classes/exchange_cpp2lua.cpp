#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "expose_stub_classes.h"
#	include "cpp_class_ops.h"
#	include "expose_class_ops.h"


class Exchange_cpp2lua : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Exchange_cpp2lua);
		CPPUNIT_TEST(push_intPushed_topOfStackTypeIsNumber);
		CPPUNIT_TEST(push_intPushed_luaValueIsInput);
		CPPUNIT_TEST(push_shortPushed_luaValueIsInput);
		CPPUNIT_TEST(push_boolPushed_topOfStackTypeIsBoolean);
		CPPUNIT_TEST(push_boolPushed_luaValueIsInput);
		CPPUNIT_TEST(push_floatPushed_topOfStackTypeIsNumber);
		CPPUNIT_TEST(push_floatPushed_luaValueIsInput);
		CPPUNIT_TEST(push_doublePushed_luaValueIsInput);
		CPPUNIT_TEST(push_stringPushed_topOfStackTypeIsString);
		CPPUNIT_TEST(push_stringPushed_luaValueIsInput);
		CPPUNIT_TEST(push_stringWithNullsPushed_luaValueIsInput);

		CPPUNIT_TEST(push_classPointerPushed_topOfStackTypeIsUserdata);
		CPPUNIT_TEST(push_cFunctionPointer_topOfStackTypeIsFunction);

		CPPUNIT_TEST(call_callsLuaFunctionNoParams_callReturnsTrue);
		CPPUNIT_TEST(call_callsLuaFunctionOneParam_callReturnsTrue);
		CPPUNIT_TEST(call_callsLuaFunctionThreeParams_callReturnsTrue);

		CPPUNIT_TEST(push_invalidFunctionReference_stackSizeIncreasesByOne);
		CPPUNIT_TEST(push_invalidFunctionReference_stackTopisNil);
		CPPUNIT_TEST(push_invalidFunctionReference_pushReturnsTrue);
		CPPUNIT_TEST(push_validFunctionReference_topOfStackTypeIsFunction);

		CPPUNIT_TEST(push_invalidTable_stackSizeIncreasesByOne);
		CPPUNIT_TEST(push_invalidTable_stackTopIsNil);
		CPPUNIT_TEST(push_invalidTable_pushReturnsTrue);
		CPPUNIT_TEST(push_validTableReference_topOfStackTypeIsTable);

#if OOLUA_STORE_LAST_ERROR == 1
	CPPUNIT_TEST(call_callsUnknownLuaFunction_callReturnsFalse);

	CPPUNIT_TEST(push_FunctionReferenceFromDifferentLuaState_pushReturnsFalse);
	CPPUNIT_TEST(push_FunctionReferenceFromDifferentState_getLastErrorHasAnEntry);
	CPPUNIT_TEST(push_tableFromDifferentState_pushReturnsFalse);
	CPPUNIT_TEST(push_tableFromDifferentState_getLastErrorHasAnEntry);

#endif

#if OOLUA_USE_EXCEPTIONS == 1
	CPPUNIT_TEST(call_callsUnknownLuaFunction_throwsRuntimeError);

	CPPUNIT_TEST(push_FunctionReferenceFromDifferentLuaState_throwRuntimeError);
	CPPUNIT_TEST(push_tableFromDifferentState_throwRuntimeError);

	CPPUNIT_TEST(push_FunctionReferenceFromDifferentYetRelatedLuaState_returnsTrue);
	CPPUNIT_TEST(push_tableFromDifferentYetRelatedLuaState_returnsTrue);

#endif

	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
public:
	Exchange_cpp2lua()
		: m_lua(0)
	{}
	LVD_NOCOPY(Exchange_cpp2lua)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}
	template<typename T>
	std::string stringise(T t)
	{
		std::stringstream ss;
		ss <<t;
		return ss.str();
	}
	std::string stringise(std::string & s)
	{
		return std::string("\"") + s + std::string("\"");
	}
	std::string stringise(bool b)
	{
		return b? std::string("true") : std::string("false");
	}
	template<typename T>
	void assert_lua_value_is_input(T input)
	{
		m_lua->run_chunk(std::string("func = function(input) assert(input == ") + stringise(input) + std::string(") end") );
		//if the assert is fired in lua then call returns false
		CPPUNIT_ASSERT_EQUAL(true, m_lua->call("func", input));
	}
	template<typename T>
	void assert_top_of_stack_is_type_after_push(int type, T toBePushed)
	{
		OOLUA::push(*m_lua, toBePushed);
		CPPUNIT_ASSERT_EQUAL(type, lua_type(*m_lua, -1));
	}

	void push_intPushed_topOfStackTypeIsNumber()
	{
		int input(0);
		assert_top_of_stack_is_type_after_push(LUA_TNUMBER, input);
	}

	void push_intPushed_luaValueIsInput()
	{
		int input(0);
		assert_lua_value_is_input(input);
	}

	void push_shortPushed_luaValueIsInput()
	{
		short input(0);
		assert_lua_value_is_input(input);
	}

	void push_boolPushed_topOfStackTypeIsBoolean()
	{
		bool input(false);
		assert_top_of_stack_is_type_after_push(LUA_TBOOLEAN, input);
	}

	void push_boolPushed_luaValueIsInput()
	{
		bool input(false);
		assert_lua_value_is_input(input);
	}

	void push_floatPushed_topOfStackTypeIsNumber()
	{
		float input(0.f);
		assert_top_of_stack_is_type_after_push(LUA_TNUMBER, input);
	}

	void push_floatPushed_luaValueIsInput()
	{
		float input(0.f);
		assert_lua_value_is_input(input);
	}

	void push_doublePushed_luaValueIsInput()
	{
		double input(0.0);
		assert_lua_value_is_input(input);
	}

	void push_stringPushed_topOfStackTypeIsString()
	{
		std::string input;
		assert_top_of_stack_is_type_after_push(LUA_TSTRING, input);
	}

	void push_stringPushed_luaValueIsInput()
	{
		std::string input("hello world");
		assert_lua_value_is_input(input);
	}

	void push_stringWithNullsPushed_luaValueIsInput()
	{
		std::string input("\0hel\0lo wo\0rl\0d\0", 16);
		assert_lua_value_is_input(input);
	}

	void push_classPointerPushed_topOfStackTypeIsUserdata()
	{
		m_lua->register_class<Stub1>();
		Stub1 input;
		assert_top_of_stack_is_type_after_push(LUA_TUSERDATA, &input);
	}

	void push_cFunctionPointer_topOfStackTypeIsFunction()
	{
		assert_top_of_stack_is_type_after_push(LUA_TFUNCTION, lua_gettop);
	}

	void call_callsLuaFunctionNoParams_callReturnsTrue()
	{
		std::string foo("foo = function()end");
		m_lua->run_chunk(foo);
		CPPUNIT_ASSERT_EQUAL(true, m_lua->call("foo"));
	}

	void call_callsLuaFunctionOneParam_callReturnsTrue()
	{
		std::string foo("foo = function(i)end");
		m_lua->run_chunk(foo);
		CPPUNIT_ASSERT_EQUAL(true, m_lua->call("foo", 1));
	}

	void call_callsLuaFunctionThreeParams_callReturnsTrue()
	{
		std::string foo("foo = function(i, j, k)end");
		m_lua->run_chunk(foo);
		CPPUNIT_ASSERT_EQUAL(true, m_lua->call("foo", 1, 2, 3));
	}

	void push_invalidFunctionReference_pushReturnsTrue()
	{
		OOLUA::Lua_func_ref f;
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::push(*m_lua, f));
	}

	void push_invalidFunctionReference_stackSizeIncreasesByOne()
	{
		OOLUA::Lua_func_ref f;
		OOLUA::push(*m_lua, f);
		//we can not just check for nil on the stack as an empty stack
		//will return a false positive
		CPPUNIT_ASSERT_EQUAL(1, m_lua->stack_count());
	}

	void push_invalidFunctionReference_stackTopisNil()
	{
		OOLUA::Lua_func_ref f;
		OOLUA::push(*m_lua, f);
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}

	void push_validFunctionReference_topOfStackTypeIsFunction()
	{
		lua_pushcclosure(*m_lua, lua_gettop, 0);
		OOLUA::Lua_func_ref ref(*m_lua, luaL_ref(*m_lua, LUA_REGISTRYINDEX));
		OOLUA::push(*m_lua, ref);
		CPPUNIT_ASSERT_EQUAL(LUA_TFUNCTION, lua_type(*m_lua, 1));
	}

	void push_invalidTable_pushReturnsTrue()
	{
		OOLUA::Table t;
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::push(*m_lua, t) );
	}

	void push_invalidTable_stackSizeIncreasesByOne()
	{
		OOLUA::Table t;
		push(*m_lua, t);
		CPPUNIT_ASSERT_EQUAL(1, m_lua->stack_count());
	}

	void push_invalidTable_stackTopIsNil()
	{
		OOLUA::Table t;
		push(*m_lua, t);
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}

	void push_validTableReference_topOfStackTypeIsTable()
	{
		lua_createtable(*m_lua, 0, 0);
		OOLUA::Lua_table_ref ref(*m_lua, luaL_ref(*m_lua, LUA_REGISTRYINDEX));
		OOLUA::push(*m_lua, ref);
		CPPUNIT_ASSERT_EQUAL(LUA_TTABLE, lua_type(*m_lua, 1));
	}


	void pullFunctionReference(OOLUA::Lua_func_ref& f)
	{
		m_lua->load_chunk("return ");
		OOLUA::pull(*m_lua, f);
	}

	void pullValidTable(OOLUA::Table& t)
	{
		lua_createtable(*m_lua, 0, 0);
		OOLUA::pull(*m_lua, t);
	}

#if OOLUA_STORE_LAST_ERROR == 1
	void call_callsUnknownLuaFunction_callReturnsFalse()
	{
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call("foo"));
	}

	void push_tableFromDifferentState_pushReturnsFalse()
	{
		OOLUA::Table t;
		pullValidTable(t);

		OOLUA::Script s;
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::push(s, t));
	}

	void push_tableFromDifferentState_getLastErrorHasAnEntry()
	{
		OOLUA::Table t;
		pullValidTable(t);

		OOLUA::Script s;
		OOLUA::push(s, t);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::get_last_error(s).empty());
	}

	void push_FunctionReferenceFromDifferentLuaState_pushReturnsFalse()
	{
		OOLUA::Lua_func_ref f;
		pullFunctionReference(f);

		OOLUA::Script s;
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::push(s, f));
	}

	void push_FunctionReferenceFromDifferentState_getLastErrorHasAnEntry()
	{
		OOLUA::Lua_func_ref f;
		pullFunctionReference(f);

		OOLUA::Script s;
		OOLUA::push(s, f);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::get_last_error(s).empty());
	}
#endif

#if OOLUA_USE_EXCEPTIONS == 1
	void call_callsUnknownLuaFunction_throwsRuntimeError()
	{
		CPPUNIT_ASSERT_THROW(m_lua->call("foo"), OOLUA::Runtime_error);
	}

	void push_tableFromDifferentState_throwRuntimeError()
	{
		OOLUA::Table t;
		pullValidTable(t);

		OOLUA::Script s;
		CPPUNIT_ASSERT_THROW(OOLUA::push(s, t), OOLUA::Runtime_error);
	}

	void push_tableFromDifferentYetRelatedLuaState_returnsTrue()
	{
		lua_State* child = lua_newthread(*m_lua);
		OOLUA::Table t;
		pullValidTable(t);

		CPPUNIT_ASSERT_EQUAL(true, OOLUA::push(child, t));
	}

	void push_FunctionReferenceFromDifferentLuaState_throwRuntimeError()
	{
		OOLUA::Lua_func_ref f;
		pullFunctionReference(f);

		OOLUA::Script s;
		CPPUNIT_ASSERT_THROW(OOLUA::push(s, f), OOLUA::Runtime_error);
	}

	void push_FunctionReferenceFromDifferentYetRelatedLuaState_returnsTrue()
	{
		lua_State* child = lua_newthread(*m_lua);

		OOLUA::Lua_func_ref f;
		pullFunctionReference(f);

		CPPUNIT_ASSERT_EQUAL(true, OOLUA::push(child, f));
	}

#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(Exchange_cpp2lua);
