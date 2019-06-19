#include "expose_integral_function_params.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_FUNCTIONS_CONST(IntegerFunctionInTraits)
OOLUA_EXPORT_FUNCTIONS(IntegerFunctionInTraits
						, value
						, ref
						, ptr
						, constant
						, refConst
						, ptrConst
						, constPtrConst
						, refConstPtrConst)

OOLUA_EXPORT_FUNCTIONS_CONST(IntegerFunctionInOutTraits)
OOLUA_EXPORT_FUNCTIONS(IntegerFunctionInOutTraits
						, ref
						, ptr
						, refPtr
						, twoRefs)

OOLUA_EXPORT_FUNCTIONS_CONST(IntegerFunctionOutTraits)
OOLUA_EXPORT_FUNCTIONS(IntegerFunctionOutTraits
						, ref
						, ptr
						, refPtr)

OOLUA_EXPORT_FUNCTIONS_CONST(CharFunctionInTraits)
OOLUA_EXPORT_FUNCTIONS(CharFunctionInTraits
						, ptr
						, ptrConst
						, refPtrConst)

OOLUA_EXPORT_FUNCTIONS_CONST(FloatFunctionInTraits)
OOLUA_EXPORT_FUNCTIONS(FloatFunctionInTraits, value)

OOLUA_EXPORT_FUNCTIONS_CONST(DoubleFunctionInTraits)
OOLUA_EXPORT_FUNCTIONS(DoubleFunctionInTraits, value)

OOLUA_EXPORT_FUNCTIONS_CONST(CFunctionInTraits)
OOLUA_EXPORT_FUNCTIONS(CFunctionInTraits, value)
