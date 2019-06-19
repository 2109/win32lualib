#include "hierarchy_expose.h"

#ifdef OOLUA_LUABIND_COMPARE
#	include "luabind/luabind.hpp"
#	include "luabind/operator.hpp"
#endif

OOLUA_EXPORT_FUNCTIONS(ProfileBase
								   ,increment_a_base
								   ,virtual_func
								   ,pure_virtual_func)
OOLUA_EXPORT_FUNCTIONS_CONST(ProfileBase)

OOLUA_EXPORT_NO_FUNCTIONS(ProfileAnotherBase)
OOLUA_EXPORT_NO_FUNCTIONS(ProfileDerived)
OOLUA_EXPORT_NO_FUNCTIONS(ProfileMultiBases)

void open_Luabind_hierarchy(lua_State* l)
{
#ifdef OOLUA_LUABIND_COMPARE
	luabind::open(l);
	luabind::module(l)
		[
			luabind::class_<ProfileBase>("ProfileBase")
			.def("increment_a_base",&ProfileBase::increment_a_base)
			.def("virtual_func",&ProfileBase::virtual_func)
			.def("pure_virtual_func",&ProfileBase::pure_virtual_func)
		];
	luabind::module(l)
		[
			luabind::class_<ProfileAnotherBase>("ProfileAnotherBase")
			.def(luabind::constructor<>())
		];
	luabind::module(l)
		[
			luabind::class_<ProfileDerived,ProfileBase>("ProfileDerived")
			.def(luabind::constructor<>())
		];
	luabind::module(l)
		[
			luabind::class_<ProfileMultiBases,luabind::bases<ProfileAnotherBase,ProfileBase> >("ProfileMultiBases")
			.def(luabind::constructor<>())
		];
#else
	(void)l;
#endif
}

#ifdef OOLUA_LUABRIDGE_COMPARE
#	include "LuaBridge.h"
#endif

void open_LuaBridge_hierarchy(lua_State* l)
{
#ifdef OOLUA_LUABRIDGE_COMPARE
	using namespace luabridge;
	getGlobalNamespace (l)
		.beginClass <ProfileBase> ("ProfileBase")
			.addFunction ("increment_a_base", &ProfileBase::increment_a_base)
			.addFunction ("virtual_func", &ProfileBase::virtual_func)
			.addFunction ("pure_virtual_func", &ProfileBase::pure_virtual_func)
		.endClass()
		.beginClass<ProfileAnotherBase>("ProfileAnotherBase")
			.addConstructor<void(*)(void)>()
		.endClass()
		.deriveClass <ProfileDerived, ProfileBase> ("ProfileDerived")
			.addConstructor<void(*)(void)>()
		.endClass()
	//	.beginClass<ProfileAnotherBase>("ProfileMultiBases")
		.deriveClass<ProfileMultiBases,ProfileDerived> ("ProfileMultiBases")
	//	luabind::class_<ProfileMultiBases,luabind::bases<ProfileAnotherBase,ProfileBase> >("ProfileMultiBases")
			.addConstructor<void(*)(void)>()
		.endClass()
	;
#else
	(void)l;
#endif
}


#if defined OOLUA_SLB_COMPARE && LUA_VERSION_NUM == 502
#	include <SLB3/implementation.h>
SLB3_IMPLEMENTATION(ProfileBase, C) 
{
	C.set("increment_a_base",&ProfileBase::increment_a_base);
	C.set("virtual_func", &ProfileBase::virtual_func);
	C.set("pure_virtual_func", &ProfileBase::pure_virtual_func);
}
SLB3_IMPLEMENTATION(ProfileAnotherBase, C) 
{
	C.constructor();
}
SLB3_IMPLEMENTATION(ProfileDerived, C) 
{
	C.constructor();
	C.inherits<ProfileBase>();
}
SLB3_IMPLEMENTATION(ProfileMultiBases, C) 
{
	C.constructor();
	C.inherits<ProfileBase>();
	C.inherits<ProfileDerived>();
	C.inherits<ProfileAnotherBase>();
}

#endif
