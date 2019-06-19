#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include "expose_integral_function_params.h"


class IntegralFunctionOutParams : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(IntegralFunctionOutParams);
	CPPUNIT_TEST(inOutTraitRef_luaPassesInt_functionReceivesInputedValue);
	CPPUNIT_TEST(inOutTraitRef_luaPassesInt_functionPushesBackNumber);
	CPPUNIT_TEST(inOutTraitRef_luaPassesIntCppAssignsNewValue_returnIsNewlyAssignedValue);

	CPPUNIT_TEST(inOutTraitPtr_luaPassesInt_functionReceivesInputedValue);
	CPPUNIT_TEST(inOutTraitPtr_luaPassesInt_functionPushesBackNumber);
	CPPUNIT_TEST(inOutTraitPtr_luaPassesIntCppAssignsNewValue_returnIsNewlyAssignedValue);

	CPPUNIT_TEST(inOutTraitRefPtr_luaPassesInt_functionReceivesInputedValue);
	CPPUNIT_TEST(inOutTraitRefPtr_luaPassesInt_functionPushesBackNumber);
	CPPUNIT_TEST(inOutTraitRefPtr_luaPassesIntCppAssignsNewValue_returnIsNewlyAssignedValue);

	CPPUNIT_TEST(outTraitRef_luaDoesNotPassIntCppAssignsNewValue_returnIsNewlyAssignedValue);
	CPPUNIT_TEST(outTraitPtr_luaDoesNotPassIntCppAssignsNewValue_returnIsNewlyAssignedValue);
	CPPUNIT_TEST(outTraitRefPtr_luaDoesNotPassIntCppAssignsNewValue_returnIsNewlyAssignedValue);

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

	struct OutParamHelper
	{
		OutParamHelper(OOLUA::Script * m_lua)
			: mock()
			, object(&mock)
			, expected(2)
		{
			m_lua->register_class<IntegerFunctionOutTraits>();
		}
		IntegerFunctionOutTraitsMock mock;
		IntegerFunctionOutTraits* object;
		int expected;
	};

	struct InOutParamHelper
	{
		InOutParamHelper(OOLUA::Script * m_lua)
			: mock()
			, object(&mock)
			, input(1)
			, expected(2)
		{
			m_lua->register_class<IntegerFunctionInOutTraits>();
		}
		IntegerFunctionInOutTraitsMock mock;
		IntegerFunctionInOutTraits* object;
		int  input;
		int  expected;
	};
	template<typename InputAndResultType>
	void assert_top_of_stack_is_expected_value(InputAndResultType const& expected)
	{
		InputAndResultType top_of_stack(0);
		OOLUA::pull(*m_lua, top_of_stack);
		CPPUNIT_ASSERT_EQUAL(expected, top_of_stack);
	}

	void inOutTraitRef_luaPassesInt_functionReceivesInputedValue()
	{
		InOutParamHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::Eq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) return object:ref(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

	void inOutTraitRef_luaPassesInt_functionPushesBackNumber()
	{
		m_lua->register_class<IntegerFunctionInOutTraits>();
		::testing::NiceMock<IntegerFunctionInOutTraitsMock> fake;
		m_lua->run_chunk("return function(object) return object:ref(1) end");
		m_lua->call(1, static_cast<IntegerFunctionInOutTraits*>(&fake));
		CPPUNIT_ASSERT_EQUAL(LUA_TNUMBER, lua_type(*m_lua, -1));
	}

	/**[IntegerInOutParamUsage]*/
	void inOutTraitRef_luaPassesIntCppAssignsNewValue_returnIsNewlyAssignedValue()
	{
		InOutParamHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(helper.expected));
		m_lua->run_chunk("return function(object) return object:ref(1) end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.expected);
	}
	/**[IntegerInOutParamUsage]*/

	void inOutTraitPtr_luaPassesInt_functionReceivesInputedValue()
	{
		InOutParamHelper helper(m_lua);
		m_lua->run_chunk("return function(object, input) return object:ptr(input) end");
		EXPECT_CALL(helper.mock, ptr(::testing::Pointee(::testing::Eq(helper.input)))).Times(1);
		m_lua->call(1, helper.object, helper.input);
	}

	void inOutTraitPtr_luaPassesInt_functionPushesBackNumber()
	{
		m_lua->register_class<IntegerFunctionInOutTraits>();
		::testing::NiceMock<IntegerFunctionInOutTraitsMock> fake;
		m_lua->run_chunk("return function(object) return object:ptr(1) end");
		m_lua->call(1, static_cast<IntegerFunctionInOutTraits*>(&fake));
		CPPUNIT_ASSERT_EQUAL(LUA_TNUMBER, lua_type(*m_lua, -1));
	}

	void inOutTraitPtr_luaPassesIntCppAssignsNewValue_returnIsNewlyAssignedValue()
	{
		InOutParamHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr(::testing::_)).Times(1).WillOnce(::testing::SetArgumentPointee<0>(helper.expected));
		m_lua->run_chunk("return function(object) return object:ptr(1) end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.expected);
	}

	void inOutTraitRefPtr_luaPassesInt_functionReceivesInputedValue()
	{
		InOutParamHelper helper(m_lua);
		m_lua->run_chunk("return function(object, input) return object:refPtr(input) end");
		EXPECT_CALL(helper.mock, refPtr(::testing::Pointee(::testing::Eq(helper.input)))).Times(1);
		m_lua->call(1, helper.object, helper.input);
	}

	void inOutTraitRefPtr_luaPassesInt_functionPushesBackNumber()
	{
		m_lua->register_class<IntegerFunctionInOutTraits>();
		::testing::NiceMock<IntegerFunctionInOutTraitsMock> fake;
		m_lua->run_chunk("return function(object) return object:refPtr(1) end");
		m_lua->call(1, static_cast<IntegerFunctionInOutTraits*>(&fake));
		CPPUNIT_ASSERT_EQUAL(LUA_TNUMBER, lua_type(*m_lua, -1));
	}

	void inOutTraitRefPtr_luaPassesIntCppAssignsNewValue_returnIsNewlyAssignedValue()
	{
		InOutParamHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refPtr(::testing::_)).Times(1).WillOnce(::testing::SetArgumentPointee<0>(helper.expected));
		m_lua->run_chunk("return function(object) return object:refPtr(1) end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.expected);
	}

	void outTraitRef_luaDoesNotPassIntCppAssignsNewValue_returnIsNewlyAssignedValue()
	{
		OutParamHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(helper.expected));
		m_lua->run_chunk("return function(object) return object:ref() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.expected);
	}

	void outTraitPtr_luaDoesNotPassIntCppAssignsNewValue_returnIsNewlyAssignedValue()
	{
		OutParamHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr(::testing::_)).Times(1).WillOnce(::testing::SetArgumentPointee<0>(helper.expected));
		m_lua->run_chunk("return function(object) return object:ptr() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.expected);
	}

	void outTraitRefPtr_luaDoesNotPassIntCppAssignsNewValue_returnIsNewlyAssignedValue()
	{
		OutParamHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refPtr(::testing::_)).Times(1).WillOnce(::testing::SetArgumentPointee<0>(helper.expected));
		m_lua->run_chunk("return function(object) return object:refPtr() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.expected);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(IntegralFunctionOutParams);
