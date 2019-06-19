
#include "expose_false_integral_function_params.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_FUNCTIONS(FunctionRefParam
						, inOutTraitRef
						, inTraitRef
						, inTrait
						, inTraitRefConst
						, outTraitRef)

OOLUA_EXPORT_FUNCTIONS_CONST(FunctionRefParam)



OOLUA_EXPORT_FUNCTIONS(TableRefParam
						, inOutTraitRef
						, inTraitRef
						, inTrait
						, inTraitRefConst
						, outTraitRef)

OOLUA_EXPORT_FUNCTIONS_CONST(TableRefParam)


OOLUA_EXPORT_FUNCTIONS(TableObjectParam
						, inOutTraitRef
						, inTraitRef
						, inTrait
						, inTraitRefConst
						, outTraitRef)

OOLUA_EXPORT_FUNCTIONS_CONST(TableObjectParam)


OOLUA_EXPORT_FUNCTIONS(LuaStateParam, value)
OOLUA_EXPORT_FUNCTIONS_CONST(LuaStateParam)
