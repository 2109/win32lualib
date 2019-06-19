#include "set_n_get_expose.h"


#ifdef OOLUA_LUABIND_COMPARE
#	include "luabind/luabind.hpp"
#	include "luabind/operator.hpp"
#endif

OOLUA_EXPORT_FUNCTIONS(Set_get,set)
OOLUA_EXPORT_FUNCTIONS_CONST(Set_get,get)

void open_Luabind_set_n_get(lua_State *l)
{
#ifdef OOLUA_LUABIND_COMPARE
	luabind::open(l);
	luabind::module(l)
		[
			luabind::class_<Set_get>("Set_get")
			.def(luabind::constructor<>())
			.def("set",&Set_get::set)
			.def("get",&Set_get::get)
		];
#else
	(void)l;
#endif
}


#ifdef OOLUA_LUABRIDGE_COMPARE
#	include "LuaBridge.h"
#endif

void open_LuaBridge_set_n_get(lua_State* l)
{
#ifdef OOLUA_LUABRIDGE_COMPARE
	typedef void (*default_constructor) (void);
	using namespace luabridge;
	getGlobalNamespace(l)
		.beginClass <Set_get>("Set_get")
			.addConstructor<void(*)(void)>()
			.addFunction("set",&Set_get::set)
			.addFunction("get",&Set_get::get)
		.endClass ();
#else
	(void)l;
#endif
}


#if defined OOLUA_SLB_COMPARE && LUA_VERSION_NUM == 502
#	include <SLB3/implementation.h>
	SLB3_IMPLEMENTATION(Set_get, C) 
	{
		C.constructor();
		C.set("set",&Set_get::set);
		C.set("get",&Set_get::get);
	}
#endif

