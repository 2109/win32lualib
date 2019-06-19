#include "oolua.h"

int main()
{
	return OOLUA::lua_acquire_ptr<int const*>::out;
}
