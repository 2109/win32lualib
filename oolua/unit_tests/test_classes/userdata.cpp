#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"

namespace
{
	class A
	{
	public:
		int a(){return 1;}
	};
	class B
	{
	public:
		int b(){return 2;}
	};
} // namespace


OOLUA_PROXY(A)
	OOLUA_MEM_FUNC(int, a)
OOLUA_PROXY_END

OOLUA_EXPORT_FUNCTIONS(A, a)
OOLUA_EXPORT_FUNCTIONS_CONST(A)

OOLUA_PROXY(B)
	OOLUA_MEM_FUNC(int, b)
OOLUA_PROXY_END

OOLUA_EXPORT_FUNCTIONS(B, b)
OOLUA_EXPORT_FUNCTIONS_CONST(B)

class UserData : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(UserData);
		CPPUNIT_TEST(cppMethodCall_passTwoInstancesTwice_noException);
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
public:
	UserData()
		: m_lua(0)
	{}
	LVD_NOCOPY(UserData)
	void setUp()
	{
		m_lua = new OOLUA::Script;
		m_lua->register_class<A>();
		m_lua->register_class<B>();
	}
	void tearDown()
	{
		delete m_lua;
	}

	void cppMethodCall_passTwoInstancesTwice_noException()
	{
		m_lua->run_chunk(\
				"foo = function(o1, o2, o3, o4)\n"
					"o3:a()\n"
					"o4:b()\n"
				"end");
		A a;
		B b;
		CPPUNIT_ASSERT_NO_THROW(m_lua->call("foo", &a, &b, &a, &b));
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(UserData);
