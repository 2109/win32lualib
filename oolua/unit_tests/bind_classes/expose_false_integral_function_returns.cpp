
#include "expose_false_integral_function_returns.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_FUNCTIONS(FunctionRefReturn
						, value
						, ref
						, refConst)
OOLUA_EXPORT_FUNCTIONS_CONST(FunctionRefReturn)

OOLUA_EXPORT_FUNCTIONS(TableRefReturn
						, value
						, ref
						, refConst)
OOLUA_EXPORT_FUNCTIONS_CONST(TableRefReturn)

OOLUA_EXPORT_FUNCTIONS(TableObjectReturn
						, value
						, ref
						, refConst)
OOLUA_EXPORT_FUNCTIONS_CONST(TableObjectReturn)
