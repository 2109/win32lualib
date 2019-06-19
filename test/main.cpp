#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include "oolua.h"

using namespace std;

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_allocator.h"


class Student {
public:
	Student() {
		_age = 29;
		_name = "mrq";
	}

	Student(int age, std::string& name) {
		_age = age;
		_name = name;
	}

	Student(int age) {
		_age = age;
	}

	Student(std::string& name) {
		_name = name;
	}

	Student(Student& stu) {
		_age = stu.getAge();
		_name = stu.getName();
	}

	int getAge() {
		return _age;
	}

	void setAge(int age) {
		_age = age;
	}

	std::string getName() {
		return _name;
	}

	void setName(std::string name) {
		_name = name;
	}

	void setFriend(Student* friendStu) {
		_friend = friendStu;
	}

	std::string getFriendName() {
		return _friend->getName();
	}

	~Student(){
		cout << "release" << endl;
	}

private:
	int _age;
	std::string _name;
	Student* _friend;
};

OOLUA_PROXY(Student)
OOLUA_CTORS(
OOLUA_CTOR(int)
OOLUA_CTOR(std::string&)
OOLUA_CTOR(int, std::string&)
OOLUA_CTOR(Student&)
)
OOLUA_MFUNC(getAge)
OOLUA_MFUNC(getName)
OOLUA_MFUNC(setAge)
OOLUA_MFUNC(setName)
OOLUA_MFUNC(setFriend)
OOLUA_MFUNC(getFriendName)
OOLUA_PROXY_END
OOLUA_EXPORT_FUNCTIONS(Student, getAge, setAge, getName, setName, setFriend, getFriendName)
OOLUA_EXPORT_FUNCTIONS_CONST(Student)


int
get_test(lua_State* L) {
	lua_pushstring(L, "fuck");
	return 1;
}

int main(int argc, char *argv[]) {

	void* ptr = malloc(1024);
	OOLUA::Script script;
	script.register_class<Student>();

	lua_pushcfunction(script.state(), get_test);
	lua_setglobal(script.state(), "get_test");

	if ( !script.run_file("test_oolua.lua") ) {
		cout << OOLUA::get_last_error(script) << endl;
	}


	return 0;
}