#include "expose_hierarchy.h"
#include "oolua_dsl_export.h"

/*[ExportAbstract1]*/
OOLUA_EXPORT_FUNCTIONS(Abstract1, func1, virtualVoidParam3Int)
OOLUA_EXPORT_FUNCTIONS_CONST(Abstract1)
/*[ExportAbstract1]*/

OOLUA_EXPORT_FUNCTIONS(Abstract2, func2_1)
OOLUA_EXPORT_FUNCTIONS_CONST(Abstract2, constVirtualFunction)

OOLUA_EXPORT_FUNCTIONS(Abstract3, func3_1)
OOLUA_EXPORT_FUNCTIONS_CONST(Abstract3)

/*[ExportDerived1Abstract1]*/
OOLUA_EXPORT_FUNCTIONS(Derived1Abstract1)
OOLUA_EXPORT_FUNCTIONS_CONST(Derived1Abstract1)
/*[ExportDerived1Abstract1]*/

OOLUA_EXPORT_NO_FUNCTIONS(TwoAbstractBases)
OOLUA_EXPORT_NO_FUNCTIONS(DerivedFromTwoAbstractBasesAndAbstract3)
