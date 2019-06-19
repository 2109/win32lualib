#ifndef EXPOSE_MAX_PARAMS_H_
#	define EXPOSE_MAX_PARAMS_H_

#	include "cpp_max_params.h"
#	include "oolua_dsl.h"

OOLUA_PROXY(VoidReturnIntParam)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(one)
	OOLUA_MFUNC(two)
	OOLUA_MFUNC(three)
	OOLUA_MFUNC(four)
	OOLUA_MFUNC(five)
	OOLUA_MFUNC(six)
	OOLUA_MFUNC(seven)
	OOLUA_MFUNC(eight)
OOLUA_PROXY_END

#endif
