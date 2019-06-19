#ifndef EXPOSE_CONST_FUNCS_H_
#	define EXPOSE_CONST_FUNCS_H_

#	include "cpp_const_funcs.h"
#	include "oolua_dsl.h"

OOLUA_PROXY(Constant)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC_CONST(cpp_func_const)
	OOLUA_MFUNC(cpp_func)
OOLUA_PROXY_END

OOLUA_PROXY(DerivesToUseConstMethod, Constant)
OOLUA_PROXY_END

#endif
