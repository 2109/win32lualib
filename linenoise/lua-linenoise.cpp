#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


#include "linenoise.hpp"

using namespace std;

static lua_State* gL = NULL;

void CompletionCallback(const char* buffer, std::vector<std::string>& completions) {

	lua_pushvalue(gL, 3);
	lua_pushstring(gL, buffer);
	int r = lua_pcall(gL, 1, 1, 0);
	if ( r != LUA_OK )  {
		fprintf(stderr, "%s\n", lua_tostring(gL, -1));
		lua_pop(gL, 1);
		return;
	}

	if ( !lua_isnil(gL, -1) ) {
		lua_pushnil(gL);
		while ( lua_next(gL, -2) != 0 ) {
			const char* result = lua_tostring(gL, -1);
			completions.push_back(result);
			lua_pop(gL, 1);
		}
	}
	lua_pop(gL, 1);
}


static int
lreadline(lua_State* L) {
	const char* prompt = luaL_optstring(L, 1, ">>");
	const char* history = luaL_optstring(L, 2, "history.txt");

	linenoise::SetMultiLine(true);
	linenoise::SetHistoryMaxLen(10);
	linenoise::SetCompletionCallback(CompletionCallback);
	linenoise::LoadHistory(history);

	gL = L;
	std::string line = linenoise::Readline(prompt);
	if ( line.empty() == false ) {
		linenoise::AddHistory(line.c_str());
		linenoise::SaveHistory(history);

		gL = NULL;
		lua_pushstring(L, line.c_str());
		return 1;
	}
	gL = NULL;
	return 0;
}


extern "C" __declspec( dllexport ) int
luaopen_linenoise(lua_State* L){
	lua_pushcfunction(L, lreadline);
	return 1;
}