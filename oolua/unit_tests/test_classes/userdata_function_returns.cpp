#	include "userdata_function_returns.h"
#	include "expose_stub_classes.h"
#	include "expose_userdata_function_returns.h"

class UserDataFunctionReturns : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(UserDataFunctionReturns);
		CPPUNIT_TEST(functionReturn_returnsRefToInstance_gcFlagIsFalse);
		CPPUNIT_TEST(functionReturn_returnsRefToInstance_resultComparesEqualToInstance);

		CPPUNIT_TEST(functionReturn_returnsRefToConstantInstance_gcFlagIsFalse);
		CPPUNIT_TEST(functionReturn_returnsRefToConstantInstance_resultComparesEqualToInstance);

		CPPUNIT_TEST(functionReturn_returnsPtrToInstance_gcFlagIsFalse);
		CPPUNIT_TEST(functionReturn_returnsPtrToInstance_resultComparesEqualToInstance);

		CPPUNIT_TEST(functionReturn_returnsPtrToConstantInstance_constFlagIsTrue);
		CPPUNIT_TEST(functionReturn_returnsPtrToConstantInstance_gcFlagIsFalse);
		CPPUNIT_TEST(functionReturn_returnsPtrToConstantInstance_resultComparesEqualToInstance);

		CPPUNIT_TEST(functionReturn_returnsRefPtrToConstantInstance_gcFlagIsFalse);
		CPPUNIT_TEST(functionReturn_returnsRefPtrToConstantInstance_resultComparesEqualToInstance);

		CPPUNIT_TEST(functionReturn_returnsRefConstPtrToConstantInstance_gcFlagIsFalse);
		CPPUNIT_TEST(functionReturn_returnsRefConstPtrToConstantInstance_resultComparesEqualToInstance);

		CPPUNIT_TEST(functionReturn_returnsByValue_instanceIsToBeGarbageCollected);

		CPPUNIT_TEST(luaReturnTrait_callsMethodPtr_returnValueIsToBeGarbageCollected);
		CPPUNIT_TEST(luaReturnTrait_callsMethodPtrConst_returnValueIsToBeGarbageCollected);
		CPPUNIT_TEST(luaReturnTrait_callsMethodPtrConst_returnValueIsConst);
		CPPUNIT_TEST(luaReturnTrait_callsMethodRefPtrConst_returnValueIsToBeGarbageCollected);
		CPPUNIT_TEST(luaReturnTrait_callsMethodRefPtrConst_returnValueIsConst);
		CPPUNIT_TEST(luaReturnTrait_callsMethodRefConstPtrConst_returnValueIsToBeGarbageCollected);
		CPPUNIT_TEST(luaReturnTrait_callsMethodRefConstPtrConst_returnValueIsConst);

		CPPUNIT_TEST(maybeNullTrait_callsMethodPtrWhichReturnsNull_stackSizeIncreasesByOne);
		CPPUNIT_TEST(maybeNullTrait_callsMethodPtrWhichReturnsNull_stackTopIsNil);
		CPPUNIT_TEST(maybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackSizeIncreasesByOne);
		CPPUNIT_TEST(maybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopIsUserData);
		CPPUNIT_TEST(maybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopGcValueIsFalse);
		CPPUNIT_TEST(maybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackSizeIncreasesByOne);
		CPPUNIT_TEST(maybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackTopIsNil);
		CPPUNIT_TEST(maybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackSizeIncreasesByOne);
		CPPUNIT_TEST(maybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackTopIsUserData);

		CPPUNIT_TEST(maybeNullTrait_callsCFunctionWhichReturnsNull_stackSizeIncreasesByOne);
		CPPUNIT_TEST(maybeNullTrait_callsCFunctionWhichReturnsNull_stackTopIsNil);
		CPPUNIT_TEST(maybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackSizeIncreasesByOne);
		CPPUNIT_TEST(maybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackTopIsUserdata);

		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodPtrWhichReturnsNull_stackSizeIncreasesByOne);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodPtrWhichReturnsNull_stackTopIsNil);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackSizeIncreasesByOne);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopIsUserData);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopGcValueIsTrue);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackSizeIncreasesByOne);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackTopIsNil);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackSizeIncreasesByOne);
		CPPUNIT_TEST(luaMaybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackTopIsUserData);

		CPPUNIT_TEST(luaMaybeNullTrait_callsCFunctionWhichReturnsNull_stackSizeIncreasesByOne);
		CPPUNIT_TEST(luaMaybeNullTrait_callsCFunctionWhichReturnsNull_stackTopIsNil);
		CPPUNIT_TEST(luaMaybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackSizeIncreasesByOne);
		CPPUNIT_TEST(luaMaybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackTopIsUserdata);
		CPPUNIT_TEST(luaMaybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackTopGcValueIsTrue);

	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
	struct DefaultTraitHelper
	{
		DefaultTraitHelper(OOLUA::Script* lua)
			: mock()
			, object(&mock)
			, return_stub()
			, m_lua(lua)
		{
			m_lua->register_class<UserDataFunctionDefaultReturnTraits>();
			m_lua->register_class<Stub1>();
		}

		bool call_object_method(std::string const& method_name)
		{
			bool res = m_lua->run_chunk("return function(object, method) return object[method](object) end");
			if(!res)return res;
			return m_lua->call(-1, object, method_name);
		}

		UserDataFunctionDefaultReturnTraitsMock mock;
		UserDataFunctionDefaultReturnTraits* object;
		Stub1 return_stub;
		OOLUA::Script* m_lua;
	};

	struct ReturnTraitHelper
	{
		ReturnTraitHelper(OOLUA::Script* lua)
			: mock()
			, object(&mock)
			, return_stub()
			, m_lua(lua)
		{
			m_lua->register_class<UserDataFunctionReturnTraits>();
			m_lua->register_class<Stub1>();
		}

		bool call_object_method(std::string const& method_name)
		{
			bool res = m_lua->run_chunk("return function(object, method) return object[method](object) end");
			if(!res)return res;
			return m_lua->call(-1, object, method_name);
		}

		UserDataFunctionReturnTraitsMock mock;
		UserDataFunctionReturnTraits* object;
		Stub1 return_stub;
		OOLUA::Script* m_lua;
	};

	struct MaybeNullTraitHelper
	{
		MaybeNullTraitHelper(OOLUA::Script* lua)
			: mock()
			, object(&mock)
			, return_stub()
			, m_lua(lua)
		{
			m_lua->register_class<UserDataFunctionReturnMaybeNullTraits>();
			m_lua->register_class<Stub1>();
		}

		bool call_object_method(std::string const& method_name)
		{
			bool res = m_lua->run_chunk("return function(object, method) return object[method](object) end");
			if(!res)return res;
			OOLUA::Lua_func_ref func; m_lua->pull(func);
			return m_lua->call(func, object, method_name);
		}

		UserDataFunctionReturnMaybeNullTraitsMock mock;
		UserDataFunctionReturnMaybeNullTraits* object;
		Stub1 return_stub;
		OOLUA::Script* m_lua;
	};

	struct LuaMaybeNullTraitHelper
	{
		LuaMaybeNullTraitHelper(OOLUA::Script* lua)
			: mock()
			, object(&mock)
			, return_stub()
			, m_lua(lua)
		{
			m_lua->register_class<UserDataFunctionReturnLuaMaybeNullTraits>();
			m_lua->register_class<Stub1>();
		}

		bool call_object_method(std::string const& method_name)
		{
			bool res = m_lua->run_chunk("return function(object, method) return object[method](object) end");
			if(!res)return res;
			OOLUA::Lua_func_ref func; m_lua->pull(func);
			return m_lua->call(func, object, method_name);
		}

		UserDataFunctionReturnLuaMaybeNullTraitsMock mock;
		UserDataFunctionReturnLuaMaybeNullTraits* object;
		Stub1 return_stub;
		OOLUA::Script* m_lua;
	};

public:
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	void assert_that_tops_gc_flag_is(bool flag)
	{
		OOLUA::INTERNAL::Lua_ud* ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(flag, OOLUA::INTERNAL::userdata_is_to_be_gced(ud));
	}
	void assert_that_tops_const_flag_is(bool flag)
	{
		OOLUA::INTERNAL::Lua_ud* ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(flag, OOLUA::INTERNAL::userdata_is_constant(ud));
	}

	void set_tops_gc_flag_to(bool flag)
	{
		OOLUA::INTERNAL::userdata_gc_value(static_cast<OOLUA::INTERNAL::Lua_ud*>(lua_touserdata(*m_lua, -1)), flag);
	}

	void functionReturn_returnsRefToInstance_gcFlagIsFalse()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ref()).Times(1).WillOnce(::testing::ReturnRef(helper.return_stub));
		helper.call_object_method("ref");
		assert_that_tops_gc_flag_is(false);
	};

	void functionReturn_returnsRefToInstance_resultComparesEqualToInstance()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ref()).Times(1).WillOnce(::testing::ReturnRef(helper.return_stub));
		helper.call_object_method("ref");
		assert_return_equals_input<Stub1*>(*m_lua, &helper.return_stub);
	};

	void functionReturn_returnsRefToConstantInstance_gcFlagIsFalse()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refConst()).Times(1).WillOnce(::testing::ReturnRef(helper.return_stub));
		helper.call_object_method("refConst");

		assert_that_tops_gc_flag_is(false);
	};

	void functionReturn_returnsRefToConstantInstance_resultComparesEqualToInstance()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, refConst()).Times(1).WillOnce(::testing::ReturnRef(helper.return_stub));
		helper.call_object_method("refConst");
		assert_return_equals_input<Stub1 const*>(*m_lua, &helper.return_stub);
	};

	void functionReturn_returnsPtrToInstance_gcFlagIsFalse()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return<Stub1*>(&helper.return_stub));
		helper.call_object_method("ptr");
		assert_that_tops_gc_flag_is(false);
	};

	void functionReturn_returnsPtrToInstance_resultComparesEqualToInstance()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return<Stub1*>(&helper.return_stub));
		helper.call_object_method("ptr");
		assert_return_equals_input<Stub1*>(*m_lua, &helper.return_stub);
	};


	void functionReturn_returnsPtrToConstantInstance_gcFlagIsFalse()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptrConst()).Times(1).WillOnce(::testing::Return<Stub1 const*>(&helper.return_stub));
		helper.call_object_method("ptrConst");
		assert_that_tops_gc_flag_is(false);
	};

	void functionReturn_returnsPtrToConstantInstance_constFlagIsTrue()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptrConst()).Times(1).WillOnce(::testing::Return<Stub1 const*>(&helper.return_stub));
		helper.call_object_method("ptrConst");
		assert_that_tops_const_flag_is(true);
	};

	void functionReturn_returnsPtrToConstantInstance_resultComparesEqualToInstance()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptrConst()).Times(1).WillOnce(::testing::Return<Stub1 const*>(&helper.return_stub));
		helper.call_object_method("ptrConst");
		assert_return_equals_input<Stub1 const*>(*m_lua, &helper.return_stub);
	};

	void functionReturn_returnsRefPtrToConstantInstance_gcFlagIsFalse()
	{
		DefaultTraitHelper helper(m_lua);
		Stub1 const* stub = &helper.return_stub;
		EXPECT_CALL(helper.mock, refPtrConst()).Times(1).WillOnce(::testing::ReturnRef(stub));
		helper.call_object_method("refPtrConst");
		assert_that_tops_gc_flag_is(false);
	};

	void functionReturn_returnsRefPtrToConstantInstance_resultComparesEqualToInstance()
	{
		DefaultTraitHelper helper(m_lua);
		Stub1 const* stub = &helper.return_stub;
		EXPECT_CALL(helper.mock, refPtrConst()).Times(1).WillOnce(::testing::ReturnRef(stub));
		helper.call_object_method("refPtrConst");
		assert_return_equals_input<Stub1 const *>(*m_lua, &helper.return_stub);
	};

	void functionReturn_returnsRefConstPtrToConstantInstance_gcFlagIsFalse()
	{
		DefaultTraitHelper helper(m_lua);
		Stub1 const*const stub = &helper.return_stub;
		EXPECT_CALL(helper.mock, refConstPtrConst()).Times(1).WillOnce(::testing::ReturnRef(stub));
		helper.call_object_method("refConstPtrConst");
		assert_that_tops_gc_flag_is(false);
	};

	void functionReturn_returnsRefConstPtrToConstantInstance_resultComparesEqualToInstance()
	{
		DefaultTraitHelper helper(m_lua);
		Stub1 const*const stub = &helper.return_stub;
		EXPECT_CALL(helper.mock, refConstPtrConst()).Times(1).WillOnce(::testing::ReturnRef(stub));
		helper.call_object_method("refConstPtrConst");
		assert_return_equals_input<Stub1 const *>(*m_lua, &helper.return_stub);
	};

	void functionReturn_returnsByValue_instanceIsToBeGarbageCollected()
	{
		DefaultTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, value()).Times(1).WillOnce(::testing::Return(helper.return_stub));
		helper.call_object_method("value");
		assert_that_tops_gc_flag_is(true);
	}

	/**[LuaReturnTraitPtrUsage]*/
	void luaReturnTrait_callsMethodPtr_returnValueIsToBeGarbageCollected()
	{
		ReturnTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("ptr");
		assert_that_tops_gc_flag_is(true);
		set_tops_gc_flag_to(false);
	}
	/**[LuaReturnTraitPtrUsage]*/

	void luaReturnTrait_callsMethodPtrConst_returnValueIsToBeGarbageCollected()
	{
		ReturnTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptrConst()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("ptrConst");
		assert_that_tops_gc_flag_is(true);
		set_tops_gc_flag_to(false);
	}

	void luaReturnTrait_callsMethodPtrConst_returnValueIsConst()
	{
		ReturnTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptrConst()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("ptrConst");
		assert_that_tops_const_flag_is(true);
		set_tops_gc_flag_to(false);
	}

	void luaReturnTrait_callsMethodRefPtrConst_returnValueIsToBeGarbageCollected()
	{
		ReturnTraitHelper helper(m_lua);
		Stub1 const* return_value = &helper.return_stub;
		EXPECT_CALL(helper.mock, refPtrConst()).Times(1).WillOnce(::testing::ReturnRef(return_value));
		helper.call_object_method("refPtrConst");
		assert_that_tops_gc_flag_is(true);
		set_tops_gc_flag_to(false);
	}

	void luaReturnTrait_callsMethodRefPtrConst_returnValueIsConst()
	{
		ReturnTraitHelper helper(m_lua);
		Stub1 const* return_value = &helper.return_stub;
		EXPECT_CALL(helper.mock, refPtrConst()).Times(1).WillOnce(::testing::ReturnRef(return_value));
		helper.call_object_method("refPtrConst");
		assert_that_tops_const_flag_is(true);
		set_tops_gc_flag_to(false);
	}

	void luaReturnTrait_callsMethodRefConstPtrConst_returnValueIsToBeGarbageCollected()
	{
		ReturnTraitHelper helper(m_lua);
		Stub1 const * const return_value = &helper.return_stub;
		EXPECT_CALL(helper.mock, refConstPtrConst()).Times(1).WillOnce(::testing::ReturnRef(return_value));
		helper.call_object_method("refConstPtrConst");
		assert_that_tops_gc_flag_is(true);
		set_tops_gc_flag_to(false);
	}

	void luaReturnTrait_callsMethodRefConstPtrConst_returnValueIsConst()
	{
		ReturnTraitHelper helper(m_lua);
		Stub1 const * const return_value = &helper.return_stub;
		EXPECT_CALL(helper.mock, refConstPtrConst()).Times(1).WillOnce(::testing::ReturnRef(return_value));
		helper.call_object_method("refConstPtrConst");
		assert_that_tops_const_flag_is(true);
		set_tops_gc_flag_to(false);
	}

	void maybeNullTrait_callsMethodPtrWhichReturnsNull_stackSizeIncreasesByOne()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1*>(NULL)));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void maybeNullTrait_callsMethodPtrWhichReturnsNull_stackTopIsNil()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1*>(NULL)));
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}

	void maybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackSizeIncreasesByOne()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void maybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopIsUserData()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(LUA_TUSERDATA, lua_type(*m_lua, -1));
	}

	void maybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopGcValueIsFalse()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("ptr");
		assert_that_tops_gc_flag_is(false);
	}

	void maybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackSizeIncreasesByOne()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1 *const>(NULL)));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	/**[MaybeNullTraitConstPtrUsage]*/
	void maybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackTopIsNil()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1 *const>(NULL)));
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}
	/**[MaybeNullTraitConstPtrUsage]*/

	void maybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackSizeIncreasesByOne()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void maybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackTopIsUserData()
	{
		MaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(LUA_TUSERDATA, lua_type(*m_lua, -1));
	}

	void maybeNullTrait_callsCFunctionWhichReturnsNull_stackSizeIncreasesByOne()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "null_ptr", lua_proxy_returns_null_ptr);
		int stackSizeBefore = lua_gettop(*m_lua);
		m_lua->run_chunk("return null_ptr()");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void maybeNullTrait_callsCFunctionWhichReturnsNull_stackTopIsNil()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "null_ptr", lua_proxy_returns_null_ptr);
		m_lua->run_chunk("return null_ptr()");
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}

	void maybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackSizeIncreasesByOne()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "valid_ptr", lua_proxy_returns_valid_ptr);
		int stackSizeBefore = lua_gettop(*m_lua);
		m_lua->run_chunk("return valid_ptr()");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void maybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackTopIsUserdata()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "valid_ptr", lua_proxy_returns_valid_ptr);
		m_lua->run_chunk("return valid_ptr()");
		CPPUNIT_ASSERT_EQUAL(LUA_TUSERDATA, lua_type(*m_lua, -1));
	}

	void luaMaybeNullTrait_callsMethodPtrWhichReturnsNull_stackSizeIncreasesByOne()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1*>(NULL)));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void luaMaybeNullTrait_callsMethodPtrWhichReturnsNull_stackTopIsNil()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1*>(NULL)));
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}

	void luaMaybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackSizeIncreasesByOne()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
		set_tops_gc_flag_to(false);
	}

	void luaMaybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopIsUserData()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("ptr");
		CPPUNIT_ASSERT_EQUAL(LUA_TUSERDATA, lua_type(*m_lua, -1));
		set_tops_gc_flag_to(false);
	}

	/**[LuaMaybeNullTraitPtrUsage]*/
	void luaMaybeNullTrait_callsMethodPtrWhichReturnsValidPtr_stackTopGcValueIsTrue()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, ptr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("ptr");
		assert_that_tops_gc_flag_is(true);
		set_tops_gc_flag_to(false);
	}
	/**[LuaMaybeNullTraitPtrUsage]*/

	void luaMaybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackSizeIncreasesByOne()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1 *const>(NULL)));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void luaMaybeNullTrait_callsMethodConstPtrWhichReturnsNull_stackTopIsNil()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(static_cast<Stub1 *const>(NULL)));
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}

	void luaMaybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackSizeIncreasesByOne()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		int stackSizeBefore = lua_gettop(*m_lua);
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
		set_tops_gc_flag_to(false);
	}

	void luaMaybeNullTrait_callsMethodConstPtrWhichReturnsValidPtr_stackTopIsUserData()
	{
		LuaMaybeNullTraitHelper helper(m_lua);
		EXPECT_CALL(helper.mock, constPtr()).Times(1).WillOnce(::testing::Return(&helper.return_stub));
		helper.call_object_method("constPtr");
		CPPUNIT_ASSERT_EQUAL(LUA_TUSERDATA, lua_type(*m_lua, -1));
		set_tops_gc_flag_to(false);
	}

	void luaMaybeNullTrait_callsCFunctionWhichReturnsNull_stackSizeIncreasesByOne()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "null_ptr", lua_proxy_returns_lua_null_ptr);
		int stackSizeBefore = lua_gettop(*m_lua);
		m_lua->run_chunk("return null_ptr()");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
	}

	void luaMaybeNullTrait_callsCFunctionWhichReturnsNull_stackTopIsNil()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "null_ptr", lua_proxy_returns_lua_null_ptr);
		m_lua->run_chunk("return null_ptr()");
		CPPUNIT_ASSERT_EQUAL(LUA_TNIL, lua_type(*m_lua, -1));
	}

	void luaMaybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackSizeIncreasesByOne()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "valid_ptr", lua_proxy_returns_lua_valid_ptr);
		int stackSizeBefore = lua_gettop(*m_lua);
		m_lua->run_chunk("return valid_ptr()");
		CPPUNIT_ASSERT_EQUAL(stackSizeBefore+1, lua_gettop(*m_lua));
		set_tops_gc_flag_to(false);
	}

	void luaMaybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackTopIsUserdata()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "valid_ptr", lua_proxy_returns_lua_valid_ptr);
		m_lua->run_chunk("return valid_ptr()");
		CPPUNIT_ASSERT_EQUAL(LUA_TUSERDATA, lua_type(*m_lua, -1));
		set_tops_gc_flag_to(false);
	}

	void luaMaybeNullTrait_callsCFunctionWhichReturnsValidPtr_stackTopGcValueIsTrue()
	{
		m_lua->register_class<Stub1>();
		OOLUA::set_global(*m_lua, "valid_ptr", lua_proxy_returns_lua_valid_ptr);
		m_lua->run_chunk("return valid_ptr()");
		assert_that_tops_gc_flag_is(true);
		set_tops_gc_flag_to(false);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(UserDataFunctionReturns);
