
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include "expose_userdata_function_params.h"


class UserDataFunctionOutParams : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(UserDataFunctionOutParams);
		CPPUNIT_TEST(outTraitRef_luaPassesNoParam_topOfStackIsOwnedByLua);
		CPPUNIT_TEST(outTraitRef_luaPassesNoParamCppSetsPublicMember_resultsMemberHasExpectedValue);

		CPPUNIT_TEST(OutTraitPtr_luaPassesNoParam_topOfStackIsOwnedByLua);
		CPPUNIT_TEST(outTraitPtr_luaPassesNoParamCppSetsPublicMember_resultsMemberHasExpectedValue);

		CPPUNIT_TEST(OutTraitRefPtr_luaPassesNoParam_topOfStackIsOwnedByLua);
		CPPUNIT_TEST(outTraitRefPtr_luaPassesNoParamCppSetsPublicMember_resultsMemberHasExpectedValuee);
	CPPUNIT_TEST_SUITE_END();
	OOLUA::Script * m_lua;
public:
	void setUp()
	{
		m_lua = new OOLUA::Script;
		m_lua->register_class<OutParamUserData>();
		m_lua->register_class<HasIntMember>();
	}
	void tearDown()
	{
		delete m_lua;
	}

	struct OutHelper
	{
		OutHelper()
			: mock()
			, object(&mock)
			, expected(2)
		{}
		OutParamUserDataMock mock;
		OutParamUserData* object;
		HasIntMember expected;
	};

	void assert_return_value_has_expected_value(HasIntMember& expected)
	{
		OOLUA::cpp_acquire_ptr<HasIntMember*> result;
		OOLUA::pull(*m_lua, result);
		CPPUNIT_ASSERT_EQUAL(expected.member, result.m_ptr->member);
		delete result.m_ptr;
	}

	void outTraitRef_luaPassesNoParam_topOfStackIsOwnedByLua()
	{
		::testing::NiceMock<OutParamUserDataMock> stub;
		m_lua->run_chunk("return function(obj) return obj:outTraitRef() end");
		m_lua->call(1, static_cast<OutParamUserData*>(&stub));
		OOLUA::INTERNAL::Lua_ud * ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::userdata_is_to_be_gced(ud));
	}

	void outTraitRef_luaPassesNoParamCppSetsPublicMember_resultsMemberHasExpectedValue()
	{
		OutHelper helper;
		EXPECT_CALL(helper.mock, ref(::testing::_)).Times(1).WillOnce(::testing::SetArgReferee<0>(helper.expected));
		m_lua->run_chunk("return function(obj) return obj:outTraitRef() end");
		m_lua->call(1, helper.object);
		assert_return_value_has_expected_value(helper.expected);
	}

	void OutTraitPtr_luaPassesNoParam_topOfStackIsOwnedByLua()
	{
		::testing::NiceMock<OutParamUserDataMock> stub;
		m_lua->run_chunk("return function(obj) return obj:outTraitPtr() end");
		m_lua->call(1, static_cast<OutParamUserData*>(&stub));
		OOLUA::INTERNAL::Lua_ud * ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::userdata_is_to_be_gced(ud));
	}

	void outTraitPtr_luaPassesNoParamCppSetsPublicMember_resultsMemberHasExpectedValue()
	{
		OutHelper helper;
		EXPECT_CALL(helper.mock, ptr(::testing::_)).Times(1).WillOnce(::testing::SetArgumentPointee<0>(helper.expected));
		m_lua->run_chunk("return function(obj) return obj:outTraitPtr() end");
		m_lua->call(1, helper.object);
		assert_return_value_has_expected_value(helper.expected);
	}

	/**[UserDataOutParamUsage]*/
	void OutTraitRefPtr_luaPassesNoParam_topOfStackIsOwnedByLua()
	{
		::testing::NiceMock<OutParamUserDataMock> stub;
		m_lua->run_chunk("return function(obj) return obj:outTraitRefPtr() end");
		m_lua->call(1, static_cast<OutParamUserData*>(&stub));
		OOLUA::INTERNAL::Lua_ud * ud = static_cast<OOLUA::INTERNAL::Lua_ud *>(lua_touserdata(*m_lua, -1));
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::userdata_is_to_be_gced(ud));
	}
	/**[UserDataOutParamUsage]*/

	void outTraitRefPtr_luaPassesNoParamCppSetsPublicMember_resultsMemberHasExpectedValuee()
	{
		OutHelper helper;
		EXPECT_CALL(helper.mock, refPtr(::testing::_)).Times(1).WillOnce(::testing::SetArgumentPointee<0>(helper.expected));
		m_lua->run_chunk("return function(obj) return obj:outTraitRefPtr() end");
		m_lua->call(1, helper.object);
		assert_return_value_has_expected_value(helper.expected);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(UserDataFunctionOutParams);
