
#	include "oolua_tests_pch.h"
#	include "oolua.h"
#	include "common_cppunit_headers.h"

#	include "expose_hierarchy.h"

template<typename Base_type, typename Derived_type>
struct BaseAndDerived
{
	typedef Derived_type Derived;
	typedef Base_type Base;
	BaseAndDerived(OOLUA::Script* l)
		: derived()
		, derived_ptr(&derived)
		, derived_ptr_const(&derived)
		, base_ptr(&derived)
		, base_ptr_const(&derived)
	{
		l->register_class<Base>();
		l->register_class<Derived>();
	}
	BaseAndDerived(BaseAndDerived const&);
	BaseAndDerived& operator = (BaseAndDerived const&);
	Derived derived;
	Derived* derived_ptr;
	Derived const* derived_ptr_const;
	Base * base_ptr;
	Base const* base_ptr_const;
};

typedef BaseAndDerived<TwoAbstractBases, DerivedFromTwoAbstractBasesAndAbstract3 > BaseAndDerivedNoOffsets;
typedef BaseAndDerived<Abstract3, DerivedFromTwoAbstractBasesAndAbstract3 > BaseAndDerivedHaveOffsets;

template<typename Class_type>
struct Class_instance
{
	typedef Class_type Type;
	Class_instance(OOLUA::Script* l)
		: instance()
		, instance_ptr(&instance)
		, instance_ptr_const(&instance)
	{
		l->register_class<Type>();
	}
	Type instance;
	Type* instance_ptr;
	Type const* instance_ptr_const;
};

class ClassExchange : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(ClassExchange);
	CPPUNIT_TEST(BaseAndDerivedPointers_checkThereIsNoOffsetWhichIsRequiredForTests_addressCompareEqual);
	CPPUNIT_TEST(BaseAndDerivedPointers_checkTheyDoHaveOffsetsWhichIsRequiredForTests_addressAreNotTheSame);
	CPPUNIT_TEST(checkMetatable_pushAbstract3Pointer_pointersMetatableEqualsAbstract3Metatable);

	CPPUNIT_TEST(changeOfMetatable_pushBaseAndThenDerived_basePointerChangesToUsingDerivedMetatable);
	CPPUNIT_TEST(changeOfMetatable_pushBaseWithSameAddressAndThenDerived_basePointerChangesToUsingDerivedMetatable);

	CPPUNIT_TEST(const_pushConstAbstract3_userDataIsConst);
	CPPUNIT_TEST(const_vmPushConstAbstract3_userDataIsConst);

	CPPUNIT_TEST(noneConst_pushNoneConstAbstract3_userDataIsNotConst);
	CPPUNIT_TEST(noneConst_vmPushNoneConstAbstract3_userDataIsNotConst);

	CPPUNIT_TEST(metatable_pushConstAbstract3_metaTableIsEqualToAbstract3);

	CPPUNIT_TEST(noChangeOfConstness_pushInstanceThenSameInstanceYetAsConst_firstPushedIsNotConst);
	CPPUNIT_TEST(noChangeOfConstness_pushInstanceThenSameInstanceYetAsConst_secondPushedIsNotConst);

	CPPUNIT_TEST(changeOfConstness_pushConstInstanceThenSameInstanceYetAsNoneConst_firstPushedIsNotConst);
	CPPUNIT_TEST(changeOfConstness_pushConstInstanceThenSameInstanceYetAsNoneConst_secondPushedIsNotConst);


	CPPUNIT_TEST(noChangeOfConstness_pushConstThenConst_firstPushedIsConst);
	CPPUNIT_TEST(noChangeOfConstness_pushConstThenConst_secondPushedIsConst);

	CPPUNIT_TEST(noChangeOfConstness_pushNoneConstThenNoneConst_firstPushedIsNotConst);
	CPPUNIT_TEST(noChangeOfConstness_pushNoneConstThenNoneConst_secondPushedIsNotConst);

	CPPUNIT_TEST(changeOfConstness_pushConstBaseWithSameAddressAndThenNoneConstDerived_baseChangesToNoneConst);
	CPPUNIT_TEST(changeOfMetatable_pushConstBaseWithSameAddressAndThenNoneConstDerived_baseMetatableChangesToDerived);
	CPPUNIT_TEST(noChangeOfConstness_pushConstBaseWithSameAddressAndThenConstDerived_baseIsConst);

	CPPUNIT_TEST(changeOfConstness_pushConstBaseAndThenNoneConstDerived_baseChangesToNoneConst);
	CPPUNIT_TEST(changeOfMetatable_pushConstBaseAndThenNoneConstDerived_baseMetatableChangesToDerived);
	CPPUNIT_TEST(noChangeOfConstness_pushConstBaseThenConstDerived_baseIsConst);

	//This test fails and is a limitation of the library
	//CPPUNIT_TEST( differentRootsOfaTree_twoRootsPassedToLua_luaUdComparesEqual );

	CPPUNIT_TEST_SUITE_END();
	OOLUA::Script * m_lua;
public:
	ClassExchange()
		: m_lua(0)
	{}
	LVD_NOCOPY(ClassExchange)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}
	void assert_metatable_of_type_at_index_is_same_as_name(int index, char const* name)
	{
		lua_getmetatable(*m_lua, index);//ud ... metatable
		lua_getfield(*m_lua, LUA_REGISTRYINDEX, name);//ud ... metatable metatable
		int equal_result(lua_rawequal(*m_lua, -2, -1));
		int rawequal_returns_1_if_equal(1);
		CPPUNIT_ASSERT_EQUAL(rawequal_returns_1_if_equal, equal_result);
	}

	void BaseAndDerivedPointers_checkThereIsNoOffsetWhichIsRequiredForTests_addressCompareEqual()
	{
		BaseAndDerivedNoOffsets no_offsets(m_lua);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("If this test fails please inform me of which compiler you are using "
							, true
							, (void*)no_offsets.base_ptr ==(void*)no_offsets.derived_ptr); /*NOLINT(readability/casting)*/
	}
	void BaseAndDerivedPointers_checkTheyDoHaveOffsetsWhichIsRequiredForTests_addressAreNotTheSame()
	{
		BaseAndDerivedHaveOffsets no_offsets(m_lua);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("If this test fails please inform me of which compiler you are using "
							, false
							, (void*)no_offsets.base_ptr ==(void*)no_offsets.derived_ptr); /*NOLINT(readability/casting)*/
	}
	void checkMetatable_pushAbstract3Pointer_pointersMetatableEqualsAbstract3Metatable()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3* base = &derived;
		m_lua->register_class<Abstract3>();
		OOLUA::push(*m_lua, base);//ud

		assert_metatable_of_type_at_index_is_same_as_name(-1, OOLUA::Proxy_class<Abstract3>::class_name);
	}


	void const_pushConstAbstract3_userDataIsConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 const * base = &derived;
		m_lua->register_class<Abstract3>();
		OOLUA::push(*m_lua, base);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -1));
	}
	void const_vmPushConstAbstract3_userDataIsConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 const * base = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -1));
	}

	void noneConst_pushNoneConstAbstract3_userDataIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		m_lua->register_class<Abstract3>();
		OOLUA::push(*m_lua, base);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -1));
	}

	void noneConst_vmPushNoneConstAbstract3_userDataIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -1));
	}


	/*
	void metatable_pushAbstract3_metaTableIsEqualToAbstract3()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		m_lua->register_class<Abstract3>();
		OOLUA::push(*m_lua,base);

		assert_metatable_of_type_at_index_is_same_as_name(-1,OOLUA::Proxy_class<Abstract3>::class_name);
	}
	 */
	void metatable_pushConstAbstract3_metaTableIsEqualToAbstract3()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 const * base = &derived;
		m_lua->register_class<Abstract3>();
		OOLUA::push(*m_lua, base);

		assert_metatable_of_type_at_index_is_same_as_name(-1, OOLUA::Proxy_class<Abstract3>::class_name);
	}




	void noChangeOfConstness_pushInstanceThenSameInstanceYetAsConst_firstPushedIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		Abstract3 const * base_const = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base);
		m_lua->push(base_const);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 1));
	}

	void noChangeOfConstness_pushInstanceThenSameInstanceYetAsConst_secondPushedIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		Abstract3 const * base_const = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base);
		m_lua->push(base_const);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 2));
	}

	void changeOfConstness_pushConstInstanceThenSameInstanceYetAsNoneConst_firstPushedIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		Abstract3 const * base_const = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base_const);
		m_lua->push(base);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 1));
	}
	void changeOfConstness_pushConstInstanceThenSameInstanceYetAsNoneConst_secondPushedIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		Abstract3 const * base_const = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base_const);
		m_lua->push(base);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 2));
	}



	void noChangeOfConstness_pushConstThenConst_firstPushedIsConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 const * base_const = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base_const);
		m_lua->push(base_const);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 1));
	}

	void noChangeOfConstness_pushConstThenConst_secondPushedIsConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 const * base_const = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base_const);
		m_lua->push(base_const);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 2));
	}

	void noChangeOfConstness_pushNoneConstThenNoneConst_firstPushedIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3  * base = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base);
		m_lua->push(base);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 1));
	}
	void noChangeOfConstness_pushNoneConstThenNoneConst_secondPushedIsNotConst()
	{
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract3 * base = &derived;
		m_lua->register_class<Abstract3>();
		m_lua->push(base);
		m_lua->push(base);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, 2));
	}


	void changeOfConstness_pushConstBaseWithSameAddressAndThenNoneConstDerived_baseChangesToNoneConst()
	{
		BaseAndDerivedNoOffsets no_offsets(m_lua);
		OOLUA::push(*m_lua, no_offsets.base_ptr_const);
		OOLUA::push(*m_lua, no_offsets.derived_ptr);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -2));
	}

	void changeOfMetatable_pushConstBaseWithSameAddressAndThenNoneConstDerived_baseMetatableChangesToDerived()
	{
		BaseAndDerivedNoOffsets no_offsets(m_lua);
		OOLUA::push(*m_lua, no_offsets.base_ptr_const);
		OOLUA::push(*m_lua, no_offsets.derived_ptr);
		assert_metatable_of_type_at_index_is_same_as_name(-2
														  , OOLUA::Proxy_class<BaseAndDerivedNoOffsets::Derived>::class_name);
	}

	void noChangeOfConstness_pushConstBaseWithSameAddressAndThenConstDerived_baseIsConst()
	{
		BaseAndDerivedNoOffsets no_offsets(m_lua);
		OOLUA::push(*m_lua, no_offsets.base_ptr_const);
		OOLUA::push(*m_lua, no_offsets.derived_ptr_const);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -2));
	}

	void changeOfConstness_pushConstBaseAndThenNoneConstDerived_baseChangesToNoneConst()
	{
		BaseAndDerivedHaveOffsets have_offsets(m_lua);
		OOLUA::push(*m_lua, have_offsets.base_ptr_const);
		OOLUA::push(*m_lua, have_offsets.derived_ptr);
		CPPUNIT_ASSERT_EQUAL(false, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -2));
	}

	void changeOfMetatable_pushConstBaseAndThenNoneConstDerived_baseMetatableChangesToDerived()
	{
		BaseAndDerivedHaveOffsets have_offsets(m_lua);
		OOLUA::push(*m_lua, have_offsets.base_ptr_const);
		OOLUA::push(*m_lua, have_offsets.derived_ptr);
		assert_metatable_of_type_at_index_is_same_as_name(-2
														, OOLUA::Proxy_class<BaseAndDerivedHaveOffsets::Derived>::class_name);
	}

	void noChangeOfConstness_pushConstBaseThenConstDerived_baseIsConst()
	{
		BaseAndDerivedHaveOffsets no_offsets(m_lua);
		OOLUA::push(*m_lua, no_offsets.base_ptr_const);
		OOLUA::push(*m_lua, no_offsets.derived_ptr_const);
		CPPUNIT_ASSERT_EQUAL(true, OOLUA::INTERNAL::ud_at_index_is_const(*m_lua, -2));
	}


	void changeOfMetatable_pushBaseAndThenDerived_basePointerChangesToUsingDerivedMetatable()
	{
		BaseAndDerivedHaveOffsets have_offsets(m_lua);
		OOLUA::push(*m_lua, have_offsets.base_ptr);
		OOLUA::push(*m_lua, have_offsets.derived_ptr);

		assert_metatable_of_type_at_index_is_same_as_name(-2
							, OOLUA::Proxy_class<BaseAndDerivedHaveOffsets::Derived>::class_name);
	}

	void changeOfMetatable_pushBaseWithSameAddressAndThenDerived_basePointerChangesToUsingDerivedMetatable()
	{
        BaseAndDerivedNoOffsets no_offsets(m_lua);
		OOLUA::push(*m_lua, no_offsets.base_ptr);
		OOLUA::push(*m_lua, no_offsets.derived_ptr);

		assert_metatable_of_type_at_index_is_same_as_name(-2
			, OOLUA::Proxy_class<BaseAndDerivedNoOffsets::Derived>::class_name);
	}


	void checkMetatable_pushConstDerivedAndThenConstBase_derivedUsesDerivedMetatable()
	{
		BaseAndDerivedHaveOffsets have_offsets(m_lua);
		OOLUA::push(*m_lua, have_offsets.derived_ptr_const);
		OOLUA::push(*m_lua, have_offsets.base_ptr_const);

		assert_metatable_of_type_at_index_is_same_as_name(-2
														  , OOLUA::Proxy_class<BaseAndDerivedHaveOffsets::Derived>::class_name);
	}

	/**
	\addtogroup OOLuaKnownLimitations
	@{
	\section BaseClasses Incorrect creation of userdata
	OOLua incorrectly creates a new userdata when it should reuse one which has already
	been created.
	\see http://code.google.com/p/oolua/issues/detail?id=5

	\code{.cpp}
	void differentRootsOfaTree_twoRootsPassedToLua_luaUdComparesEqual()
	{
		OOLUA::register_class<DerivedFromTwoAbstractBasesAndAbstract3>(*m_lua);
		DerivedFromTwoAbstractBasesAndAbstract3 derived;
		Abstract2* a2 = &derived;
		Abstract3* a3 = &derived;
		OOLUA::push(*m_lua, a2);
		OOLUA::push(*m_lua, a3);
		OOLUA::INTERNAL::Lua_ud* ud_a2 = static_cast<OOLUA::INTERNAL::Lua_ud*>(lua_touserdata(*m_lua,-2));
		OOLUA::INTERNAL::Lua_ud* ud_a3 = static_cast<OOLUA::INTERNAL::Lua_ud*>(lua_touserdata(*m_lua,-1));
		CPPUNIT_ASSERT_EQUAL(true, ud_a2 == ud_a3);
	}
	\endcode
	@}
	*/
};
CPPUNIT_TEST_SUITE_REGISTRATION(ClassExchange);
