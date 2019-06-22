#include <iostream>
#include "lua.hpp"
#include "parser.h"

int luaopen_ptocxx(lua_State* L);

int main() {
	/*lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	luaL_requiref(L, "pto", luaopen_ptocxx, 1);

	if ( luaL_loadfile(L, "test.lua") != LUA_OK )  {
	std::cerr << lua_tostring(L, -1) << std::endl;
	exit(1);
	}

	if ( lua_pcall(L, 0, 0, 0) != LUA_OK )  {
	std::cerr << lua_tostring(L, -1) << std::endl;
	exit(1);
	}*/

	try {
		LuaPto::ParserContext ctx("./");
		ctx.Import("login");
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	
	return 0;
}