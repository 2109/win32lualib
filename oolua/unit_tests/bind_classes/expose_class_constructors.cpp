#include "expose_class_constructors.h"
#include "oolua_dsl_export.h"

OOLUA_EXPORT_NO_FUNCTIONS(ParamConstructor)
OOLUA_EXPORT_NO_FUNCTIONS(TableRefConstructor)
OOLUA_EXPORT_NO_FUNCTIONS(WithOutConstructors)
OOLUA_EXPORT_NO_FUNCTIONS(LuaStateConstructors)
OOLUA_EXPORT_NO_FUNCTIONS(ConstructorThrowsStdException)
OOLUA_EXPORT_NO_FUNCTIONS(DefaultConstructorThrowsStdException)

OOLUA_EXPORT_FUNCTIONS(LightVoidUserDataConstructor)
OOLUA_EXPORT_FUNCTIONS_CONST(LightVoidUserDataConstructor
							, get_ptr)

OOLUA_EXPORT_FUNCTIONS(LightNoneVoidUserDataConstructor)
OOLUA_EXPORT_FUNCTIONS_CONST(LightNoneVoidUserDataConstructor
							, get_ptr)
