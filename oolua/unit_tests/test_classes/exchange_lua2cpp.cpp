#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "expose_stub_classes.h"



class Exchange_lua2cpp : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Exchange_lua2cpp);
	CPPUNIT_TEST(pullSignedInt_integerIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullSignedChar_integerIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullSignedShort_integerIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullUnsignedInt_integerIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullUnsignedChar_integerIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullUnsignedShort_integerIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullPlainChar_integerIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullFloat_numberIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullDouble_numberIsOnTheStack_resultEqualToInput);
	CPPUNIT_TEST(pullInts_functionReturnsTheTwoIntsInputted_resultsEqualToInputs);
	CPPUNIT_TEST(pullClass_classIsOnTheStack_addressComparesEqualToInput);
	CPPUNIT_TEST(pullClasses_functionReturnsTheTwoDifferentInputtedClasses_pointersComparesEqualToInputs);
	CPPUNIT_TEST(pullTable_luaTableOnStack_tableIsValid);
	CPPUNIT_TEST(pullLuaFunction_luaFunctionOnStack_functionIsValid);

	CPPUNIT_TEST(pullTable_nilOnStack_tableIsInValid);
	CPPUNIT_TEST(pullTable_luaTableOnStack_pullReturnsTrue);
	CPPUNIT_TEST(pullTableRef_nilOnStack_tableIsInValid);
	CPPUNIT_TEST(pullTableRef_validTableOnStack_tableIsValid);
	CPPUNIT_TEST(pullLuaFunction_nilOnStack_functionIsInValid);

#if OOLUA_USE_EXCEPTIONS == 1
	CPPUNIT_TEST(pullLuaFuncRef_stackIsEmpty_throwsRunTimeError);
#else
	CPPUNIT_TEST(pullLuaFuncRef_stackIsEmpty_callReturnsFalse);
	CPPUNIT_TEST(pullLuaFuncRef_stackIsEmpty_errorStringIsNotEmpty);
#endif
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
public:
	Exchange_lua2cpp()
		: m_lua(0)
	{}
	LVD_NOCOPY(Exchange_lua2cpp)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	void pullSignedInt_integerIsOnTheStack_resultEqualToInput()
	{
		signed int input = -55; lua_pushinteger(*m_lua, input);
		signed int result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}

	void pullSignedChar_integerIsOnTheStack_resultEqualToInput()
	{
		signed char input(-1); lua_pushinteger(*m_lua, input);
		signed char result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}

	void pullSignedShort_integerIsOnTheStack_resultEqualToInput()
	{
		signed short input(-255); lua_pushinteger(*m_lua, input);
		signed short result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}

	void pullUnsignedInt_integerIsOnTheStack_resultEqualToInput()
	{
		unsigned int input = 5; lua_pushinteger(*m_lua, input);
		unsigned int result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}

	void pullUnsignedChar_integerIsOnTheStack_resultEqualToInput()
	{
		unsigned char input(1); lua_pushinteger(*m_lua, input);
		unsigned char result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}

	void pullUnsignedShort_integerIsOnTheStack_resultEqualToInput()
	{
		unsigned short input(255); lua_pushinteger(*m_lua, input);
		unsigned short result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}

	void pullPlainChar_integerIsOnTheStack_resultEqualToInput()
	{
		char input(120); lua_pushinteger(*m_lua, input);
		char result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(input, result);
	}

	void pullFloat_numberIsOnTheStack_resultEqualToInput()
	{
		float input(51.1234f); lua_pushnumber(*m_lua, input);
		float  result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(input, result, 0.1f);
	}

	void pullDouble_numberIsOnTheStack_resultEqualToInput()
	{
		double input(5551.1234); lua_pushnumber(*m_lua, input);
		double  result; OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(input, result, 0.1);
	}

	void pullClass_classIsOnTheStack_addressComparesEqualToInput()
	{
		m_lua->register_class<Stub1>();
		Stub1 input;
		Stub1* result(0);
		OOLUA::push(*m_lua, &input);
		OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(&input, result);
	}

	void pullClasses_functionReturnsTheTwoDifferentInputtedClasses_pointersComparesEqualToInputs()
	{
		m_lua->register_class<Stub1>();
		m_lua->register_class<Stub2>();
		Stub1 input1;
		Stub1* result1(0);
		Stub2 input2;
		Stub2* result2(0);

		m_lua->run_chunk("return function(...) return ... end");
		m_lua->call(1, &input1, &input2);
		OOLUA::pull(*m_lua, result2);
		OOLUA::pull(*m_lua, result1);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("input2 is incorrect", &input2, result2);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("input1 is incorrect", &input1, result1);
	}

	void pullInts_functionReturnsTheTwoIntsInputted_resultsEqualToInputs()
	{
		int input1 = 54;
		int input2 = 2;
		int result1(0);
		int result2(0);

		m_lua->run_chunk("return function(...) return ... end");
		m_lua->call(1, &input1, &input2);

		OOLUA::pull(*m_lua, result2);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("input2 is incorrect", input2, result2);
		OOLUA::pull(*m_lua, result1);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("input1 is incorrect", input1, result1);
	}


	void pullTable_luaTableOnStack_tableIsValid()
	{
		lua_createtable(*m_lua, 0, 0);
		OOLUA::Table table;
		OOLUA::pull(*m_lua, table);
		CPPUNIT_ASSERT_EQUAL(true, table.valid());
	}

	void pullTable_luaTableOnStack_pullReturnsTrue()
	{
		lua_createtable(*m_lua, 0, 0);
		OOLUA::Table table;
		bool result = OOLUA::pull(*m_lua, table);
		CPPUNIT_ASSERT_EQUAL(true, result);
	}

	/**[FunctionReferencePull]*/
	void pullLuaFunction_luaFunctionOnStack_functionIsValid()
	{
		lua_pushcclosure(*m_lua, lua_gettop, 0);
		OOLUA::Lua_func_ref lua_func;
		OOLUA::pull(*m_lua, lua_func);
		CPPUNIT_ASSERT_EQUAL(true, lua_func.valid());
	}
	/**[FunctionReferencePull]*/


	void pullTable_nilOnStack_tableIsInValid()
	{
		//start with a valid table
		OOLUA::Table table(OOLUA::new_table(*m_lua));
		lua_pushnil(*m_lua);
		OOLUA::pull(*m_lua, table);
		CPPUNIT_ASSERT_EQUAL(false, table.valid());
	}

	void pullTableRef_nilOnStack_tableIsInValid()
	{
		OOLUA::Lua_table_ref table;
		//start with a valid table
		{
			lua_createtable(*m_lua, 0, 0);
			OOLUA::pull(*m_lua, table);
		}
		lua_pushnil(*m_lua);
		OOLUA::pull(*m_lua, table);
		CPPUNIT_ASSERT_EQUAL(false, table.valid());
	}

	/**[PullingTableRefOffTheStack]*/
	void pullTableRef_validTableOnStack_tableIsValid()
	{
		lua_createtable(*m_lua, 0, 0);
		OOLUA::Lua_table_ref table;
		OOLUA::pull(*m_lua, table);
		CPPUNIT_ASSERT_EQUAL(true, table.valid());
	}
	/**[PullingTableRefOffTheStack]*/

	void pullLuaFunction_nilOnStack_functionIsInValid()
	{
		OOLUA::Lua_func_ref lua_func;
		//start with a valid function
		{
			lua_pushcclosure(*m_lua, lua_gettop, 0);
			OOLUA::pull(*m_lua, lua_func);
		}
		lua_pushnil(*m_lua);
		OOLUA::pull(*m_lua, lua_func);
		CPPUNIT_ASSERT_EQUAL(false, lua_func.valid());
	}

#if OOLUA_USE_EXCEPTIONS == 1
	void pullLuaFuncRef_stackIsEmpty_throwsRunTimeError()
	{
		OOLUA::Lua_func_ref func;
		CPPUNIT_ASSERT_THROW(OOLUA::pull(*m_lua, func), OOLUA::Runtime_error);
	}
#else
	void pullLuaFuncRef_stackIsEmpty_callReturnsFalse()
	{
		OOLUA::Lua_func_ref func;
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::pull(*m_lua, func));
	}

	void pullLuaFuncRef_stackIsEmpty_errorStringIsNotEmpty()
	{
		OOLUA::Lua_func_ref func;
		OOLUA::pull(*m_lua, func);
		std::string error_str = OOLUA::get_last_error(*m_lua);
		CPPUNIT_ASSERT_EQUAL(false, error_str.empty());
	}
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(Exchange_lua2cpp);
