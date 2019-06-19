
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include "expose_userdata_function_params.h"



class UserDataFunctionInParams : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(UserDataFunctionInParams);

#if  OOLUA_USE_EXCEPTIONS == 1
		CPPUNIT_TEST(inTraitConst_ref_throwsRuntimeError);
		CPPUNIT_TEST(inTraitConst_ptr_throwsRuntimeError);
		CPPUNIT_TEST(inTraitConst_refPtr_throwsRuntimeError);
#elif OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(inTraitConst_ref_callReturnsFalse);
		CPPUNIT_TEST(inTraitConst_ptr_callReturnsFalse);
		CPPUNIT_TEST(inTraitConst_refPtr_callReturnsFalse);
#endif
		CPPUNIT_TEST(inTrait_value_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTraitConst_value_calledOnceWithCorrectValue);

		CPPUNIT_TEST(inTrait_constant_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTraitConst_constant_calledOnceWithCorrectValue);

		CPPUNIT_TEST(inTrait_ref_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTrait_ptr_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTrait_refPtr_calledOnceWithCorrectValue);

		CPPUNIT_TEST(inTrait_refConst_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTraitConst_refConst_calledOnceWithCorrectValue);

		CPPUNIT_TEST(inTrait_ptrConst_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTraitConst_ptrConst_calledOnceWithCorrectValue);

		CPPUNIT_TEST(inTrait_refPtrConst_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTraitConst_refPtrConst_calledOnceWithCorrectValue);

		CPPUNIT_TEST(inTrait_constPtrConst_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTraitConst_constPtrConst_calledOnceWithCorrectValue);

		CPPUNIT_TEST(inTrait_refConstPtrConst_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inTraitConst_refConstPtrConst_calledOnceWithCorrectValue);
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

	struct InHelper
	{
		InHelper(OOLUA::Script * lua)
			: m_lua(lua)
			, mock()
			, object(&mock)
			, inputParam(2)
			, inputParam_ptrConst(&inputParam)
		{
			m_lua->register_class<InParamUserData>();
			m_lua->register_class<HasIntMember>();
		}
		void run_method()
		{
			m_lua->run_chunk("return function(object, method,stub) object[method](object, stub) end");
		}
		OOLUA::Script * m_lua;
		InParamUserDataMock mock;
		InParamUserData* object;
		HasIntMember inputParam;
		HasIntMember const* inputParam_ptrConst;
	};

	void inTrait_value_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, value(::testing::Eq(helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "value", &helper.inputParam);
	}

	void inTraitConst_value_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, value(::testing::Eq(helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "value", helper.inputParam_ptrConst);
	}

	void inTrait_constant_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constant(::testing::Eq(helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "constant", &helper.inputParam);
	}

	void inTraitConst_constant_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constant(::testing::Eq(helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "constant", helper.inputParam_ptrConst);
	}

	void inTrait_ref_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::Ref(helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "ref", &helper.inputParam);
	}

#if OOLUA_USE_EXCEPTIONS == 1
	void inTraitConst_ref_throwsRuntimeError()
	{
		InHelper helper(m_lua);
		helper.run_method();
		CPPUNIT_ASSERT_THROW(m_lua->call(1, helper.object, "ref", helper.inputParam_ptrConst), OOLUA::Runtime_error);
	}
#elif OOLUA_STORE_LAST_ERROR == 1
	void inTraitConst_ref_callReturnsFalse()
	{
		InHelper helper(m_lua);
		helper.run_method();
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.object, "ref", helper.inputParam_ptrConst));
	}
#endif

	void inTrait_refConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refConst(::testing::Ref(helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "refConst", &helper.inputParam);
	}

	void inTraitConst_refConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refConst(::testing::Ref(helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "refConst", helper.inputParam_ptrConst);
	}

	void inTrait_ptr_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr(::testing::Eq(&helper.inputParam))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "ptr", &helper.inputParam);
	}

#if OOLUA_USE_EXCEPTIONS == 1
	void inTraitConst_ptr_throwsRuntimeError()
	{
		InHelper helper(m_lua);
		helper.run_method();
		CPPUNIT_ASSERT_THROW((m_lua->call(1, helper.object, "ptr", helper.inputParam_ptrConst)), OOLUA::Runtime_error);
	}
#elif OOLUA_STORE_LAST_ERROR == 1
	void inTraitConst_ptr_callReturnsFalse()
	{
		InHelper helper(m_lua);
		helper.run_method();
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.object, "ptr", helper.inputParam_ptrConst));
	}
#endif

	void inTrait_ptrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "ptrConst", &helper.inputParam);
	}

	void inTraitConst_ptrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "ptrConst", helper.inputParam_ptrConst);
	}

	void inTrait_refPtr_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		HasIntMember * ptr_instance = &helper.inputParam;
		EXPECT_CALL(helper.mock, refPtr(::testing::Eq(ptr_instance))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "refPtr", &helper.inputParam);
	}

#if OOLUA_USE_EXCEPTIONS == 1
	void inTraitConst_refPtr_throwsRuntimeError()
	{
		InHelper helper(m_lua);
		helper.run_method();
		CPPUNIT_ASSERT_THROW((m_lua->call(1, helper.object, "refPtr", helper.inputParam_ptrConst)), OOLUA::Runtime_error);
	}
#elif OOLUA_STORE_LAST_ERROR == 1
	void inTraitConst_refPtr_callReturnsFalse()
	{
		InHelper helper(m_lua);
		helper.run_method();
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.object, "refPtr", helper.inputParam_ptrConst));
	}
#endif

	void inTrait_refPtrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refPtrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "refPtrConst", &helper.inputParam);
	}

	/**[UserDataProxyInTraitUsage]*/
	void inTraitConst_refPtrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refPtrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "refPtrConst", helper.inputParam_ptrConst);
	}
	/**[UserDataProxyInTraitUsage]*/

	void inTrait_constPtrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "constPtrConst", &helper.inputParam);
	}

	void inTraitConst_constPtrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "constPtrConst", helper.inputParam_ptrConst);
	}

	void inTrait_refConstPtrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refConstPtrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "refConstPtrConst", &helper.inputParam);
	}

	void inTraitConst_refConstPtrConst_calledOnceWithCorrectValue()
	{
		InHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refConstPtrConst(::testing::Eq(helper.inputParam_ptrConst))).Times(1);
		helper.run_method();
		m_lua->call(1, helper.object, "refConstPtrConst", helper.inputParam_ptrConst);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(UserDataFunctionInParams);
