
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"

#	include "expose_integral_function_returns.h"

class IntegralFunctionReturns : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(IntegralFunctionReturns);
		CPPUNIT_TEST(cString_functionReturnsPtrToChar_resultIsExpectedValue);
		CPPUNIT_TEST(cString_functionReturnsPtrToConstChar_resultIsExpectedValue);
	CPPUNIT_TEST_SUITE_END();
	OOLUA::Script* m_lua;
public:
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	void cString_functionReturnsPtrToChar_resultIsExpectedValue()
	{
		CStringFunctionReturnMock mock;
		CStringFunctionReturn* object = &mock;
		char * input = const_cast<char*>("hello world");
		EXPECT_CALL(mock, ptr()).Times(1).WillOnce(::testing::Return(input));

		m_lua->register_class<CStringFunctionReturn>();
		m_lua->run_chunk("return function(object) return object:ptr() end");
		m_lua->call(1, object);
		std::string result;
		m_lua->pull(result);
		CPPUNIT_ASSERT_EQUAL(std::string(input), result);
	}

	void cString_functionReturnsPtrToConstChar_resultIsExpectedValue()
	{
		CStringFunctionReturnMock mock;
		CStringFunctionReturn* object = &mock;
		char const * input = "hello world";
		EXPECT_CALL(mock, ptrConst()).Times(1).WillOnce(::testing::Return(input));

		m_lua->register_class<CStringFunctionReturn>();
		m_lua->run_chunk("return function(object) return object:ptrConst() end");
		m_lua->call(1, object);
		std::string result;
		m_lua->pull(result);
		CPPUNIT_ASSERT_EQUAL(std::string(input), result);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(IntegralFunctionReturns);
