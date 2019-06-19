#	include "oolua.h"
#	include "common_cppunit_headers.h"
#	include "gmock/gmock.h"
#	include "expose_stub_classes.h"
namespace
{
	enum anonymousNamespacedEnum{anonymousNamespacedEnumValue = 1};
}

struct IsThisAStringClass{};
struct IsThisAStringClassItHasNoneConstCStrMethod
{
	char const* c_str(){return 0;}
};

class isIntegralStringClass : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(isIntegralStringClass);
		CPPUNIT_TEST(int_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(bool_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(float_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(double_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(void_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(anonymousNamespacedEnum_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(hasProxy_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(luaRef_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(luaTable_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(luaStatePtr_isIntegralStringClass_resultIsZero);

		CPPUNIT_TEST(classWithNoneCstrMethod_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(classWithNoneConstCstrMethod_isIntegralStringClass_resultIsZero);

		CPPUNIT_TEST(stdWstring_isIntegralStringClass_resultIsZero);
		CPPUNIT_TEST(stdString_isIntegralStringClass_resultIsTrue);

	CPPUNIT_TEST_SUITE_END();
public:
	void int_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<int>::value));
	}
	void bool_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<bool>::value));
	}
	void float_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<float>::value));
	}
	void double_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<double>::value));
	}
	void void_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<void>::value));
	}
	void hasProxy_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<Stub1>::value));
	}

	void anonymousNamespacedEnum_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<anonymousNamespacedEnum>::value));
	}
	void luaRef_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<OOLUA::Lua_func_ref>::value));
	}
	void luaTable_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<OOLUA::Table>::value));
	}
	void luaStatePtr_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<lua_State*>::value));
	}
	void classWithNoneCstrMethod_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<IsThisAStringClass>::value));
	}
	void classWithNoneConstCstrMethod_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<IsThisAStringClassItHasNoneConstCStrMethod>::value));
	}

	void stdWstring_isIntegralStringClass_resultIsZero()
	{
		CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(OOLUA::STRING::is_integral_string_class<std::wstring>::value));
	}

	void stdString_isIntegralStringClass_resultIsTrue()
	{
		CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(OOLUA::STRING::is_integral_string_class<std::string>::value));
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(isIntegralStringClass);
