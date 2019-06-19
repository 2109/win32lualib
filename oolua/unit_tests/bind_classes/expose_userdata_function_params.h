#ifndef EXPOSE_USERDATA_FUNCTION_PARAMS_H_
#	define EXPOSE_USERDATA_FUNCTION_PARAMS_H_

#include "oolua_dsl.h"
#include "cpp_userdata_function_params.h"

/**[ExposeOutParamsUserData]*/
OOLUA_PROXY(HasIntMember)
OOLUA_PROXY_END

OOLUA_PROXY(InParamUserData)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(value)
	OOLUA_MFUNC(constant)
	OOLUA_MFUNC(ref)
	OOLUA_MFUNC(refConst)
	OOLUA_MFUNC(ptr)
	OOLUA_MFUNC(refPtr)

	OOLUA_MFUNC(ptrConst)
	/**[UserDataProxyInTraitRefPtrConst]*/
	OOLUA_MFUNC(refPtrConst)
	/**[UserDataProxyInTraitRefPtrConst]*/
	OOLUA_MFUNC(constPtrConst)
	OOLUA_MFUNC(refConstPtrConst)
OOLUA_PROXY_END

OOLUA_PROXY(OutParamUserData)
	OOLUA_TAGS(Abstract)
	OOLUA_MEM_FUNC_RENAME(outTraitRef, void, ref, out_p<HasIntMember&>)
	OOLUA_MEM_FUNC_RENAME(outTraitPtr, void, ptr, out_p<HasIntMember*>)//When does an out_p on a pointer make sense??
	/**[UserDataProxyOutTraitRefPtr]*/
	OOLUA_MEM_FUNC_RENAME(outTraitRefPtr, void, refPtr, out_p<HasIntMember*&>)
	/**[UserDataProxyOutTraitRefPtr]*/
	OOLUA_MEM_FUNC_RENAME(inOutTraitRef, void, ref, in_out_p<HasIntMember&>)
	OOLUA_MEM_FUNC_RENAME(inOutTraitPtr, void, ptr, in_out_p<HasIntMember*>)//When does an out_p on a pointer make sense??
	OOLUA_MEM_FUNC_RENAME(inOutTraitRefPtr, void, refPtr, in_out_p<HasIntMember*&>)
OOLUA_PROXY_END

OOLUA_PROXY(InOutParamUserData)
	OOLUA_TAGS(Abstract)
OOLUA_PROXY_END
/**[ExposeOutParamsUserData]*/

OOLUA_PROXY(LightParamUserData)
	OOLUA_TAGS(Abstract)
	/**[ExposeLightUserDataParam]*/
	OOLUA_MEM_FUNC(void, value, light_p<void*>)
	/**[ExposeLightUserDataParam]*/
OOLUA_PROXY_END

OOLUA_PROXY(LightNoneVoidParamUserData)
	OOLUA_TAGS(Abstract)
	/**[ExposeLightUserDataNonVoidParam]*/
	OOLUA_MEM_FUNC(void, ptr, light_p<InvalidStub*>)
	/**[ExposeLightUserDataNonVoidParam]*/
OOLUA_PROXY_END

#endif
