#include "expose_const_func.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_FUNCTIONS(Constant, cpp_func)
OOLUA_EXPORT_FUNCTIONS_CONST(Constant, cpp_func_const)

OOLUA_EXPORT_NO_FUNCTIONS(DerivesToUseConstMethod)
