#include "oolua.h"
#include "expose_stub_classes.h"

int main()
{
	return OOLUA::cpp_in_p<Stub1&>::out;
}
