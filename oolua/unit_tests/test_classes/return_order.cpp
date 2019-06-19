#	include "oolua_tests_pch.h"
#	include "common_cppunit_headers.h"
#	include "oolua.h"

/**[CppTraitReturnOrderOneParam]*/
struct ReturnOrder
{
	enum {returnValue=-1, paramValue};
	int foo(int& bar)
	{
		bar = paramValue;
		return returnValue;
	}
};
/**[CppTraitReturnOrderOneParam]*/

/**[ProxyTraitReturnOrderOneParam]*/
OOLUA_PROXY(ReturnOrder)
	OOLUA_MEM_FUNC(int, foo, in_out_p<int&>)
OOLUA_PROXY_END
/**[ProxyTraitReturnOrderOneParam]*/

OOLUA_EXPORT_FUNCTIONS(ReturnOrder, foo)
OOLUA_EXPORT_FUNCTIONS_CONST(ReturnOrder)

class TestingReturnOrder : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestingReturnOrder);
		CPPUNIT_TEST(luaReturnOrder_luaFunctionWhichReturnsMultipleValuesToCpp_orderFromTopOfStackIsInput2Input1);
		CPPUNIT_TEST(ordering_functionWhichHasAReturnValueAndAlsoReturnsAnInOutParam_topOfStackIsTheInOutParam);
		CPPUNIT_TEST(ordering_functionWhichHasAReturnValueAndAlsoReturnsAnInOutParam_slotBeneathTopOfStackIsFunctionReturn);
		CPPUNIT_TEST_SUITE_END();
	OOLUA::Script * m_lua;
public:
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	/**[TestLuaReturnOrder]*/
	void luaReturnOrder_luaFunctionWhichReturnsMultipleValuesToCpp_orderFromTopOfStackIsInput2Input1()
	{
		m_lua->run_chunk("return function(input1, input2) return input1, input2 end ");
		int input1(1);
		int input2(2);

		m_lua->call(1, input1, input2);
		/*
		  ========
		 | input2 | <-- stack top
		  ========
		 | input1 |
		  ========
		 |   ...  |
		 */
		int topOfStack, nextSlot;
		OOLUA::pull(*m_lua, topOfStack);
		OOLUA::pull(*m_lua, nextSlot);

		CPPUNIT_ASSERT_EQUAL(input2, topOfStack);
		CPPUNIT_ASSERT_EQUAL(input1, nextSlot);
	}
	/**[TestLuaReturnOrder]*/

	/**[TestTraitReturnOrderTop]*/
	void ordering_functionWhichHasAReturnValueAndAlsoReturnsAnInOutParam_topOfStackIsTheInOutParam()
	{
		/*
		  ======================
		 | param with out trait | <-- stack top
		  ======================
		 |   function return    |
		  ======================
		 |         ...          |
		 */
		ReturnOrder proxy;
		m_lua->register_class(&proxy);
		m_lua->run_chunk("return function(object) return object:foo(0--[[DontCareInitialParamValue--]]) end");
		m_lua->call(1, &proxy);

		int topOfStack;
		m_lua->pull(topOfStack);

		CPPUNIT_ASSERT_EQUAL(static_cast<int>(ReturnOrder::paramValue), topOfStack);
	}
	/**[TestTraitReturnOrderTop]*/

	/**[TestTraitReturnOrderNextSlot]*/
	void ordering_functionWhichHasAReturnValueAndAlsoReturnsAnInOutParam_slotBeneathTopOfStackIsFunctionReturn()
	{
		ReturnOrder proxy;
		m_lua->register_class(&proxy);
		m_lua->run_chunk("return function(object) return object:foo(0--[[DontCareInitialParamValue--]]) end");
		m_lua->call(1, &proxy);

		int dontCare, nextSlot;
		m_lua->pull(dontCare);
		m_lua->pull(nextSlot);

		CPPUNIT_ASSERT_EQUAL(static_cast<int>(ReturnOrder::returnValue), nextSlot);
	}
	/**[TestTraitReturnOrderNextSlot]*/
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestingReturnOrder);
