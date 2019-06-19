#ifndef EXPOSE_CLASS_ENUMS_H_
#	define EXPOSE_CLASS_ENUMS_H_

#	include "oolua_dsl.h"
#	include "cpp_class_enums.h"

/*
Register_class_enums : tag for hey there are enums in this class that need registering when
the class type is registered
*/

OOLUA_PROXY(ClassWithEnums)
	OOLUA_TAGS(
		No_public_constructors
		, Register_class_enums
	)
	OOLUA_ENUMS(
		OOLUA_ENUM(UNNAMED_ENUM_0)
		OOLUA_ENUM(UNNAMED_ENUM_1)
		OOLUA_ENUM(ENUM_0)
		OOLUA_ENUM(ENUM_1)
	)
OOLUA_PROXY_END

#endif
