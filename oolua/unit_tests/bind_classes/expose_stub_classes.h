#ifndef EXPOSE_STUB_CLASSES_H_
#	define EXPOSE_STUB_CLASSES_H_

#include "oolua_dsl.h"
#include "cpp_stub_classes.h"

/**[MinimalProxyClass]*/
OOLUA_PROXY(Stub1)
OOLUA_PROXY_END
/**[MinimalProxyClass]*/


OOLUA_PROXY(Stub2)
OOLUA_PROXY_END

OOLUA_PROXY(Stub3)
OOLUA_PROXY_END


OOLUA_PROXY(InvalidStub)
OOLUA_PROXY_END


OOLUA_PROXY(Return_double)
OOLUA_PROXY_END

#endif
