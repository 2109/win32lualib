#ifndef EXPOSE_INTEGRAL_FUNCTION_RETURNS_H_
#	define EXPOSE_INTEGRAL_FUNCTION_RETURNS_H_

#include "cpp_integral_function_returns.h"
#include "oolua_dsl.h"

OOLUA_PROXY(CStringFunctionReturn)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(ptr)
	OOLUA_MFUNC(ptrConst)
OOLUA_PROXY_END

#endif

