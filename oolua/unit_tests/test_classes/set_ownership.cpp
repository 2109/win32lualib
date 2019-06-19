#	include "oolua_tests_pch.h"
#	include "common_cppunit_headers.h"
#	include "oolua.h"

#	include "cpp_private_destructor.h"
#	include "expose_stub_classes.h"
#	include "expose_ownership.h"


class Ownership : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Ownership);
		CPPUNIT_TEST(setOwnerIsRegistered_onConstantPointer_callReturnsTrue);
		CPPUNIT_TEST(setOwnerIsRegistered_onNoneConstantPointer_callReturnsTrue);
		CPPUNIT_TEST(setOwner_luaPassesOwnershipToCpp_udGcBoolIsFalse);
		CPPUNIT_TEST(setOwner_luaTakesOwnership_udGcBoolIsTrue);
		CPPUNIT_TEST(setOwner_luaTakesOwnershipAndThenPassesItToCpp_udGcBoolIsFalse);

#if OOLUA_STORE_LAST_ERROR	== 1
		CPPUNIT_TEST(setOwner_luaOwnsOnInstanceWithNoPublicDestructor_callSetOwnerReturnsFalse);
		CPPUNIT_TEST(setOwner_cppOwnsOnInstanceWithNoPublicDestructor_callSetOwnerReturnsFalse);
#endif


#if OOLUA_USE_EXCEPTIONS == 1
		CPPUNIT_TEST(setOwner_luaOwnsOnInstanceWithNoPublicDestructor_throwsRuntimeError);
		CPPUNIT_TEST(setOwner_cppOwnsOnInstanceWithNoPublicDestructor_throwsRuntimeError);
		CPPUNIT_TEST(setOwner_functionCalledWithNoneClassType_throwsRuntimeError);
		CPPUNIT_TEST(setOwner_functionCalledWithClassTypeYetNoEnumType_throwsRuntimeError);
#endif

		CPPUNIT_TEST(luaParamOutP_ref2Ptr_userDataPtrComparesEqualToValueSetInFunction);
		CPPUNIT_TEST(luaParamOutP_ref2Ptr_topOfStackGcIsTrue);

		CPPUNIT_TEST(luaParamOutP_ref2PtrConst_userDataPtrComparesEqualToValueSetInFunction);
		CPPUNIT_TEST(luaParamOutP_ref2PtrConst_topOfStackGcIsTrue);

		CPPUNIT_TEST(callFunction_passingPointerUsingLuaAcquirePtr_topOfStackGcIsTrue);
		CPPUNIT_TEST(callFunction_passingPointerUsingLuaAcquirePtr_topOfComparesEqualToStackPointer);

	/*
	 OOLUA::cpp_in_p<Stub1*======
	 OOLUA::cpp_in_p<Stub1*&======

	 OOLUA::cpp_in_p<Stub1 *const
	 OOLUA::cpp_in_p<Stub1 *const&=======

	 OOLUA::cpp_in_p<Stub1 const*
	 OOLUA::cpp_in_p<Stub1 const*&=========

	 OOLUA::cpp_in_p<Stub1 const* const
	 OOLUA::cpp_in_p<Stub1 const* const&
	 */

		CPPUNIT_TEST(cppInP_ptr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);

		CPPUNIT_TEST(cppInP_ref2Ptr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);

		CPPUNIT_TEST(cppInP_constPtr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);
		CPPUNIT_TEST(cppInP_ref2ConstPtr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);

		CPPUNIT_TEST(cppInP_ptr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);
		CPPUNIT_TEST(cppInP_ref2Ptr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);

		CPPUNIT_TEST(cppInP_constPtr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);
		CPPUNIT_TEST(cppInP_ref2ConstPtr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse);

	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
	template<typename Type>
	bool call_set_owner(Type* object, std::string const& owner)
	{
		bool result = m_lua->run_chunk("return function(obj, owner) obj:set_owner(_G[owner]) end");
		return result && m_lua->call(1, object, owner);
	}

public:
	Ownership()
		: m_lua(0)
	{}
	LVD_NOCOPY(Ownership)
	void setUp()
	{
		m_lua = new OOLUA::Script;
		m_lua->register_class<Stub1>();
	}
	void tearDown()
	{
		delete m_lua;
	}

	void setOwnerIsRegistered_onConstantPointer_callReturnsTrue()
	{
		Stub1 stub;
		Stub1 const* s(&stub);
		CPPUNIT_ASSERT_EQUAL(true, call_set_owner(s, "Cpp_owns"));
	}

	void setOwnerIsRegistered_onNoneConstantPointer_callReturnsTrue()
	{
		Stub1 stub;
		CPPUNIT_ASSERT_EQUAL(true, call_set_owner(&stub, "Cpp_owns"));
	}

	void setOwner_luaPassesOwnershipToCpp_udGcBoolIsFalse()
	{
		Stub1 stub;
		call_set_owner(&stub, "Cpp_owns");
		OOLUA::INTERNAL::is_there_an_entry_for_this_void_pointer(*m_lua, &stub);
		OOLUA::INTERNAL::Lua_ud* ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::userdata_is_to_be_gced(ud));
	}

	void setOwner_luaTakesOwnership_udGcBoolIsTrue()
	{
		Stub1 stub;
		call_set_owner(&stub, "Lua_owns");
		OOLUA::INTERNAL::is_there_an_entry_for_this_void_pointer(*m_lua, &stub);
		OOLUA::INTERNAL::Lua_ud* ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::userdata_is_to_be_gced(ud));
		//we have to change back to Cpp or else delete will be called on a stack instance.
		OOLUA::INTERNAL::userdata_gc_value(ud, false);
	}

	void setOwner_luaTakesOwnershipAndThenPassesItToCpp_udGcBoolIsFalse()
	{
		Stub1 stub;
		call_set_owner(&stub, "Lua_owns");
		call_set_owner(&stub, "Cpp_owns");
		OOLUA::INTERNAL::is_there_an_entry_for_this_void_pointer(*m_lua, &stub);
		OOLUA::INTERNAL::Lua_ud* ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::userdata_is_to_be_gced(ud));
	}


#if OOLUA_STORE_LAST_ERROR	== 1
	void setOwner_luaOwnsOnInstanceWithNoPublicDestructor_callSetOwnerReturnsFalse()
	{
		m_lua->register_class<PrivateDestructor>();
		PrivateDestructor* mock = PrivateDestructor::create();
		bool result = call_set_owner(mock, "Lua_owns");
		CPPUNIT_ASSERT_EQUAL(false, result);
		mock->release();
	}

	void setOwner_cppOwnsOnInstanceWithNoPublicDestructor_callSetOwnerReturnsFalse()
	{
		m_lua->register_class<PrivateDestructor>();
		PrivateDestructor* mock = PrivateDestructor::create();
		bool result = call_set_owner(mock, "Cpp_owns");
		CPPUNIT_ASSERT_EQUAL(false, result);
		mock->release();
	}
#endif


#if OOLUA_USE_EXCEPTIONS == 1
	void setOwner_luaOwnsOnInstanceWithNoPublicDestructor_throwsRuntimeError()
	{
		m_lua->register_class<PrivateDestructor>();
		PrivateDestructor* mock = PrivateDestructor::create();
		CPPUNIT_ASSERT_THROW(call_set_owner(mock, "Lua_owns"), OOLUA::Runtime_error);
		mock->release();
	}

	void setOwner_cppOwnsOnInstanceWithNoPublicDestructor_throwsRuntimeError()
	{
		m_lua->register_class<PrivateDestructor>();
		PrivateDestructor* mock = PrivateDestructor::create();
		CPPUNIT_ASSERT_THROW(call_set_owner(mock, "Cpp_owns"), OOLUA::Runtime_error);
		mock->release();
	}

	void setOwner_functionCalledWithNoneClassType_throwsRuntimeError()
	{
		m_lua->register_class<Stub1>();
		CPPUNIT_ASSERT_THROW(m_lua->run_chunk("Stub1.set_owner(1, Cpp_owns)"), OOLUA::Runtime_error);
	}

	void setOwner_functionCalledWithClassTypeYetNoEnumType_throwsRuntimeError()
	{
		m_lua->register_class<Stub1>();
		CPPUNIT_ASSERT_THROW(m_lua->run_chunk("Stub1.set_owner(Stub1.new(), 'foo')"), OOLUA::Runtime_error);
	}
#endif


	OOLUA::INTERNAL::Lua_ud * get_ud_helper()
	{
		return static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
	}

	void luaParamOutP_ref2Ptr_userDataPtrComparesEqualToValueSetInFunction()
	{
		OwnershipParamUserDataMock mock;
		OwnershipParamUserData* object = &mock;
		Stub1 return_stub;

		EXPECT_CALL(mock, refPtr(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(&return_stub));
		m_lua->register_class<OwnershipParamUserData>();
		m_lua->run_chunk("return function(object) return object:lua_takes_ownership_of_ref_2_ptr() end");
		m_lua->call(1, object);

		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		OOLUA::INTERNAL::userdata_gc_value(ud, false);//stop delete being called on this stack pointer
		CPPUNIT_ASSERT_EQUAL(static_cast<void*>(&return_stub), ud->void_class_ptr);
	}


	void luaParamOutP_ref2Ptr_topOfStackGcIsTrue_old()
	{
		OwnershipParamUserDataMock mock;
		OwnershipParamUserData* object = &mock;
		Stub1 return_stub;

		EXPECT_CALL(mock, refPtr(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(&return_stub));
		m_lua->register_class<OwnershipParamUserData>();
		m_lua->run_chunk("return function(object) return object:lua_takes_ownership_of_ref_2_ptr() end");
		m_lua->call(1, object);

		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		bool gc_value = OOLUA::INTERNAL::userdata_is_to_be_gced(ud);
		OOLUA::INTERNAL::userdata_gc_value(ud, false);//stop delete being called on this stack pointer
		CPPUNIT_ASSERT_EQUAL(true, gc_value);
	}

	void luaParamOutP_ref2Ptr_topOfStackGcIsTrue()
	{
		OwnershipParamUserDataMock mock;
		OwnershipParamUserData* object = &mock;
		Stub1 return_stub;

		EXPECT_CALL(mock, refPtr(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(&return_stub));
		/**[TestLuaOutTrait]*/
		m_lua->register_class<OwnershipParamUserData>();
		m_lua->run_chunk("return function(object) return object:lua_takes_ownership_of_ref_2_ptr() end");
		m_lua->call(1, object);
		//there is now a proxy type on top of the stack which Lua owns
		/**[TestLuaOutTrait]*/
		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		bool gc_value = OOLUA::INTERNAL::userdata_is_to_be_gced(ud);
		OOLUA::INTERNAL::userdata_gc_value(ud, false);//stop delete being called on this stack pointer
		CPPUNIT_ASSERT_EQUAL(true, gc_value);
	}


	void luaParamOutP_ref2PtrConst_userDataPtrComparesEqualToValueSetInFunction()
	{
		OwnershipParamUserDataMock mock;
		OwnershipParamUserData* object = &mock;
		Stub1 return_stub;

		EXPECT_CALL(mock, refPtrConst(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(&return_stub));
		m_lua->register_class<OwnershipParamUserData>();
		m_lua->run_chunk("return function(object) return object:lua_takes_ownership_of_ref_2_ptr_const() end");
		m_lua->call(1, object);
		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		OOLUA::INTERNAL::userdata_gc_value(ud, false);//stop delete being called on this stack pointer
		CPPUNIT_ASSERT_EQUAL(static_cast<void*>(&return_stub), ud->void_class_ptr);
	}

	void luaParamOutP_ref2PtrConst_topOfStackGcIsTrue()
	{
		OwnershipParamUserDataMock mock;
		OwnershipParamUserData* object = &mock;
		Stub1 return_stub;

		EXPECT_CALL(mock, refPtrConst(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(&return_stub));
		m_lua->register_class<OwnershipParamUserData>();
		m_lua->run_chunk("return function(object) return object:lua_takes_ownership_of_ref_2_ptr_const() end");
		m_lua->call(1, object);
		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		bool gc_value = OOLUA::INTERNAL::userdata_is_to_be_gced(ud);
		OOLUA::INTERNAL::userdata_gc_value(ud, false);//stop delete being called on this stack pointer
		CPPUNIT_ASSERT_EQUAL(true, gc_value);
	}
	/**[ExampleLuaAcquirePtr]*/
	void callFunction_passingPointerUsingLuaAcquirePtr_topOfStackGcIsTrue()
	{
		Stub1 stub;
		m_lua->run_chunk("foo = function(param) return param end");
		m_lua->call("foo", OOLUA::lua_acquire_ptr<Stub1*>(&stub));
		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		bool gc_value = OOLUA::INTERNAL::userdata_is_to_be_gced(ud);
		OOLUA::INTERNAL::userdata_gc_value(ud, false);///stop delete being called on what is really a stack pointer
		CPPUNIT_ASSERT_EQUAL(true, gc_value);
	}
	/**[ExampleLuaAcquirePtr]*/

	void callFunction_passingPointerUsingLuaAcquirePtr_topOfComparesEqualToStackPointer()
	{
		Stub1 stub;
		m_lua->run_chunk("foo = function(param) return param end");

		//improve code coverage using default constructor
		OOLUA::lua_acquire_ptr<Stub1*> ptr;
		ptr.m_ptr = &stub;
		m_lua->call("foo", ptr);
		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		OOLUA::INTERNAL::userdata_gc_value(ud, false);//stop delete being called on this stack pointer
		CPPUNIT_ASSERT_EQUAL(static_cast<void*>(&stub), ud->void_class_ptr);
	}

	bool returnGarbageCollectValueAfterCppTakingOwnership(std::string func_name)
	{
		::testing::NiceMock<OwnershipParamUserDataMock> object;
		Stub1 stub;
		m_lua->register_class<OwnershipParamUserData>();
		m_lua->run_chunk("return function(object, param, name) object[name](object, param) return param end");
		m_lua->call(1
					, static_cast<OwnershipParamUserData*>(&object)
					, OOLUA::lua_acquire_ptr<Stub1*>(&stub)
					, func_name);
		OOLUA::INTERNAL::Lua_ud * ud = get_ud_helper();
		return OOLUA::INTERNAL::userdata_is_to_be_gced(ud);
	}

	/**[TestCppOutTrait]*/
	void cppInP_ptr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						 "cpp_takes_ownership_of_ptr_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}
	/**[TestCppOutTrait]*/

	void cppInP_ref2Ptr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						"cpp_takes_ownership_of_ref_to_ptr_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void cppInP_constPtr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						"cpp_takes_ownership_of_const_ptr_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void cppInP_ref2ConstPtr2UserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						"cpp_takes_ownership_of_ref_2_const_ptr_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void cppInP_ptr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						"cpp_takes_ownership_of_ptr_to_const_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void cppInP_ref2Ptr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						"cpp_takes_ownership_of_ref_to_ptr_to_const_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void cppInP_constPtr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						"cpp_takes_ownership_of_const_ptr_to_const_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void cppInP_ref2ConstPtr2ConstUserDataType_passingPtrThatLuaOwns_topOfStackGcIsFalse()
	{
		bool result = returnGarbageCollectValueAfterCppTakingOwnership(
						"cpp_takes_ownership_of_ref_to_const_ptr_to_const_param");
		CPPUNIT_ASSERT_EQUAL(false, result);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ownership);
