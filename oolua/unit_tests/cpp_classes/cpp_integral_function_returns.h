#ifndef CPP_INTEGRAL_FUNCTION_RETURNS_H_
#	define CPP_INTEGRAL_FUNCTION_RETURNS_H_

#	include "cpp_function_returns.h"

struct fake_integral_return_specialisation{};

typedef FunctionReturnType<fake_integral_return_specialisation, char> CStringFunctionReturn;
typedef MockFunctionReturnType<fake_integral_return_specialisation, char> CStringFunctionReturnMock;

#endif
