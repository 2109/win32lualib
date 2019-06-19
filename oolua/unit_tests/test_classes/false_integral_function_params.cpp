#	include "oolua_tests_pch.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include "oolua.h"
#	include "expose_false_integral_function_params.h"


bool idx_equal(lua_State* vm, int idx1, int idx2)
{
#if LUA_VERSION_NUM < 502
	return !!lua_equal(vm, idx1, idx2);
#else
	return !!lua_compare(vm, idx1, idx2, LUA_OPEQ);
#endif
}

/*Visual Studio C4100 unreferenced formal parameter result_listener. GoogleMock*/
#ifdef _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4100 )
#endif
MATCHER_P(RefIsEqualToValue, value, "")
{
	OOLUA::push(value.state(), arg);
	OOLUA::push(value.state(), value);
	return idx_equal(value.state(), -1, -2);
}

MATCHER_P(RefIsInvalid, dummyValue, "")
{
	return arg.valid() == false;
}
#ifdef _MSC_VER
#	pragma warning(pop)
#endif

class FalseIntegralFunctionParams : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(FalseIntegralFunctionParams);
		CPPUNIT_TEST(inTraitFuncRef_passesInstance_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inTraitFuncRef_passesReference_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inTraitFuncRef_passesReferenceToConstant_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inOutTraitFuncRef_functionAssignsValueToParam_resultComparesEqualWithSeed);
		CPPUNIT_TEST(outTraitFuncRef_functionAssignsValueToParam_resultComparesEqualWithSeed);

		CPPUNIT_TEST(inTraitTableRef_passesInstance_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inTraitTableRef_passesReference_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inTraitTableRef_passesReferenceToConstant_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inOutTraitTableRef_functionAssignsValueToParam_resultComparesEqualWithSeed);
		CPPUNIT_TEST(outTraitTableRef_functionAssignsValueToParam_resultComparesEqualWithSeed);

		CPPUNIT_TEST(inTraitTableObject_passesInstance_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inTraitTableObject_passesReference_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inTraitTableObject_passesReferenceToConstant_parameterComparesEqualToSeed);
		CPPUNIT_TEST(inOutTraitTableObject_functionAssignsValueToParam_resultComparesEqualWithSeed);
		CPPUNIT_TEST(outTraitTableObject_functionAssignsValueToParam_resultComparesEqualWithSeed);

		CPPUNIT_TEST(inTraitFuncRef_nilPassed_calledWithInvalidFuncReference);
		CPPUNIT_TEST(inTraitTableRef_nilPassed_calledWithInvalidTableReference);
		CPPUNIT_TEST(inTraitTableObject_nilPassed_calledWithInvalidTableObject);
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
	struct FuncRefHelper
	{
		FuncRefHelper(OOLUA::Script& s)
			: mock()
			, object(&mock)
		{
			s.register_class<FunctionRefParam>();
			s.run_chunk("return function()end");
			OOLUA::pull(s, ref);
		}
		void assign_ref(OOLUA::Lua_func_ref& r)
		{
			OOLUA::push(ref.state(), ref);
			OOLUA::pull(ref.state(), r);
		}
		void assert_top_is_equal_to_ref()
		{
			OOLUA::push(ref.state(), ref);
			CPPUNIT_ASSERT_EQUAL(true, idx_equal(ref.state(), -1, -2));
		}

		FunctionRefParamMock mock;
		FunctionRefParam* object;
		OOLUA::Lua_func_ref ref;
	};

	void inTraitFuncRef_passesInstance_parameterComparesEqualToSeed()
	{
		FuncRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTrait(param) end");
		EXPECT_CALL(f.mock, value(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inTraitFuncRef_passesReference_parameterComparesEqualToSeed()
	{
		FuncRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTraitRef(param) end");
		EXPECT_CALL(f.mock, ref(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inTraitFuncRef_passesReferenceToConstant_parameterComparesEqualToSeed()
	{
		FuncRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTraitRefConst(param) end");
		EXPECT_CALL(f.mock, refConst(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inOutTraitFuncRef_functionAssignsValueToParam_resultComparesEqualWithSeed()
	{
		FuncRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:inOutTraitRef(function()end) end");
		EXPECT_CALL(f.mock, ref(::testing::_)).Times(1).WillOnce(::testing::Invoke(&f, &FuncRefHelper::assign_ref));
		m_lua->call(1, f.object);
		f.assert_top_is_equal_to_ref();
	}

	void outTraitFuncRef_functionAssignsValueToParam_resultComparesEqualWithSeed()
	{
		FuncRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:outTraitRef() end");
		EXPECT_CALL(f.mock, ref(::testing::_)).Times(1).WillOnce(::testing::Invoke(&f, &FuncRefHelper::assign_ref));
		m_lua->call(1, f.object);
		f.assert_top_is_equal_to_ref();
	}


	struct TableRefHelper
	{
		TableRefHelper(OOLUA::Script& s)
			: mock()
			, object(&mock)
		{
			s.register_class<TableRefParam>();
			lua_newtable(s);
			OOLUA::pull(s, ref);
		}
		void assign_ref(OOLUA::Lua_table_ref& r)
		{
			OOLUA::push(ref.state(), ref);
			OOLUA::pull(ref.state(), r);
		}
		void assert_top_is_equal_to_ref()
		{
			OOLUA::push(ref.state(), ref);
			CPPUNIT_ASSERT_EQUAL(true, idx_equal(ref.state(), -1, -2));
		}
		TableRefParamMock mock;
		TableRefParam* object;
		OOLUA::Lua_table_ref ref;
	};

	void inTraitTableRef_passesInstance_parameterComparesEqualToSeed()
	{
		TableRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTrait(param) end");
		EXPECT_CALL(f.mock, value(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inTraitTableRef_passesReference_parameterComparesEqualToSeed()
	{
		TableRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTraitRef(param) end");
		EXPECT_CALL(f.mock, ref(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inTraitTableRef_passesReferenceToConstant_parameterComparesEqualToSeed()
	{
		TableRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTraitRefConst(param) end ");
		EXPECT_CALL(f.mock, refConst(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inOutTraitTableRef_functionAssignsValueToParam_resultComparesEqualWithSeed()
	{
		TableRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:inOutTraitRef({}) end");
		EXPECT_CALL(f.mock, ref(::testing::_)).Times(1).WillOnce(::testing::Invoke(&f, &TableRefHelper::assign_ref));
		m_lua->call(1, f.object);

		f.assert_top_is_equal_to_ref();
	}

	void outTraitTableRef_functionAssignsValueToParam_resultComparesEqualWithSeed()
	{
		TableRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:outTraitRef() end");
		EXPECT_CALL(f.mock, ref(::testing::_)).Times(1).WillOnce(::testing::Invoke(&f, &TableRefHelper::assign_ref));
		m_lua->call(1, f.object);

		f.assert_top_is_equal_to_ref();
	}

	struct TableObjectHelper
	{
		TableObjectHelper(OOLUA::Script& s)
			: mock()
			, object(&mock)
		{
			s.register_class<TableObjectParam>();
			lua_newtable(s);
			OOLUA::pull(s, ref);
		}
		void assign_ref(OOLUA::Table& r)
		{
			OOLUA::push(ref.state(), ref);
			OOLUA::pull(ref.state(), r);
		}
		void assert_top_is_equal_to_ref()
		{
			OOLUA::push(ref.state(), ref);
			CPPUNIT_ASSERT_EQUAL(true, idx_equal(ref.state(), -1, -2));
		}
		TableObjectParamMock mock;
		TableObjectParam* object;
		OOLUA::Table ref;
	};

	void inTraitTableObject_passesInstance_parameterComparesEqualToSeed()
	{
		TableObjectHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTrait(param) end");
		EXPECT_CALL(f.mock, value(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inTraitTableObject_passesReference_parameterComparesEqualToSeed()
	{
		TableObjectHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTraitRef(param) end");
		EXPECT_CALL(f.mock, ref(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inTraitTableObject_passesReferenceToConstant_parameterComparesEqualToSeed()
	{
		TableObjectHelper f(*m_lua);
		m_lua->run_chunk("return function(object, param) return object:inTraitRefConst(param) end ");
		EXPECT_CALL(f.mock, refConst(RefIsEqualToValue(f.ref))).Times(1);
		m_lua->call(1, f.object, f.ref);
	}

	void inOutTraitTableObject_functionAssignsValueToParam_resultComparesEqualWithSeed()
	{
		TableObjectHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:inOutTraitRef({}) end");
		EXPECT_CALL(f.mock, ref(::testing::_)).Times(1).WillOnce(::testing::Invoke(&f, &TableObjectHelper::assign_ref));
		m_lua->call(1, f.object);

		f.assert_top_is_equal_to_ref();
	}

	void outTraitTableObject_functionAssignsValueToParam_resultComparesEqualWithSeed()
	{
		TableObjectHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:outTraitRef() end");
		EXPECT_CALL(f.mock, ref(::testing::_)).Times(1).WillOnce(::testing::Invoke(&f, &TableObjectHelper::assign_ref));
		m_lua->call(1, f.object);

		f.assert_top_is_equal_to_ref();
	}

	void inTraitFuncRef_nilPassed_calledWithInvalidFuncReference()
	{
		FuncRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:inTrait(nil) end");
		OOLUA::Lua_func_ref dummyValue;
		EXPECT_CALL(f.mock, value(RefIsInvalid(dummyValue))).Times(1);
		m_lua->call(1, f.object);
	}

	void inTraitTableRef_nilPassed_calledWithInvalidTableReference()
	{
		TableRefHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:inTrait(nil) end");
		OOLUA::Lua_table_ref dummyValue;
		EXPECT_CALL(f.mock, value(RefIsInvalid(dummyValue))).Times(1);
		m_lua->call(1, f.object);
	}

	void inTraitTableObject_nilPassed_calledWithInvalidTableObject()
	{
		TableObjectHelper f(*m_lua);
		m_lua->run_chunk("return function(object) return object:inTrait(nil) end");
		OOLUA::Table dummyValue;
		EXPECT_CALL(f.mock, value(RefIsInvalid(dummyValue))).Times(1);
		m_lua->call(1, f.object);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(FalseIntegralFunctionParams);
