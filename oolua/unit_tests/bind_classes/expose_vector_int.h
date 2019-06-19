#ifndef EXPOSE_VECTOR_INT_H_
#	define EXPOSE_VECTOR_INT_H_

#	include <vector>
#	include "oolua_dsl.h"

/**[StdVectorProxy]*/
//typedef the type of vector into the global namespace
//This is required as a vector has more than one template type
//and the commas in the template confuse a macro.
typedef std::vector<int> vector_int;

OOLUA_PROXY(vector_int)
	//C++11 adds an overload
	//OOLUA_MFUNC(push_back)
	OOLUA_MEM_FUNC(void, push_back, class_::const_reference)
	OOLUA_MFUNC(pop_back)
	OOLUA_MFUNC_CONST(size)
OOLUA_PROXY_END
/**[StdVectorProxy]*/
#endif
