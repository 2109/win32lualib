#ifndef EXPOSE_INTEGRAL_FUNCTION_PARAMS_H_
#	define EXPOSE_INTEGRAL_FUNCTION_PARAMS_H_

#include "oolua_dsl.h"
#include "cpp_integral_function_params.h"
#include "cpp_function_params.h"


OOLUA_PROXY(IntegerFunctionInTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(ptr)
	OOLUA_MFUNC(constant)
	OOLUA_MFUNC(refConst)
	OOLUA_MFUNC(ptrConst)
	OOLUA_MFUNC(constPtrConst)
	OOLUA_MFUNC(refConstPtrConst)
OOLUA_PROXY_END

OOLUA_PROXY(IntegerFunctionInOutTraits)
	OOLUA_TAGS(Abstract)
	/**[IntegerInOutOutTraitRef]*/
	OOLUA_MEM_FUNC(void, ref, in_out_p<int&>)
	/**[IntegerInOutOutTraitRef]*/
	OOLUA_MEM_FUNC(void, ptr, in_out_p<int*>)
	OOLUA_MEM_FUNC(void, refPtr, in_out_p<int*&>)
	OOLUA_MEM_FUNC(void, twoRefs, in_out_p<int&>, in_out_p<int&>)
OOLUA_PROXY_END

OOLUA_PROXY(IntegerFunctionOutTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MEM_FUNC(void, ref, out_p<int&>)
	OOLUA_MEM_FUNC(void, ptr, out_p<int*>)
	OOLUA_MEM_FUNC(void, refPtr, out_p<int*&>)
OOLUA_PROXY_END

OOLUA_PROXY(CharFunctionInTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(ptr)
	OOLUA_MFUNC(ptrConst)
	OOLUA_MFUNC(refPtrConst)
OOLUA_PROXY_END

OOLUA_PROXY(FloatFunctionInTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
OOLUA_PROXY_END

OOLUA_PROXY(DoubleFunctionInTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
OOLUA_PROXY_END

OOLUA_PROXY(CFunctionInTraits)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
OOLUA_PROXY_END

#endif
