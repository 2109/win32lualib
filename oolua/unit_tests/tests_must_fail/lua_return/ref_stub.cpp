#include "oolua.h"
#include "expose_stub_classes.h"

int main()
{
	return OOLUA::lua_return<Stub1&>::out;
}
