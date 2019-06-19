#include "oolua.h"
#include "expose_stub_classes.h"

int main()
{
	return OOLUA::lua_out_p<Stub1 const&>::out;
}
