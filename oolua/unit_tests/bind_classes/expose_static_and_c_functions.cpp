#include "expose_static_and_c_functions.h"
#include "oolua_dsl_export.h"

/*[ClassStaticFunctionExport]*/
OOLUA_EXPORT_NO_FUNCTIONS(ClassHasStaticFunction)
/*[ClassStaticFunctionExport]*/


/*DealingWithOverloadedStaticOrCFunctions*/
//the function being proxied is overloaded, specify more info using a cast
OOLUA_CFUNC(
			((void(*)(void))ClassHasStaticFunction::static_function)
			, oolua_ClassHasStaticFunction_static_function
			)

//the function being proxied is overloaded, use the Expressive OOLUA_C_FUNCTION
int oolua_ClassHasStaticFunction_static_function_int(lua_State* vm)
{
	//this function is overloaded
	OOLUA_C_FUNCTION(void, ClassHasStaticFunction::static_function, int)
}
/*DealingWithOverloadedStaticOrCFunctions*/

