
#	include "oolua_tests_pch.h"
#	include "common_cppunit_headers.h"
#	include "oolua.h"
#	include "expose_false_integral_function_returns.h"
#	include "gmock/gmock.h"

namespace
{
	bool idx_equal(lua_State* vm, int idx1, int idx2)
	{
#if LUA_VERSION_NUM < 502
		return !!lua_equal(vm, idx1, idx2);
#else
		return !!lua_compare(vm, idx1, idx2, LUA_OPEQ);
#endif
	}
} //namespace

class FalseIntegralFunctionReturns : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(FalseIntegralFunctionReturns);
		CPPUNIT_TEST(funcRef_returnsPlain_topOfStackIsSeed);
		CPPUNIT_TEST(funcRef_returnsRef_topOfStackIsSeed);
		CPPUNIT_TEST(funcRef_returnsRefConst_topOfStackIsSeed);
		CPPUNIT_TEST(tableRef_returnsPlain_topOfStackIsSeed);
		CPPUNIT_TEST(tableRef_returnsRef_topOfStackIsSeed);
		CPPUNIT_TEST(tableRef_returnsRefConst_topOfStackIsSeed);
		CPPUNIT_TEST(tableObject_returnsPlain_topOfStackIsSeed);
		CPPUNIT_TEST(tableObject_returnsRef_topOfStackIsSeed);
		CPPUNIT_TEST(tableObject_returnsRefConst_topOfStackIsSeed);
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

	struct FunRefHelper
	{
		FunRefHelper(OOLUA::Script& s)
		{
			s.register_class<FunctionRefReturn>();
			s.run_chunk("return function()end");
			OOLUA::pull(s, ref);
		}
		void assert_top_is_equal_with_ref()
		{
			OOLUA::push(ref.state(), ref);
			CPPUNIT_ASSERT_EQUAL(true, idx_equal(ref.state(), -1, -2));
		}
		OOLUA::Lua_func_ref ref;
		FunctionRefReturnMock object;
	};

	void funcRef_returnsPlain_topOfStackIsSeed()
	{
		FunRefHelper f(*m_lua);
		EXPECT_CALL(f.object, value()).Times(1).WillOnce(::testing::Return(f.ref));
		m_lua->run_chunk("return function(object) return object:value() end");
		m_lua->call(1, static_cast<FunctionRefReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}

	void funcRef_returnsRef_topOfStackIsSeed()
	{
		FunRefHelper f(*m_lua);
		EXPECT_CALL(f.object, ref()).Times(1).WillOnce(::testing::ReturnRef(f.ref));
		m_lua->run_chunk("return function(object) return object:ref() end");
		m_lua->call(1, static_cast<FunctionRefReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}

	void funcRef_returnsRefConst_topOfStackIsSeed()
	{
		FunRefHelper f(*m_lua);
		EXPECT_CALL(f.object, refConst()).Times(1).WillOnce(::testing::ReturnRef(f.ref));
		m_lua->run_chunk("return function(object) return object:refConst() end");
		m_lua->call(1, static_cast<FunctionRefReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}


	struct TableRefHelper
	{
		TableRefHelper(OOLUA::Script& s)
		{
			s.register_class<TableRefReturn>();
			lua_newtable(s);
			OOLUA::pull(s, ref);
		}
		void assert_top_is_equal_with_ref()
		{
			OOLUA::push(ref.state(), ref);
			CPPUNIT_ASSERT_EQUAL(true, idx_equal(ref.state(), -1, -2));
		}
		OOLUA::Lua_table_ref ref;
		TableRefReturnMock object;
	};

	void tableRef_returnsPlain_topOfStackIsSeed()
	{
		TableRefHelper f(*m_lua);
		EXPECT_CALL(f.object, value()).Times(1).WillOnce(::testing::Return(f.ref));
		m_lua->run_chunk("return function(object) return object:value() end");
		m_lua->call(1, static_cast<TableRefReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}

	void tableRef_returnsRef_topOfStackIsSeed()
	{
		TableRefHelper f(*m_lua);
		EXPECT_CALL(f.object, ref()).Times(1).WillOnce(::testing::ReturnRef(f.ref));
		m_lua->run_chunk("return function(object) return object:ref() end");
		m_lua->call(1, static_cast<TableRefReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}

	void tableRef_returnsRefConst_topOfStackIsSeed()
	{
		TableRefHelper f(*m_lua);
		EXPECT_CALL(f.object, refConst()).Times(1).WillOnce(::testing::ReturnRef(f.ref));
		m_lua->run_chunk("return function(object) return object:refConst() end");
		m_lua->call(1, static_cast<TableRefReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}


	struct TableObjectHelper
	{
		TableObjectHelper(OOLUA::Script& s)
		{
			s.register_class<TableObjectReturn>();
			lua_newtable(s);
			OOLUA::pull(s, ref);
		}
		void assert_top_is_equal_with_ref()
		{
			OOLUA::push(ref.state(), ref);
			CPPUNIT_ASSERT_EQUAL(true, idx_equal(ref.state(), -1, -2));
		}
		OOLUA::Table ref;
		TableObjectReturnMock object;
	};

	void tableObject_returnsPlain_topOfStackIsSeed()
	{
		TableObjectHelper f(*m_lua);
		EXPECT_CALL(f.object, value()).Times(1).WillOnce(::testing::Return(f.ref));
		m_lua->run_chunk("return function(object) return object:value() end");
		m_lua->call(1, static_cast<TableObjectReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}

	void tableObject_returnsRef_topOfStackIsSeed()
	{
		TableObjectHelper f(*m_lua);
		EXPECT_CALL(f.object, ref()).Times(1).WillOnce(::testing::ReturnRef(f.ref));
		m_lua->run_chunk("return function(object) return object:ref() end");
		m_lua->call(1, static_cast<TableObjectReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}

	void tableObject_returnsRefConst_topOfStackIsSeed()
	{
		TableObjectHelper f(*m_lua);
		EXPECT_CALL(f.object, refConst()).Times(1).WillOnce(::testing::ReturnRef(f.ref));
		m_lua->run_chunk("return function(object) return object:refConst() end");
		m_lua->call(1, static_cast<TableObjectReturn*>(&f.object));

		f.assert_top_is_equal_with_ref();
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(FalseIntegralFunctionReturns);
