
#include "oolua.h"
#include "set_n_get_expose.h"
#include "hierarchy_expose.h"
#include <iostream>



#ifdef OOLUA_SWIG_COMPARE
	extern "C"
	{
		extern int luaopen_swig_profile(lua_State* L);
		extern void OOLUA_SWIG_pass_pointer_to_lua(lua_State* L,void* ptr,char* type,int own);
	}
#endif

#ifdef OOLUA_LUABIND_COMPARE
#	include "luabind/luabind.hpp"
#endif

#ifdef OOLUA_LUABRIDGE_COMPARE
#	include "LuaBridge.h"
#endif

#if defined OOLUA_SLB_COMPARE && LUA_VERSION_NUM == 502
#	include "SLB3/slb.h"
#	include "SLB3/extra/script.h"
#endif

void run_compare(lua_State* lvm, std::string const& func_name, std::string const& binding )
{
	OOLUA::Lua_function caller(lvm);
	if (! OOLUA::run_file(lvm,"./binding_compare.lua") || ! caller(func_name,binding) )
		std::cout <<OOLUA::get_last_error(lvm);
}

int main()
{

#ifdef OOLUA_LUABIND_COMPARE
#	ifdef MFUNC_TEST
	{
		OOLUA::Script L;
		open_Luabind_set_n_get(L);
		run_compare(L,"mfunc","LuaBind");
	}
#	endif
#	ifdef VFUNC_TEST
	{
		OOLUA::Script L;
		open_Luabind_hierarchy(L);
		run_compare(L,"vfunc","LuaBind");
	}
#	endif
#	ifdef CLASS_PARAM_IMPLICIT_CAST_TEST
	{
		OOLUA::Script L;
		open_Luabind_hierarchy(L);
		run_compare(L,"class_param","LuaBind");
	}
#	endif
#endif



#ifdef OOLUA_LUABRIDGE_COMPARE
#	ifdef MFUNC_TEST
	{
		OOLUA::Script L;
		open_LuaBridge_set_n_get(L);
		run_compare(L,"mfunc","LuaBridge");
	}
#	endif

#	ifdef VFUNC_TEST
	{
		OOLUA::Script L;
		open_LuaBridge_hierarchy(L);
		run_compare(L,"vfunc","LuaBridge");
	}
#	endif
#	ifdef CLASS_PARAM_IMPLICIT_CAST_TEST
	{
		OOLUA::Script L;
		open_LuaBridge_hierarchy(L);
		run_compare(L,"class_param","LuaBridge");
	}
#	endif
#endif



#ifdef MFUNC_TEST	
	{
		OOLUA::Script L;
		L.register_class<Set_get>();
		run_compare(L,"mfunc","OOLua");
	}
#endif

#ifdef VFUNC_TEST
	{
		OOLUA::Script L;
		OOLUA::register_class_and_bases<ProfileMultiBases>(L);
		run_compare(L,"vfunc","OOLua");
	}
#endif

#ifdef CLASS_PARAM_IMPLICIT_CAST_TEST
	{
		OOLUA::Script L;
		OOLUA::register_class_and_bases<ProfileMultiBases>(L);
		run_compare(L,"class_param","OOLua");
	}
#endif



#if defined OOLUA_SLB_COMPARE && LUA_VERSION_NUM == 502
#	ifdef MFUNC_TEST
	{
		SLB3::extra::Script L;
		L.init();
		SLB3::Register<Set_get>(L);
		run_compare(L,"mfunc","SLB3");
	}
#	endif

#	ifdef VFUNC_TEST
	{
		SLB3::extra::Script L;
		L.init();
		SLB3::Register<ProfileBase>(L);
		SLB3::Register<ProfileAnotherBase>(L);
		SLB3::Register<ProfileDerived>(L);
		SLB3::Register<ProfileMultiBases>(L);
		run_compare(L,"vfunc","SLB3");
	}
#	endif
#	ifdef CLASS_PARAM_IMPLICIT_CAST_TEST
	{
		SLB3::extra::Script L;
		L.init();
		SLB3::Register<ProfileBase>(L);
		SLB3::Register<ProfileAnotherBase>(L);
		SLB3::Register<ProfileDerived>(L);
		SLB3::Register<ProfileMultiBases>(L);
		run_compare(L,"class_param","SLB3");
	}
#	endif
#endif



#ifdef OOLUA_SWIG_COMPARE
#	ifdef MFUNC_TEST
	{
		OOLUA::Script L;
		luaopen_swig_profile(L);
		run_compare(L,"mfunc","SWIG");
	}
#	endif
#	ifdef VFUNC_TEST
	{
		OOLUA::Script L;
		luaopen_swig_profile(L);
		run_compare(L,"vfunc","SWIG");
	}
#	endif
#	ifdef CLASS_PARAM_IMPLICIT_CAST_TEST
	{
		OOLUA::Script L;
		luaopen_swig_profile(L);
		run_compare(L,"class_param","SWIG");
	}
#	endif
#endif


}