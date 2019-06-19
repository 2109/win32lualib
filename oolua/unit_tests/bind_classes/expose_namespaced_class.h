#ifndef EXPOSE_NAMESPACED_CLASS_H_
#	define EXPOSE_NAMESPACED_CLASS_H_

#include "oolua_dsl.h"
#include "cpp_class_in_namespace.h"

namespace OOLUA
{
	typedef TEST_NAMESPACED_CLASS::Namespaced Namespaced;
} // namespace OOLUA

OOLUA_PROXY(Namespaced)
OOLUA_PROXY_END

#endif
