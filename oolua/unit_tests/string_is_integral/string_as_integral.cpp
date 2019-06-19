#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include <string>
#	include "cpp_function_params.h"
#	include "cpp_function_returns.h"

struct oolua_string_traits{};
typedef FunctionParamType<oolua_string_traits, std::string> StringFunctionTraits;
typedef MockFunctionParamType<oolua_string_traits, std::string> StringFunctionTraitsMock;

typedef FunctionReturnType<oolua_string_traits, std::string> StringFunctionReturnTraits;
typedef MockFunctionReturnType<oolua_string_traits, std::string> StringFunctionReturnTraitsMock;


OOLUA_PROXY(StringFunctionTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(ref)
	//OOLUA_MFUNC(constant)
	OOLUA_MEM_FUNC(void, constant, std::string const)
	OOLUA_MFUNC(refConst)
	OOLUA_MEM_FUNC_RENAME(outTraitRef, void, ref, out_p<std::string&>)
	OOLUA_MEM_FUNC_RENAME(inOutTraitRef, void, ref, in_out_p<std::string&>)
OOLUA_PROXY_END

OOLUA_EXPORT_FUNCTIONS(StringFunctionTraits
						, value
						, ref
						, constant
						, refConst
						, outTraitRef
						, inOutTraitRef)
OOLUA_EXPORT_FUNCTIONS_CONST(StringFunctionTraits)


OOLUA_PROXY(StringFunctionReturnTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(constant)
//	OOLUA_MEM_FUNC(std::string const, constant)
	OOLUA_MFUNC(refConst)
OOLUA_PROXY_END

OOLUA_EXPORT_FUNCTIONS(StringFunctionReturnTraits
						, value
						, ref
						, constant
						, refConst)
OOLUA_EXPORT_FUNCTIONS_CONST(StringFunctionReturnTraits)


class StringConstructorMock
{
public:
	StringConstructorMock(std::string input)
		: m_input(input)
	{}
	std::string m_input;
};

OOLUA_PROXY(StringConstructorMock)
	OOLUA_TAGS(No_default_constructor)
	OOLUA_CTORS(
		OOLUA_CTOR(std::string)
	)
OOLUA_PROXY_END

OOLUA_EXPORT_NO_FUNCTIONS(StringConstructorMock)

class StringAsIntegral : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(StringAsIntegral);
		CPPUNIT_TEST(defaultTraitParam_value_calledOnceWithCorrectValue);
		CPPUNIT_TEST(defaultTraitParam_ref_calledOnceWithCorrectValue);
		CPPUNIT_TEST(defaultTraitParam_const_calledOnceWithCorrectValue);
		CPPUNIT_TEST(defaultTraitParam_refConst_calledOnceWithCorrectValue);
		CPPUNIT_TEST(outTraitRef_luaDoesNotPassString_calledOnceWithEmptyString);
		CPPUNIT_TEST(outTraitRef_luaDoesNotPassStringWillAssignValue_topOfStackIsExpectedValue);
		CPPUNIT_TEST(inOutTraitRef_luaPassesValue_calledOnceWithCorrectValue);
		CPPUNIT_TEST(inOutTraitRef_luaPassesValue_topOfStackIsExpectedValue);

#	if OOLUA_USE_EXCEPTIONS == 1
		CPPUNIT_TEST(defaultTraitParam_valuePassedInt_throwsRunTimeError);
#	elif OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(defaultTraitParam_valuePassedInt_callReturnsFalse);
#	endif

		CPPUNIT_TEST(returnTraitValue_cppReturnsValue_topOfStackIsExpectedValue);
		CPPUNIT_TEST(returnTraitRef_cppReturnsRef_topOfStackIsExpectedValue);
		CPPUNIT_TEST(returnTraitConst_cppReturnsConst_topOfStackIsExpectedValue);
		CPPUNIT_TEST(returnTraitRefConst_cppReturnsRefConst_topOfStackIsExpectedValue);

		CPPUNIT_TEST(constructor_stdString_calledWithExpectedInput);

#	if OOLUA_USE_EXCEPTIONS == 1
		CPPUNIT_TEST(constructor_stdStringPassedInt_throwsRunTimeError);
#	elif OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(constructor_stdStringPassedInt_runChunkReturnsFalse);
#	endif

	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;

	void assert_top_of_stack_is_expected_value(std::string & expected)
	{
		std::string top_of_stack;
		OOLUA::pull(*m_lua, top_of_stack);
		CPPUNIT_ASSERT_EQUAL(expected, top_of_stack);
	}
public:
	StringAsIntegral()
		: m_lua(0)
	{}
	LVD_NOCOPY(StringAsIntegral)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	struct Helper
	{
		Helper(lua_State* vm)
			: mock()
			, object(&mock)
			, input("std::string buffer")
		{
			OOLUA::register_class<StringFunctionTraits>(vm);
		}
		StringFunctionTraitsMock mock;
		StringFunctionTraits* object;
		std::string input;
	};

	void defaultTraitParam_value_calledOnceWithCorrectValue()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, value(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:value(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

	void defaultTraitParam_ref_calledOnceWithCorrectValue()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:ref(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

	void defaultTraitParam_const_calledOnceWithCorrectValue()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, constant(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:constant(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

	void defaultTraitParam_refConst_calledOnceWithCorrectValue()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, refConst(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:refConst(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

	void outTraitRef_luaDoesNotPassString_calledOnceWithEmptyString()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::StrEq(std::string()))).Times(1);
		m_lua->run_chunk("return function(object) object:outTraitRef() end");
		m_lua->call(1, helper.object);
	}

	void outTraitRef_luaDoesNotPassStringWillAssignValue_topOfStackIsExpectedValue()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(helper.input));
		m_lua->run_chunk("return function(object) return object:outTraitRef() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.input);
	}

	void inOutTraitRef_luaPassesValue_calledOnceWithCorrectValue()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::StrEq(helper.input))).Times(1);
		m_lua->run_chunk("return function(object, input) object:inOutTraitRef(input) end");
		m_lua->call(1, helper.object, helper.input);
	}

	void inOutTraitRef_luaPassesValue_topOfStackIsExpectedValue()
	{
		Helper helper(*m_lua);
		EXPECT_CALL(helper.mock, ref(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(helper.input));
		m_lua->run_chunk("return function(object) return object:inOutTraitRef('dontCare') end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.input);
	}

#	if OOLUA_USE_EXCEPTIONS == 1
	void defaultTraitParam_valuePassedInt_throwsRunTimeError()
	{
		Helper helper(*m_lua);
		m_lua->run_chunk("return function(object) object:value(1) end");
		CPPUNIT_ASSERT_THROW(m_lua->call(1, helper.object), OOLUA::Runtime_error);
	}
#	elif OOLUA_STORE_LAST_ERROR == 1
	void defaultTraitParam_valuePassedInt_callReturnsFalse()
	{
		Helper helper(*m_lua);
		m_lua->run_chunk("return function(object) object:value(1) end");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.object));
	}
#	endif


	struct ReturnHelper
	{
		ReturnHelper(lua_State* vm)
			: mock()
			, object(&mock)
			, output("std::string buffer")
		{
			OOLUA::register_class<StringFunctionReturnTraits>(vm);
		}
		StringFunctionReturnTraitsMock mock;
		StringFunctionReturnTraits* object;
		std::string output;
	};

	void returnTraitValue_cppReturnsValue_topOfStackIsExpectedValue()
	{
		ReturnHelper helper(*m_lua);
		EXPECT_CALL(helper.mock, value()).Times(1).WillOnce(::testing::Return(helper.output));
		m_lua->run_chunk("return function(object) return object:value() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.output);
	}

	void returnTraitRef_cppReturnsRef_topOfStackIsExpectedValue()
	{
		ReturnHelper helper(*m_lua);
		EXPECT_CALL(helper.mock, ref()).Times(1).WillOnce(::testing::ReturnRef(helper.output));
		m_lua->run_chunk("return function(object) return object:ref() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.output);
	}

	void returnTraitConst_cppReturnsConst_topOfStackIsExpectedValue()
	{
		ReturnHelper helper(*m_lua);
		EXPECT_CALL(helper.mock, constant()).Times(1).WillOnce(::testing::Return(helper.output));
		m_lua->run_chunk("return function(object) return object:constant() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.output);
	}

	void returnTraitRefConst_cppReturnsRefConst_topOfStackIsExpectedValue()
	{
		ReturnHelper helper(*m_lua);
		EXPECT_CALL(helper.mock, refConst()).Times(1).WillOnce(::testing::ReturnRef(helper.output));
		m_lua->run_chunk("return function(object) return object:refConst() end");
		m_lua->call(1, helper.object);
		assert_top_of_stack_is_expected_value(helper.output);
	}

	void constructor_stdString_calledWithExpectedInput()
	{
		m_lua->register_class<StringConstructorMock>();
		std::string const known_input = "ctor buffer";
		m_lua->run_chunk("return function(input) return StringConstructorMock.new(input) end");
		m_lua->call(1, known_input);

		lua_pushvalue(*m_lua, -1);
		StringConstructorMock* return_object(0);
		m_lua->pull(return_object);
		CPPUNIT_ASSERT_EQUAL(known_input, return_object->m_input);
	}

#	if OOLUA_USE_EXCEPTIONS == 1
	void constructor_stdStringPassedInt_throwsRunTimeError()
	{
		m_lua->register_class<StringConstructorMock>();
		CPPUNIT_ASSERT_THROW(m_lua->run_chunk("StringConstructorMock.new(1)"), OOLUA::Runtime_error);
	}
#	elif OOLUA_STORE_LAST_ERROR == 1
	void constructor_stdStringPassedInt_runChunkReturnsFalse()
	{
		m_lua->register_class<StringConstructorMock>();
		CPPUNIT_ASSERT_EQUAL(false, m_lua->run_chunk("StringConstructorMock.new(1)"));
	}
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringAsIntegral);
