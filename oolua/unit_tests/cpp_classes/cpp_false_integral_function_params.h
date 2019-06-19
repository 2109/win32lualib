#ifndef CPP_FALSE_INTEGRAL_FUNCTION_PARAMS_H_
#	define CPP_FALSE_INTEGRAL_FUNCTION_PARAMS_H_

#	include "oolua_tests_pch.h"
#	include "gmock/gmock.h"
#	include "oolua_ref.h"
#	include "oolua_table.h"
#	include "cpp_function_params.h"

struct oolua_test_false_integral_in_traits{};

typedef FunctionParamType<oolua_test_false_integral_in_traits, OOLUA::Lua_func_ref> FunctionRefParam;
typedef MockFunctionParamType<oolua_test_false_integral_in_traits, OOLUA::Lua_func_ref> FunctionRefParamMock;

typedef FunctionParamType<oolua_test_false_integral_in_traits, OOLUA::Lua_table_ref> TableRefParam;
typedef MockFunctionParamType<oolua_test_false_integral_in_traits, OOLUA::Lua_table_ref> TableRefParamMock;

typedef FunctionParamType<oolua_test_false_integral_in_traits, OOLUA::Table> TableObjectParam;
typedef MockFunctionParamType<oolua_test_false_integral_in_traits, OOLUA::Table> TableObjectParamMock;

typedef FunctionParamType<oolua_test_false_integral_in_traits, lua_State*> LuaStateParam;
typedef MockFunctionParamType<oolua_test_false_integral_in_traits, lua_State*> LuaStateParamMock;

#endif
