#ifndef EXPOSE_OWNERSHIP_H_
#	define EXPOSE_OWNERSHIP_H_

#include "expose_stub_classes.h"
#include "cpp_userdata_function_params.h"
#include "oolua_dsl.h"

OOLUA_PROXY(OwnershipParamUserData)
	OOLUA_TAGS(Abstract)

	/**[ProxyLuaOutTrait]*/
	OOLUA_MEM_FUNC_RENAME(lua_takes_ownership_of_ref_2_ptr
						, void, refPtr, lua_out_p<Stub1*&>)
	/**[ProxyLuaOutTrait]*/
	OOLUA_MEM_FUNC_RENAME(lua_takes_ownership_of_ref_2_ptr_const
						, void, refPtrConst, lua_out_p<Stub1 const * &>)

	/**[ProxyCppInTrait]*/
	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_ptr_param
						, void, ptr, cpp_in_p<Stub1*>)
	/**[ProxyCppInTrait]*/
	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_ref_to_ptr_param
						, void, refPtr, cpp_in_p<Stub1*&>)


	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_const_ptr_param
						, void, constPtr, cpp_in_p<Stub1 * const>)
	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_ref_2_const_ptr_param
						, void, refConstPtr, cpp_in_p<Stub1 * const &>)


	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_ptr_to_const_param
						, void, ptrConst, cpp_in_p<Stub1 const*>)
	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_ref_to_ptr_to_const_param
						, void, refPtrConst, cpp_in_p<Stub1 const*&>)

	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_const_ptr_to_const_param
						, void, constPtrConst, cpp_in_p<Stub1 const * const>)
	OOLUA_MEM_FUNC_RENAME(cpp_takes_ownership_of_ref_to_const_ptr_to_const_param
						, void, refConstPtrConst, cpp_in_p<Stub1 const * const&>)
OOLUA_PROXY_END

#endif
