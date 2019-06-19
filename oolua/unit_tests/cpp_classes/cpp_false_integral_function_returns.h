#ifndef CPP_FALSE_INTEGRAL_FUNCTION_RETURNS_H_
#	define CPP_FALSE_INTEGRAL_FUNCTION_RETURNS_H_

#	include "oolua_tests_pch.h"
#	include "gmock/gmock.h"
#	include "oolua_ref.h"
#	include "oolua_table.h"
#	include "cpp_function_returns.h"

struct fake_return_default{};

typedef FunctionReturnType<fake_return_default, OOLUA::Lua_func_ref> FunctionRefReturn;
typedef MockFunctionReturnType<fake_return_default, OOLUA::Lua_func_ref> FunctionRefReturnMock;

typedef FunctionReturnType<fake_return_default, OOLUA::Lua_table_ref> TableRefReturn;
typedef MockFunctionReturnType<fake_return_default, OOLUA::Lua_table_ref> TableRefReturnMock;



typedef FunctionReturnType<fake_return_default, OOLUA::Table> TableObjectReturn;
typedef MockFunctionReturnType<fake_return_default, OOLUA::Table> TableObjectReturnMock;

#endif
