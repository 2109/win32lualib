#ifndef CPP_CONST_FUNCS_H
#	define CPP_CONST_FUNCS_H_

#	include "oolua_tests_pch.h"
#	include "gmock/gmock.h"

class Constant
{
public:
	virtual ~Constant(){}
	virtual void cpp_func_const() const = 0;
	virtual void cpp_func() = 0;
};

class ConstantMock : public Constant
{
public:
	MOCK_CONST_METHOD0(cpp_func_const, void());
	MOCK_METHOD0(cpp_func, void());
};

struct DerivesToUseConstMethod : Constant
{
	void cpp_func(){}
	void cpp_func_const() const{}
};

#endif
