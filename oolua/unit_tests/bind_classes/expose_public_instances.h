#ifndef EXPOSE_PUBLIC_INSTANCES_OOLUA_H_
#	define EXPOSE_PUBLIC_INSTANCES_OOLUA_H_

#include "oolua_dsl.h"
#include "cpp_public_instances.h"
#include "cpp_stub_classes.h"

/*[PublicMembersExpose]*/
OOLUA_PROXY(Public_variables)
	OOLUA_MGET_MSET(an_int)
	OOLUA_MGET_MSET(int_ptr, get_int_ptr, set_int_ptr)
	OOLUA_MGET_MSET(dummy_instance)
	OOLUA_MGET(dummy_ref)
	OOLUA_MGET(dummy_instance_none_ptr)
	OOLUA_MGET(m_int, get_int)
	OOLUA_MGET(m_int)
	OOLUA_MSET(m_int, set_int)
	OOLUA_MSET(m_int)


OOLUA_PROXY_END
/*[PublicMembersExpose]*/

#endif
