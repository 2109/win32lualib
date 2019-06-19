#include "expose_vector_int.h"
#include "oolua_dsl_export.h"

/*[StdVectorProxyExport]*/
OOLUA_EXPORT_FUNCTIONS(vector_int, push_back, pop_back)
OOLUA_EXPORT_FUNCTIONS_CONST(vector_int, size)
/*[StdVectorProxyExport]*/
