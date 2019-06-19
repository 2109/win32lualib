
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include "expose_integral_function_params.h"

namespace
{
	int const int_set_value =1;
} // namespace

struct IntParamHelper
{
	IntParamHelper(OOLUA::Script* vm)
		: mock()
		, mockBase(&mock)
	{
		vm->register_class<IntegerFunctionInTraits>();
	}
	IntegerFunctionInTraitsMock mock;
	IntegerFunctionInTraits* mockBase;
};


class Integral_params : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Integral_params);
	CPPUNIT_TEST(int_intParam_calledOnceWithCorrectParam);
	CPPUNIT_TEST(int_intRefParam_calledOnceWithCorrectParam);
	CPPUNIT_TEST(int_intPtrParam_calledOnceWithCorrectParam);
	CPPUNIT_TEST(int_intConstParam_calledOnceWithCorrectParam);
	CPPUNIT_TEST(int_intConstRefParam_calledOnceWithCorrectParam);
	CPPUNIT_TEST(int_intConstPtrParam_calledOnceWithCorrectParam);
	CPPUNIT_TEST(int_intConstPtrConstParam_calledOnceWithCorrectParam);

	CPPUNIT_TEST(string_ptrConstParam_calledOnceWithCorrectValue);
	CPPUNIT_TEST(string_refPtrConstParam_calledOnceWithCorrectValue);
	CPPUNIT_TEST(string_ptrParam_calledOnceWithCorrectValue);
#if OOLUA_USE_EXCEPTIONS == 1
	CPPUNIT_TEST(string_ptrConstParamPassedInt_runtimeError);
	CPPUNIT_TEST(string_ptrParamPassedInt_runtimeError);
#elif OOLUA_STORE_LAST_ERROR == 1
	CPPUNIT_TEST(string_ptrConstParamPassedInt_callReturnsFalse);
	CPPUNIT_TEST(string_ptrParamPassedInt_callReturnsFalse);
#endif

	CPPUNIT_TEST(float_floatParam_CalledOnceWithCorrectParamWithinEpsilon);
	CPPUNIT_TEST(double_doubleParam_CalledOnceWithCorrectParamWithinEpsilon);
	CPPUNIT_TEST(cFunction_memberFunctionWhichTakesLuaCFunction_calledOnceWithCorrectParam);
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script* m_lua;
	IntParamHelper* m_helper;

	template<typename T>
	void generate_run_and_call(std::string const& func_name, T& value)
	{
		m_lua->run_chunk("return function(obj, method, param) return obj[method](obj, param) end");
		m_lua->call(1, m_helper->mockBase, func_name, value);
	}
public:
	void setUp()
	{
		m_lua = new OOLUA::Script;
		m_helper = new IntParamHelper(m_lua);
	}
	void tearDown()
	{
		delete m_lua;
		delete m_helper;
	}
	void int_intParam_calledOnceWithCorrectParam()
	{
		EXPECT_CALL(m_helper->mock, value(::testing::Eq(int_set_value))).Times(1);
		generate_run_and_call("value", int_set_value);
	}

	void int_intRefParam_calledOnceWithCorrectParam()
	{
		EXPECT_CALL(m_helper->mock, ref(::testing::Eq(int_set_value))).Times(1);
		generate_run_and_call("ref", int_set_value);
	}

	void int_intPtrParam_calledOnceWithCorrectParam()
	{
		EXPECT_CALL(m_helper->mock, ptr(::testing::Pointee(int_set_value))).Times(1);
		generate_run_and_call("ptr", int_set_value);
	}

	void int_intConstParam_calledOnceWithCorrectParam()
	{
		EXPECT_CALL(m_helper->mock, constant(::testing::Eq(int_set_value))).Times(1);
		generate_run_and_call("constant", int_set_value);
	}

	void int_intConstRefParam_calledOnceWithCorrectParam()
	{
		EXPECT_CALL(m_helper->mock, refConst(::testing::Eq(int_set_value))).Times(1);
		generate_run_and_call("refConst", int_set_value);
	}

	void int_intConstPtrParam_calledOnceWithCorrectParam()
	{
		EXPECT_CALL(m_helper->mock, ptrConst(::testing::Pointee(int_set_value))).Times(1);
		generate_run_and_call("ptrConst", int_set_value);
	}

	void int_intConstPtrConstParam_calledOnceWithCorrectParam()
	{
		EXPECT_CALL(m_helper->mock, constPtrConst(::testing::Pointee(int_set_value))).Times(1);
		generate_run_and_call("constPtrConst", int_set_value);
	}

	struct StringHelper
	{
		StringHelper(lua_State* vm)
			: mock()
			, object(&mock)
			, input("input buffer")
		{
			OOLUA::register_class<CharFunctionInTraits>(vm);
		}
		CharFunctionInTraitsMock mock;
		CharFunctionInTraits* object;
		char const* input;
	};

	void string_ptrConstParam_calledOnceWithCorrectValue()
	{
		StringHelper helper(*m_lua);
		EXPECT_CALL(helper.mock, ptrConst(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:ptrConst(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

#if OOLUA_USE_EXCEPTIONS == 1
	void string_ptrConstParamPassedInt_runtimeError()
	{
		StringHelper helper(*m_lua);
		m_lua->run_chunk("return function(object) object:ptrConst(1) end");
		CPPUNIT_ASSERT_THROW(m_lua->call(1, helper.object), OOLUA::Runtime_error);
	}
#elif OOLUA_STORE_LAST_ERROR == 1
	void string_ptrConstParamPassedInt_callReturnsFalse()
	{
		StringHelper helper(*m_lua);
		m_lua->run_chunk("return function(object) object:ptrConst(1) end");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.object));
	}
#endif

	void string_refPtrConstParam_calledOnceWithCorrectValue()
	{
		StringHelper helper(*m_lua);
		EXPECT_CALL(helper.mock, refPtrConst(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:refPtrConst(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

	void string_ptrParam_calledOnceWithCorrectValue()
	{
		StringHelper helper(*m_lua);
		EXPECT_CALL(helper.mock, ptr(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:ptr(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

#if OOLUA_USE_EXCEPTIONS == 1
	void string_ptrParamPassedInt_runtimeError()
	{
		StringHelper helper(*m_lua);
		m_lua->run_chunk("return function(object) object:ptr(1) end");
		CPPUNIT_ASSERT_THROW(m_lua->call(1, helper.object), OOLUA::Runtime_error);
	}
#elif OOLUA_STORE_LAST_ERROR == 1
	void string_ptrParamPassedInt_callReturnsFalse()
	{
		StringHelper helper(*m_lua);
		m_lua->run_chunk("return function(object) object:ptr(1) end");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.object));
	}
#endif

	void float_floatParam_CalledOnceWithCorrectParamWithinEpsilon()
	{
		FloatFunctionInTraitsMock mock;
		FloatFunctionInTraits* object = &mock;
		float input = .5f;
		m_lua->register_class<FloatFunctionInTraits>();
		m_lua->run_chunk("return function(object, input) object:value(input) end");

		EXPECT_CALL(mock, value(::testing::FloatEq(input))).Times(1);
		m_lua->call(1, object, input);
	}

	void double_doubleParam_CalledOnceWithCorrectParamWithinEpsilon()
	{
		DoubleFunctionInTraitsMock mock;
		DoubleFunctionInTraits* object = &mock;
		double input = .5;
		m_lua->register_class<DoubleFunctionInTraits>();
		m_lua->run_chunk("return function(object, input) object:value(input) end");

		EXPECT_CALL(mock, value(::testing::DoubleEq(input))).Times(1);
		m_lua->call(1, object, input);
	}

	void cFunction_memberFunctionWhichTakesLuaCFunction_calledOnceWithCorrectParam()
	{
		CFunctionInTraitsMock mock;
		CFunctionInTraits* object = &mock;
		lua_CFunction input = lua_gettop;
		m_lua->register_class<CFunctionInTraits>();
		m_lua->run_chunk("return function(object, input) object:value(input) end");
		EXPECT_CALL(mock, value(::testing::Eq(input))).Times(1);
		m_lua->call(1, object, input);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Integral_params);
