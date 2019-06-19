#include "oolua.h"
#include "expose_stub_classes.h"

int main()
{
	return OOLUA::maybe_null<Stub1 const&>::out;
}
