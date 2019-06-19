
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "expose_stub_classes.h"
#	include "expose_userdata_function_returns.h"
#	include "expose_userdata_function_params.h"
#	include "expose_class_constructors.h"

class Light_ud_beta : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Light_ud_beta);
		CPPUNIT_TEST(push_voidPointerPushedToTheStack_topEqualsOne);
		CPPUNIT_TEST(push_voidPointerPushedToTheStack_typeOfStackTopEqualsLightUserData);
		CPPUNIT_TEST(push_voidPointerPushedToTheStack_pushReturnsTrue);
		CPPUNIT_TEST(push_nullPointerPushedToTheStack_typeOfStackTopEqualsLightUserData);
		CPPUNIT_TEST(pull_voidPointerPulledFromAnStackContainingLightUserData_pullReturnsTrue);

#if OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(pull_voidPointerPulledFromAnEmptyStack_pullReturnsFalse);
#elif OOLUA_USING_EXCEPTIONS == 1
		CPPUNIT_TEST(pull_voidPointerPulledFromAnEmptyStack_throwsOoluaRuntimeError);
#endif
		CPPUNIT_TEST(pull_voidPointerPulledFromAnStackContainingLightUserData_pullReturnsTrue);
		CPPUNIT_TEST(pull_voidPointerPulledFromAnStackContainingLightUserData_pulledValueEqualsPushed);
#if OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(pull_voidPointerWhenStackContainsFullUserData_pullreturnsFalse);
#elif OOLUA_USING_EXCEPTIONS == 1
		CPPUNIT_TEST(pull_voidPointerWhenStackContainsFullUserData_throwsOoluaRuntimeError);
#endif
		CPPUNIT_TEST(functionReturn_functionReturnsVoidPointer_callReturnsTrue);
		CPPUNIT_TEST(functionReturn_functionReturnsVoidPointer_topOfStackIsLightUserData);
		CPPUNIT_TEST(functionReturn_functionReturnsVoidPointer_stackValueComparesEqualToReturnValue);
		CPPUNIT_TEST(functionReturn_returnsTypeWhichIsNotVoidPointer_compiles);
		CPPUNIT_TEST(functionReturn_functionReturnsTypeWhichIsNotVoidPointer_topOfStackIsLightUserData);

		CPPUNIT_TEST(functionParam_functionWhichTakesVoidPointer_compiles);
		CPPUNIT_TEST(functionParam_functionWhichTakesVoidPointer_functionIsCalledWithTheCorrectValue);
		CPPUNIT_TEST(functionParam_functionWhichTakesNoneVoidPointer_compiles);
		CPPUNIT_TEST(functionParam_functionWhichTakesNoneVoidPointer_functionIsCalledWithTheCorrectValue);

		CPPUNIT_TEST(constructor_takesVoidPointer_compiles);
		CPPUNIT_TEST(constructor_TakesVoidPointer_callReturnsTrue);
		CPPUNIT_TEST(constructor_TakesVoidPointer_returnValueIsInputtedValue);

		CPPUNIT_TEST(constructor_takesNoneVoidPointer_compiles);
		CPPUNIT_TEST(constructor_TakesNoneVoidPointer_callReturnsTrue);
		CPPUNIT_TEST(constructor_TakesNoneVoidPointer_returnValueIsInputtedValue);

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

	/*
	The actions the library will enable with light userdata is as follows:
	* Done - Allow the pushing of a void pointer to the stack
	* Done - Allow the pulling of a void pointer from the stack
	* Done - Allow a void pointer to be passed as a parameter to a function
	* Done - Allow a void pointer to be passed to a constructor
	* Done - Allow a none void pointer to be passed to a constructor
	* Done - Allow a void pointer to be a function return
	* Done - Allow a function return pointer which is not a void pointer to be pushed
		as light userdata using a trait
	* Done - a function parameter which is not a void pointer to pulled from stack
		and cast to requested type.
	*/
	void push_voidPointerPushedToTheStack_topEqualsOne()
	{
		int i;
		void* p = &i;
		m_lua->push(p);
		CPPUNIT_ASSERT_EQUAL(1, m_lua->stack_count());
	}

	void push_voidPointerPushedToTheStack_typeOfStackTopEqualsLightUserData()
	{
		int i;
		void* p = &i;
		m_lua->push(p);
		CPPUNIT_ASSERT_EQUAL(LUA_TLIGHTUSERDATA, lua_type(*m_lua, -1));
	}

	void push_voidPointerPushedToTheStack_pushReturnsTrue()
	{
		int i;
		void* p = &i;
		CPPUNIT_ASSERT_EQUAL(true, m_lua->push(p));
	}

	void push_nullPointerPushedToTheStack_typeOfStackTopEqualsLightUserData()
	{
		void* p = 0;
		m_lua->push(p);
		CPPUNIT_ASSERT_EQUAL(LUA_TLIGHTUSERDATA, lua_type(*m_lua, -1));
	}

#if OOLUA_STORE_LAST_ERROR == 1
	void pull_voidPointerPulledFromAnEmptyStack_pullReturnsFalse()
	{
		void* p;
		CPPUNIT_ASSERT_EQUAL(false, m_lua->pull(p));
	}
#elif OOLUA_USING_EXCEPTIONS == 1
	void pull_voidPointerPulledFromAnEmptyStack_throwsOoluaRuntimeError()
	{
		void* p;
		CPPUNIT_ASSERT_THROW(m_lua->pull(p), OOLUA::Runtime_error);
	}

#endif

	void pull_voidPointerPulledFromAnStackContainingLightUserData_pullReturnsTrue()
	{
		int i = 0;
		void* p = &i;
		m_lua->push(p);
		CPPUNIT_ASSERT_EQUAL(true, m_lua->pull(p));
	}

	void pull_voidPointerPulledFromAnStackContainingLightUserData_pulledValueEqualsPushed()
	{
		int i = 0;
		void* pushed_ud = &i;
		void* pulled_ud(0);
		m_lua->push(pushed_ud);
		m_lua->pull(pulled_ud);
		CPPUNIT_ASSERT_EQUAL(pushed_ud, pulled_ud);
	}

#if OOLUA_STORE_LAST_ERROR == 1
	void pull_voidPointerWhenStackContainsFullUserData_pullreturnsFalse()
	{
		m_lua->register_class<Stub1>();
		Stub1 full_ud;
		m_lua->push(&full_ud);
		void* light_ud;
		CPPUNIT_ASSERT_EQUAL(false, m_lua->pull(light_ud));
	}
#elif OOLUA_USING_EXCEPTIONS == 1
	void pull_voidPointerWhenStackContainsFullUserData_throwsOoluaRuntimeError()
	{
		m_lua->register_class<Stub1>();
		Stub1 full_ud;
		m_lua->push(&full_ud);
		void* light_ud;
		CPPUNIT_ASSERT_THROW(m_lua->pull(light_ud), OOLUA::Runtime_error);
	}
#endif

	void functionReturn_functionReturnsVoidPointer_compiles()
	{
		::testing::NiceMock<LightUserDataFunctionReturnTraitsMock> mock;
		m_lua->register_class<LightUserDataFunctionReturnTraits>();
	}

	void functionReturn_functionReturnsVoidPointer_callReturnsTrue()
	{
		LightUserDataFunctionReturnTraitsMock mock;
		LightUserDataFunctionReturnTraits* object = &mock;

		int i(0);
		void* lightud = &i;

		EXPECT_CALL(mock, value()).Times(1).WillOnce(::testing::Return(lightud));
		m_lua->register_class<LightUserDataFunctionReturnTraits>();
		m_lua->run_chunk("return function(object) return object:value() end");

		CPPUNIT_ASSERT_EQUAL(true, m_lua->call(1, object));
	}

	void functionReturn_functionReturnsVoidPointer_topOfStackIsLightUserData()
	{
		LightUserDataFunctionReturnTraitsMock mock;
		LightUserDataFunctionReturnTraits* object = &mock;

		int i(0);
		void* lightud = &i;

		EXPECT_CALL(mock, value()).Times(1).WillOnce(::testing::Return(lightud));
		m_lua->register_class<LightUserDataFunctionReturnTraits>();
		m_lua->run_chunk("return function(object) return object:value() end");
		m_lua->call(1, object);
		CPPUNIT_ASSERT_EQUAL(LUA_TLIGHTUSERDATA, lua_type(*m_lua, -1));
	}

	void functionReturn_functionReturnsVoidPointer_stackValueComparesEqualToReturnValue()
	{
		LightUserDataFunctionReturnTraitsMock mock;
		LightUserDataFunctionReturnTraits* object = &mock;

		int i(0);
		void* lightud = &i;

		EXPECT_CALL(mock, value()).Times(1).WillOnce(::testing::Return(lightud));
		m_lua->register_class<LightUserDataFunctionReturnTraits>();
		m_lua->run_chunk("return function(object) return object:value() end");
		m_lua->call(1, object);
		void* returned_ud;
		m_lua->pull(returned_ud);
		CPPUNIT_ASSERT_EQUAL(lightud, returned_ud);
	}

	void functionReturn_returnsTypeWhichIsNotVoidPointer_compiles()
	{
		::testing::NiceMock<CastLightUserDataFunctionReturnTraitsMock> mock;
	}

	void functionReturn_functionReturnsTypeWhichIsNotVoidPointer_topOfStackIsLightUserData()
	{
		CastLightUserDataFunctionReturnTraitsMock mock;
		CastLightUserDataFunctionReturnTraits* object = &mock;

		InvalidStub lightud;;

		EXPECT_CALL(mock, ptr()).Times(1).WillOnce(::testing::Return(&lightud));
		m_lua->register_class<CastLightUserDataFunctionReturnTraits>();
		m_lua->run_chunk("return function(object) return object:ptr() end");
		m_lua->call(1, object);
		CPPUNIT_ASSERT_EQUAL(LUA_TLIGHTUSERDATA, lua_type(*m_lua, -1));
	}
//	LightUserDataFunctionReturnTraits
//	LightUserDataFunctionReturnTraitsMock
//	CastLightUserDataFunctionReturnTraits
//	CastLightUserDataFunctionReturnTraitsMock

	void functionParam_functionWhichTakesVoidPointer_compiles()
	{
		::testing::NiceMock<LightParamUserDataMock> mock;
	}

	/**[TestLightUserdataParam]*/
	void functionParam_functionWhichTakesVoidPointer_functionIsCalledWithTheCorrectValue()
	{
		LightParamUserDataMock mock;
		LightParamUserData* object = &mock;
		m_lua->register_class<LightParamUserData>();
		int i(0);
		void* input_ud = &i;
		EXPECT_CALL(mock, value(::testing::Eq(input_ud))).Times(1);
		m_lua->run_chunk("return function(object,param) return object:value(param) end");
		m_lua->call(1, object, input_ud);
	}
	/**[TestLightUserdataParam]*/

	void functionParam_functionWhichTakesNoneVoidPointer_compiles()
	{
		::testing::NiceMock<LightNoneVoidParamUserDataMock> mock;
	}

	/**[TestLightUserdataNonVoidParam]*/
	void functionParam_functionWhichTakesNoneVoidPointer_functionIsCalledWithTheCorrectValue()
	{
		LightNoneVoidParamUserDataMock mock;
		LightNoneVoidParamUserData* object = &mock;
		m_lua->register_class<LightNoneVoidParamUserData>();
		InvalidStub lightud;
		void* lightud_ptr = &lightud;
		EXPECT_CALL(mock, ptr(::testing::Eq(lightud_ptr))).Times(1);
		m_lua->run_chunk("return function(object,param) return object:ptr(param) end");
		m_lua->call(1, object, lightud_ptr);
	}
	/**[TestLightUserdataNonVoidParam]*/

	void constructor_takesVoidPointer_compiles()
	{
		m_lua->register_class<LightVoidUserDataConstructor>();
	}

	void constructor_TakesVoidPointer_callReturnsTrue()
	{
		m_lua->register_class<LightVoidUserDataConstructor>();
		InvalidStub lightud;
		void* lightud_ptr = &lightud;
		m_lua->run_chunk("return function(param) LightVoidUserDataConstructor.new(param) end");
		CPPUNIT_ASSERT_EQUAL(true, m_lua->call(1, lightud_ptr));
	}

	void constructor_TakesVoidPointer_returnValueIsInputtedValue()
	{
		m_lua->register_class<LightVoidUserDataConstructor>();
		InvalidStub lightud;
		void* lightud_ptr = &lightud;
		m_lua->run_chunk("return function(param) return LightVoidUserDataConstructor.new(param):get_ptr() end");
		m_lua->call(1, lightud_ptr);
		void* result_ptr(0);
		m_lua->pull(result_ptr);
		CPPUNIT_ASSERT_EQUAL(lightud_ptr, result_ptr);
	}

	void constructor_takesNoneVoidPointer_compiles()
	{
		m_lua->register_class<LightNoneVoidUserDataConstructor>();
	}

	void constructor_TakesNoneVoidPointer_callReturnsTrue()
	{
		m_lua->register_class<LightNoneVoidUserDataConstructor>();
		InvalidStub lightud;
		void* lightud_ptr = &lightud;
		m_lua->run_chunk("return function(param) LightNoneVoidUserDataConstructor.new(param) end");
		CPPUNIT_ASSERT_EQUAL(true, m_lua->call(1, lightud_ptr));
	}

	void constructor_TakesNoneVoidPointer_returnValueIsInputtedValue()
	{
		m_lua->register_class<LightNoneVoidUserDataConstructor>();
		InvalidStub lightud;
		void* lightud_ptr = &lightud;
		m_lua->run_chunk("return function(param) return LightNoneVoidUserDataConstructor.new(param):get_ptr() end");
		m_lua->call(1, lightud_ptr);
		void* result_ptr(0);
		m_lua->pull(result_ptr);
		CPPUNIT_ASSERT_EQUAL(lightud_ptr, result_ptr);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Light_ud_beta);
