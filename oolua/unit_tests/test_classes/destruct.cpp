
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "expose_stub_classes.h"
#	include "cpp_private_destructor.h"

#	include "common_cppunit_headers.h"

class Destruct : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(Destruct);
		CPPUNIT_TEST(gc_gcCalledAfterPassingToLua_entryForPointerIsFalse);
		CPPUNIT_TEST(register_classWithPrivateDestuctor_compiles);
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
public:
	Destruct()
		: m_lua(0)
	{}
	LVD_NOCOPY(Destruct)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	void gc_gcCalledAfterPassingToLua_entryForPointerIsFalse()
	{
		m_lua->register_class<Stub1>();
		m_lua->run_chunk("func = function(o1) end");
		Stub1 p1;
		m_lua->call("func", &p1);
		m_lua->gc();
		bool result = OOLUA::INTERNAL::is_there_an_entry_for_this_void_pointer(*m_lua, &p1);
		CPPUNIT_ASSERT_EQUAL(false, result);
	}

	void register_classWithPrivateDestuctor_compiles()
	{
		m_lua->register_class<PrivateDestructor>();
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(Destruct);
