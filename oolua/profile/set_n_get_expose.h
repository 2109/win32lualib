#ifndef PROFILE_SET_N_GET_EXPOSE_H_
#	define PROFILE_SET_N_GET_EXPOSE_H_

#include "set_and_get.h"
#include "oolua.h"

OOLUA_PROXY(Set_get)
	OOLUA_MFUNC(set)
	OOLUA_MFUNC_CONST(get)
OOLUA_PROXY_END

void open_Luabind_set_n_get(lua_State* l);

void open_LuaBridge_set_n_get(lua_State* l);

#if defined OOLUA_SLB_COMPARE && LUA_VERSION_NUM == 502
#	include <SLB3/declaration.h>
	SLB3_DECLARATION(Set_get, "Set_get", policy::Default)
#endif

#endif

