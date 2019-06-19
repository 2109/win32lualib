#ifndef HIERARCHY_OOLUA_H_
#	define HIERARCHY_OOLUA_H_

#include "hierarchy.h"
#include "oolua.h"

OOLUA_PROXY(ProfileBase)
	OOLUA_TAGS( Abstract )
	OOLUA_MFUNC(increment_a_base)
	OOLUA_MFUNC(virtual_func)
	OOLUA_MFUNC(pure_virtual_func)
OOLUA_PROXY_END

OOLUA_PROXY(ProfileAnotherBase)
OOLUA_PROXY_END

OOLUA_PROXY(ProfileDerived,ProfileBase)
OOLUA_PROXY_END

OOLUA_PROXY(ProfileMultiBases,ProfileDerived,ProfileAnotherBase)
OOLUA_PROXY_END


#if defined OOLUA_SLB_COMPARE && LUA_VERSION_NUM == 502
#	include <SLB3/declaration.h>
	SLB3_DECLARATION(ProfileBase, "ProfileBase", policy::Default)
	SLB3_DECLARATION(ProfileAnotherBase, "ProfileAnotherBase", policy::Default)
	SLB3_DECLARATION(ProfileDerived, "ProfileDerived", policy::Default)
	SLB3_DECLARATION(ProfileMultiBases, "ProfileMultiBases", policy::Default)
#endif

void open_Luabind_hierarchy(lua_State* l);

void open_LuaBridge_hierarchy(lua_State* l);

#endif
