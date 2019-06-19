
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include "expose_const_func.h"

namespace
{
	struct ConstantMockHelper
	{
		ConstantMockHelper()
			: mock()
			, ptr_to_const(&mock)
			, ptr_to(&mock)
		{}
		ConstantMock mock;
		Constant const* ptr_to_const;
		Constant * ptr_to;
	};
} // namespace

/*
 Tests that the object, on which are calls are requested, handles the following correctly :
 call none constant function on a none constant object
 call a constant function on a none constant object
 call a constant function on a constant object
error:
 call a none constant function on a constant object

 Tested for C++ proxied functions either called directly in Lua or indirectly via functions
 added to the type in Lua, which in turn will call the C++ functions.

 */

class Constant_functions : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Constant_functions);
		CPPUNIT_TEST(callConstantFunction_passedConstantInstance_calledOnce);
		CPPUNIT_TEST(callConstantFunction_passedNoneConstantInstance_calledOnce);
		CPPUNIT_TEST(callNoneConstantFunction_passedNoneConstantInstance_calledOnce);

		CPPUNIT_TEST(callLuaAddedFunctionWhichCallsConstMember_passedConstantInstance_calledOnce);
		CPPUNIT_TEST(callLuaAddedFunctionWhichCallsConstMember_passedNoneConstantInstance_calledOnce);
		CPPUNIT_TEST(callLuaAddedFunctionWhichCallsNoneConstMember_passedNoneConstantInstance_calledOnce);

#if OOLUA_STORE_LAST_ERROR == 1
		CPPUNIT_TEST(callNoneConstantFunction_passedConstantInstance_callReturnsFalse);
		CPPUNIT_TEST(callLuaAddedFunctionWhichCallsNoneConstMember_passedConstantInstance_callReturnsFalse);
		CPPUNIT_TEST(callConstantMethodInBaseClass_usesConstantInstance_callReturnsTrue);
#endif

#if OOLUA_USE_EXCEPTIONS == 1
		CPPUNIT_TEST(callNoneConstantFunction_passedConstantInstance_throwsRuntimeError);
		CPPUNIT_TEST(callLuaAddedFunctionWhichCallsNoneConstMember_passedConstantInstance_callThrowsRuntimeError);
		CPPUNIT_TEST(callConstantMethodInBaseClass_usesConstantInstance_noException);
#endif
	CPPUNIT_TEST_SUITE_END();
	OOLUA::Script * m_lua;
public:
	Constant_functions()
		: m_lua(0)
	{}
	LVD_NOCOPY(Constant_functions)
	void setUp()
	{
		m_lua = new OOLUA::Script;
		m_lua->register_class<Constant>();
	}
	void tearDown()
	{
		delete m_lua;
	}

	void callConstantFunction_passedConstantInstance_calledOnce()
	{
		ConstantMockHelper helper;
		EXPECT_CALL(helper.mock, cpp_func_const()).Times(1);
		m_lua->run_chunk("return function(obj) obj:cpp_func_const() end");
		m_lua->call(1, helper.ptr_to_const);
	}
	void callConstantFunction_passedNoneConstantInstance_calledOnce()
	{
		ConstantMockHelper helper;
		EXPECT_CALL(helper.mock, cpp_func_const()).Times(1);
		m_lua->run_chunk("return function(obj) obj:cpp_func_const() end");
		m_lua->call(1, helper.ptr_to);
	}

	void callNoneConstantFunction_passedNoneConstantInstance_calledOnce()
	{
		ConstantMockHelper helper;
		EXPECT_CALL(helper.mock, cpp_func()).Times(1);
		m_lua->run_chunk("return function(obj) obj:cpp_func() end");
		m_lua->call(1, helper.ptr_to);
	}

	void callLuaAddedFunctionWhichCallsConstMember_passedConstantInstance_calledOnce()
	{
		ConstantMockHelper helper;
		EXPECT_CALL(helper.mock, cpp_func_const()).Times(1);
		m_lua->run_chunk("function Constant:lua_const_func() self:cpp_func_const() end "
					 "return function(object) object:lua_const_func() end ");
		m_lua->call(1, helper.ptr_to_const);
	}
	void callLuaAddedFunctionWhichCallsConstMember_passedNoneConstantInstance_calledOnce()
	{
		ConstantMockHelper helper;
		EXPECT_CALL(helper.mock, cpp_func_const()).Times(1);
		m_lua->run_chunk("function Constant:lua_const_func() self:cpp_func_const() end "
						 "return function(object) object:lua_const_func() end ");
		m_lua->call(1, helper.ptr_to_const);
	}
	void callLuaAddedFunctionWhichCallsNoneConstMember_passedNoneConstantInstance_calledOnce()
	{
		ConstantMockHelper helper;
		EXPECT_CALL(helper.mock, cpp_func()).Times(1);
		m_lua->run_chunk("function Constant:lua_func() self:cpp_func() end "
						 "return function(object) object:lua_func() end ");
		m_lua->call(1, helper.ptr_to);
	}



#if OOLUA_STORE_LAST_ERROR == 1
	void callNoneConstantFunction_passedConstantInstance_callReturnsFalse()
	{
		ConstantMockHelper helper;
		m_lua->run_chunk("return function(obj) obj:cpp_func() end");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.ptr_to_const));
	}

	void callLuaAddedFunctionWhichCallsNoneConstMember_passedConstantInstance_callReturnsFalse()
	{
		ConstantMockHelper helper;
		m_lua->run_chunk("function Constant:lua_func() self:cpp_func() end "
						 "return function(object) object:lua_func() end ");
		CPPUNIT_ASSERT_EQUAL(false, m_lua->call(1, helper.ptr_to_const));
	}
	void callConstantMethodInBaseClass_usesConstantInstance_callReturnsTrue()
	{
		DerivesToUseConstMethod instance;
		DerivesToUseConstMethod const* const_instance = &instance;
		OOLUA::register_class<DerivesToUseConstMethod>(*m_lua);
		m_lua->run_chunk("return function(obj) obj:cpp_func_const() end");
		CPPUNIT_ASSERT_EQUAL(true, m_lua->call(1, const_instance));
	}

#endif

#if OOLUA_USE_EXCEPTIONS == 1
	void callNoneConstantFunction_passedConstantInstance_throwsRuntimeError()
	{
		ConstantMockHelper helper;
		m_lua->run_chunk("return function(obj) obj:cpp_func() end");
		CPPUNIT_ASSERT_THROW((m_lua->call(1, helper.ptr_to_const)), OOLUA::Runtime_error);
	}

	void callLuaAddedFunctionWhichCallsNoneConstMember_passedConstantInstance_callThrowsRuntimeError()
	{
		ConstantMockHelper helper;
		m_lua->run_chunk("function Constant:lua_func() self:cpp_func() end "
						 "return function(object) object:lua_func() end ");
		CPPUNIT_ASSERT_THROW((m_lua->call(1, helper.ptr_to_const)), OOLUA::Runtime_error);
	}

	void callConstantMethodInBaseClass_usesConstantInstance_noException()
	{
		DerivesToUseConstMethod instance;
		DerivesToUseConstMethod const* const_instance = &instance;
		OOLUA::register_class<DerivesToUseConstMethod>(*m_lua);
		m_lua->run_chunk("return function(obj) obj:cpp_func_const() end");
		CPPUNIT_ASSERT_NO_THROW(m_lua->call(1, const_instance));
	}
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(Constant_functions);
