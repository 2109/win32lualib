#ifndef EXPOSE_HEIRARCHY_H_
#	define EXPOSE_HEIRARCHY_H_

#include "oolua_dsl.h"
#include "cpp_hierarchy.h"

/*[ExposeAbstract1]*/
OOLUA_PROXY(Abstract1)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(virtualVoidParam3Int)
	OOLUA_MFUNC(func1)
OOLUA_PROXY_END
/*[ExposeAbstract1]*/

OOLUA_PROXY(Abstract2)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(func2_1)
	OOLUA_MFUNC_CONST(constVirtualFunction)
OOLUA_PROXY_END

OOLUA_PROXY(Abstract3)
	OOLUA_TAGS(Abstract)
	OOLUA_MFUNC(func3_1)
OOLUA_PROXY_END

/*[ExposeDerived1Abstract1]*/
OOLUA_PROXY(Derived1Abstract1, Abstract1)
OOLUA_PROXY_END
/*[ExposeDerived1Abstract1]*/

OOLUA_PROXY(TwoAbstractBases, Abstract1, Abstract2)
	OOLUA_MFUNC_CONST(constVirtualFunction)
OOLUA_PROXY_END


OOLUA_PROXY(DerivedFromTwoAbstractBasesAndAbstract3, TwoAbstractBases, Abstract3)
OOLUA_PROXY_END

namespace BASE_HELPERS
{
	//NOTE: Base class order is defined and used in OOLUA code.
	//Immediate bases come first in the order defined in the OOLUA_BASES_START to OOLUA_BASES_END block
	//then the bases of these base classes again in the order they were defined.
	inline std::vector<std::string> allBasesOfDerivedFromTwoAbstractBasesAndAbstract3()
	{
		std::vector<std::string> v;
		v.push_back("TwoAbstractBases");
		v.push_back("Abstract3");
		v.push_back("Abstract1");
		v.push_back("Abstract2");
		return v;
	}

	//Order for roots is defined (due to the nature in which the list is generated in OOLUA code), yet never used.
	//A root base is a base of class which has no bases itself.
	//If a class which is registered has no bases then it is not a root to itself yet can be a root for other classes.
	inline std::vector<std::string> rootBasesOfDerivedFromTwoAbstractBasesAndAbstract3()
	{
		std::vector<std::string> v;
		v.push_back("Abstract3");
		v.push_back("Abstract1");
		v.push_back("Abstract2");
		return v;
	}
} // namespace BASE_HELPERS

#endif
