#ifndef EXPOSE_FALSE_INTEGRAL_FUNCTION_PARAMS_H_

#include "cpp_false_integral_function_params.h"
#include "oolua_dsl.h"

OOLUA_PROXY(FunctionRefParam)
	OOLUA_TAGS(Abstract)
	OOLUA_MEM_FUNC_RENAME(inTrait, void, value, in_p<Lua_func_ref>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inTraitRef, void, ref, in_p<Lua_func_ref&>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inTraitRefConst, void, refConst, in_p<Lua_func_ref const&>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inOutTraitRef, void, ref, in_out_p<Lua_func_ref&>)
	OOLUA_MEM_FUNC_RENAME(outTraitRef, void, ref, out_p<Lua_func_ref&>)
OOLUA_PROXY_END


OOLUA_PROXY(TableRefParam)
	OOLUA_TAGS(Abstract)
	OOLUA_MEM_FUNC_RENAME(inTrait, void, value, in_p<Lua_table_ref>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inTraitRef, void, ref, in_p<Lua_table_ref&>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inTraitRefConst, void, refConst, in_p<Lua_table_ref const&>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inOutTraitRef, void, ref, in_out_p<Lua_table_ref&>)
	OOLUA_MEM_FUNC_RENAME(outTraitRef, void, ref, out_p<Lua_table_ref&>)
OOLUA_PROXY_END

OOLUA_PROXY(TableObjectParam)
	OOLUA_TAGS(Abstract)
	OOLUA_MEM_FUNC_RENAME(inTrait, void, value, in_p<Table>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inTraitRef, void, ref, in_p<Table&>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inTraitRefConst, void, refConst, in_p<Table const&>) //make it explict
	OOLUA_MEM_FUNC_RENAME(inOutTraitRef, void, ref, in_out_p<Table&>)
	OOLUA_MEM_FUNC_RENAME(outTraitRef, void, ref, out_p<Table&>)
OOLUA_PROXY_END

OOLUA_PROXY(LuaStateParam)
	OOLUA_TAGS(Abstract)
	/**[ProxyCallingLuaStateTrait]*/
	OOLUA_MEM_FUNC(void, value, calling_lua_state)
	/**[ProxyCallingLuaStateTrait]*/
OOLUA_PROXY_END

#endif
