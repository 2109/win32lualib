#ifndef EXPOSE_FALSE_INTEGRAL_FUNCTION_RETURNS_H_
#	define EXPOSE_FALSE_INTEGRAL_FUNCTION_RETURNS_H_

#include "oolua_dsl.h"
#include "cpp_false_integral_function_returns.h"

OOLUA_PROXY(FunctionRefReturn)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(refConst)
OOLUA_PROXY_END

OOLUA_PROXY(TableRefReturn)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(refConst)
OOLUA_PROXY_END


OOLUA_PROXY(TableObjectReturn)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(refConst)
OOLUA_PROXY_END

#endif
