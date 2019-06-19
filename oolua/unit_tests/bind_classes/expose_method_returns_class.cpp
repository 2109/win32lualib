#include "expose_method_returns_class.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_FUNCTIONS(Method_returns_class
						, ref
						, ref_const
						, ptr
						, ptr_const
						, ref_ptr_const
						, ref_const_ptr_const
						, return_stack_instance
						, returns_null)

OOLUA_EXPORT_FUNCTIONS_CONST(Method_returns_class)
