#ifndef CPP_FUNCTION_RETURNS_H_
#	define CPP_FUNCTION_RETURNS_H_


#	include "oolua_tests_pch.h"
#	include "gmock/gmock.h"

#	if defined __GNUC__ && defined __STRICT_ANSI__
#		pragma GCC system_header
#	endif

template<typename Specialisation, typename ReturnType>
class FunctionReturnType
{
public:
	virtual ~FunctionReturnType(){}
	virtual ReturnType value() = 0;
	/**[PtrReturn]*/
	virtual ReturnType* ptr() = 0;
	/**[PtrReturn]*/
	virtual ReturnType& ref()=0;
	virtual ReturnType*& refPtr() = 0;
	virtual ReturnType const constant() = 0;
	virtual ReturnType const & refConst() = 0;
	/**[PtrConstReturn]*/
	virtual ReturnType const * ptrConst() = 0;
	virtual ReturnType const* & refPtrConst() = 0;
	/**[ConstPtrReturn]*/
	virtual ReturnType * const constPtr() = 0;
	/**[ConstPtrReturn]*/
	virtual ReturnType * const& refConstPtr() = 0;
	virtual ReturnType const * const constPtrConst() = 0;
	virtual ReturnType const * const & refConstPtrConst() = 0;
};

template<typename Specialisation, typename ReturnType>
class MockFunctionReturnType : public FunctionReturnType<Specialisation, ReturnType>
{
public:
	MOCK_METHOD0_T(value, ReturnType());
	MOCK_METHOD0_T(ptr, ReturnType*());
	MOCK_METHOD0_T(ref, ReturnType&());
	MOCK_METHOD0_T(refPtr, ReturnType*&());

	MOCK_METHOD0_T(constant, ReturnType const());
	MOCK_METHOD0_T(refConst, ReturnType const &());
	MOCK_METHOD0_T(ptrConst, ReturnType const *());
	MOCK_METHOD0_T(refPtrConst, ReturnType const* &());
	MOCK_METHOD0_T(constPtr, ReturnType * const());
	MOCK_METHOD0_T(refConstPtr, ReturnType * const&());
	MOCK_METHOD0_T(constPtrConst, ReturnType const * const());
	MOCK_METHOD0_T(refConstPtrConst, ReturnType const * const &());
};


#endif
