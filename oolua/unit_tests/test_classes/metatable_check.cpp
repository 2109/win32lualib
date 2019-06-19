#	include "oolua_tests_pch.h"
#	include "oolua.h"

#	include "common_cppunit_headers.h"

struct Int_wrapper{int i;};

int equal(lua_State* vm)
{
	Int_wrapper* i1 = static_cast<Int_wrapper *>(lua_touserdata(vm, 1));
	Int_wrapper* i2 = static_cast<Int_wrapper *>(lua_touserdata(vm, 2));
	lua_pushboolean(vm, i1->i == i2->i ? 1 : 0);
	return 1;
}

class MetatableEqual : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(MetatableEqual);
	CPPUNIT_TEST(equal_luaTablesWithDifferentMetatables_assertsTrue);
	CPPUNIT_TEST(equal_userdataWithDifferentMetatables_returnsTrue);
	CPPUNIT_TEST_SUITE_END();

	OOLUA::Script * m_lua;
public:
	MetatableEqual()
		: m_lua(0)
	{}
	LVD_NOCOPY(MetatableEqual)
	void setUp()
	{
		m_lua = new OOLUA::Script;
	}
	void tearDown()
	{
		delete m_lua;
	}

	void equal_luaTablesWithDifferentMetatables_assertsTrue()
	{
		std::string chunk(" mt1 = {} "
							"mt2 = {} "
							"equal = function (lhs, rhs) "
								"return lhs.i == rhs.i "
							"end "
						"mt1[\"__eq\"] = equal "
						"mt2[\"__eq\"] = equal "
						"t1 = {} "
						"setmetatable(t1, mt1) "
						"t2 = {} "
						"setmetatable(t2, mt2) "
						"t1.i = 1 "
						"t2.i = 1 "
						"assert(t1 == t2)");
		m_lua->run_chunk(chunk);
	}

	//When setting closures in Lua, metamethods are inserted in both const and none
	//constant metatables. For some operators to work the same closure needs to be set
	//as there is a raw equal called(does not call meta method __index).
	//Pushing the same function pointer to each table causes a fail, instead the closure
	//should be pushed onto the stack and its index re-pushed when it is required.
	void equal_userdataWithDifferentMetatables_returnsTrue()
	{
		lua_State* vm = luaL_newstate();
		std::string chunk("equal = function (lhs, rhs) return lhs == rhs end");
		luaL_loadbuffer(vm, chunk.c_str(), chunk.size(), "userChunk");
		lua_pcall(vm, 0, LUA_MULTRET, 0);
		lua_getglobal(vm, "equal");

		luaL_newmetatable(vm, "mt1");
		int mt1 = lua_gettop(vm);
		luaL_newmetatable(vm, "mt2");
		int mt2 = lua_gettop(vm);

		lua_pushcfunction(vm, &equal);
		int func = lua_gettop(vm);
		lua_pushliteral(vm, "__eq");
		int metaname = lua_gettop(vm);

		lua_pushvalue(vm, metaname);
		lua_pushvalue(vm, func);
		lua_settable(vm, mt1);

		lua_pushvalue(vm, metaname);
		lua_pushvalue(vm, func);
		lua_settable(vm, mt2);

		lua_remove(vm, metaname);
		lua_remove(vm, func);

		Int_wrapper* wrapper1 = static_cast<Int_wrapper*>(lua_newuserdata(vm, sizeof(Int_wrapper)));
		int w1 = lua_gettop(vm);
		lua_pushvalue(vm, mt1);
		lua_setmetatable(vm, w1);

		Int_wrapper* wrapper2 = static_cast<Int_wrapper*>(lua_newuserdata(vm, sizeof(Int_wrapper)));
		int w2 = lua_gettop(vm);
		lua_pushvalue(vm, mt2);
		lua_setmetatable(vm, w2);

		wrapper1->i = wrapper2->i = 1;
		lua_remove(vm, mt2);
		lua_remove(vm, mt1);

		lua_pcall(vm, 2, LUA_MULTRET, 0);
		bool result = lua_toboolean(vm, -1) == 1 ? true : false;
		CPPUNIT_ASSERT_EQUAL(true, result);
		lua_close(vm);
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(MetatableEqual);
