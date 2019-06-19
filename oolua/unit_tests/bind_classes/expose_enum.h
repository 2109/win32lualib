#ifndef EXPOSE_ENUM_PARAM_H_
#	define EXPOSE_ENUM_PARAM_H_

#include "oolua_dsl.h"
#include "cpp_enum.h"

/*[ClassEnumExpose]*/
OOLUA_PROXY(Enums)
	OOLUA_TAGS(
		Register_class_enums
	)
	OOLUA_CTORS(
		OOLUA_CTOR(Enums::COLOUR)
	)
	OOLUA_ENUMS(
		OOLUA_ENUM(GREEN)
		OOLUA_ENUM(INVALID)
	)
	OOLUA_MFUNC(set_enum)
	OOLUA_MFUNC(get_enum)
OOLUA_PROXY_END
/*[ClassEnumExpose]*/

#endif
