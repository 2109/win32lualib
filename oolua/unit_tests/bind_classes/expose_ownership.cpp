#include "expose_ownership.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_FUNCTIONS(OwnershipParamUserData
								, lua_takes_ownership_of_ref_2_ptr
								, lua_takes_ownership_of_ref_2_ptr_const

								, cpp_takes_ownership_of_ptr_param
								, cpp_takes_ownership_of_ref_to_ptr_param

								, cpp_takes_ownership_of_const_ptr_param
								, cpp_takes_ownership_of_ref_2_const_ptr_param

								, cpp_takes_ownership_of_ptr_to_const_param
								, cpp_takes_ownership_of_ref_to_ptr_to_const_param

								, cpp_takes_ownership_of_const_ptr_to_const_param
								, cpp_takes_ownership_of_ref_to_const_ptr_to_const_param)

OOLUA_EXPORT_FUNCTIONS_CONST(OwnershipParamUserData)
