#ifndef EXPOSE_METHOD_RETURNS_CLASS_H_
#	define EXPOSE_METHOD_RETURNS_CLASS_H_

#	include "oolua_dsl.h"
#	include "cpp_method_returns_class.h"
#	include "expose_stub_classes.h"

OOLUA_PROXY(Method_returns_class)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(ref_const)
	OOLUA_MFUNC(ptr)
	OOLUA_MFUNC(ptr_const)
	OOLUA_MFUNC(ref_ptr_const)
	OOLUA_MFUNC(ref_const_ptr_const)
	OOLUA_MFUNC(return_stack_instance)
	OOLUA_MFUNC(returns_null)
OOLUA_PROXY_END

#endif
