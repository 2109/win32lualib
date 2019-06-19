#include "expose_out_params.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_FUNCTIONS(OutParamsTest
						, int_ref
						, two_int_refs
						, int_ptr_in_out
						, int_ref_out
						, int_ref_change
						, int_ptr
						, int_ptr_out
						, int_ptr_change_pointee
						, return_int_and_2_int_refs)
OOLUA_EXPORT_FUNCTIONS_CONST(OutParamsTest)


OOLUA_EXPORT_FUNCTIONS(OutParamsUserData
						, ref
						, ptr
						, ref_param)
OOLUA_EXPORT_FUNCTIONS_CONST(OutParamsUserData)

OOLUA_EXPORT_NO_FUNCTIONS(MockParamWithStringMember)
