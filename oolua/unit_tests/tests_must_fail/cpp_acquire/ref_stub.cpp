#include "oolua.h"
#include "expose_stub_classes.h"

int main()
{
	return OOLUA::cpp_acquire_ptr<Stub1&>::out;
}
