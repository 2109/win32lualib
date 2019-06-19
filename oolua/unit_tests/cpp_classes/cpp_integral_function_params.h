#ifndef CPP_INTEGRAL_FUNCTION_PARAMS_H_
#	define CPP_INTEGRAL_FUNCTION_PARAMS_H_

#	include "cpp_function_params.h"

struct oolua_test_int_in_traits{};
struct oolua_test_int_out_traits{};
struct oolua_test_int_in_out_traits{};

typedef FunctionParamType<oolua_test_int_in_traits, int> IntegerFunctionInTraits;
typedef MockFunctionParamType<oolua_test_int_in_traits, int> IntegerFunctionInTraitsMock;

typedef FunctionParamType<oolua_test_int_out_traits, int> IntegerFunctionOutTraits;
typedef MockFunctionParamType<oolua_test_int_out_traits, int> IntegerFunctionOutTraitsMock;

typedef FunctionParamType<oolua_test_int_in_out_traits, int> IntegerFunctionInOutTraits;
typedef MockFunctionParamType<oolua_test_int_in_out_traits, int> IntegerFunctionInOutTraitsMock;

typedef FunctionParamType<oolua_test_int_in_out_traits, char> CharFunctionInTraits;
typedef MockFunctionParamType<oolua_test_int_in_out_traits, char> CharFunctionInTraitsMock;


typedef FunctionParamType<oolua_test_int_in_traits, float> FloatFunctionInTraits;
typedef MockFunctionParamType<oolua_test_int_in_traits, float> FloatFunctionInTraitsMock;

typedef FunctionParamType<oolua_test_int_in_traits, double> DoubleFunctionInTraits;
typedef MockFunctionParamType<oolua_test_int_in_traits, double> DoubleFunctionInTraitsMock;

struct lua_State;
typedef FunctionParamType<oolua_test_int_in_traits, int(*)(lua_State*)> CFunctionInTraits; // NOLINT
typedef MockFunctionParamType<oolua_test_int_in_traits, int(*)(lua_State*)> CFunctionInTraitsMock; //NOLINT

#endif
