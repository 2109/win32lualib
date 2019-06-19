#ifndef CPP_PRIVATE_DESTRUCTOR_H_
#	define CPP_PRIVATE_DESTRUCTOR_H_

#	include "oolua.h"

struct PrivateDestructor
{
	void release()
	{
		delete this;
	}
	static PrivateDestructor* create()
	{
		return new PrivateDestructor;
	}
private:
	PrivateDestructor(PrivateDestructor const&);
	PrivateDestructor& operator =(PrivateDestructor const&);
	PrivateDestructor(){}
	~PrivateDestructor(){}
};


OOLUA_PROXY(PrivateDestructor)
	OOLUA_TAGS(
		No_public_constructors
		, No_public_destructor
	)
OOLUA_PROXY_END

#endif
