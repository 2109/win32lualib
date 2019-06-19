#include "oolua.h"
#include "expose_stub_classes.h"

int main()
{
	return OOLUA::lua_maybe_null<Stub1 const*&>::out;
}
