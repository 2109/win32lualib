--OOLua compare tests
local root = '../'
create_package('comparison',root,'ConsoleApp')

configuration {}

if os.getenv('LUAJIT_1') then
	platforms { 'x32' }
end

newoption
{
	trigger     = 'SLB3_COMPARE',
	description = 'Compare with SLB3. Note: this requires Lua 5.2'
}
newoption
{
	trigger     = 'LUABIND_COMPARE',
	description = 'Compare with Luabind. Note: this uses an unoffical branch'
}
newoption
{
	trigger     = 'SWIG_COMPARE',
	description = 'Compare with Swig.'
}
newoption
{
	trigger     = 'LUABRIDGE_COMPARE',
	description = 'Compare with LuaBridge.'
}

newoption
{
	trigger     = 'JIT_REBASE',
	description = 'Rebase the image so that the JIT allocator works'
}

local jit_rebase = function(config)
	configuration{config}
	linkoptions{'-pagezero_size 10000 -image_base 100000000'}
	configuration{'*'}	
end

local compare_SLB3 = function(config)
	configuration{config}
	defines { 'OOLUA_SLB_COMPARE','SLB3_CACHE_BASE_CLASS_METHODS=1' }
	files 
	{ 	'SLB3/**.h',
		'src/**.cc'
	}
	configuration{'*'}
end

local compare_Luabind = function(config)
	configuration{config}
	defines 
	{ 
		'OOLUA_LUABIND_COMPARE'
		,'LUABIND_NO_ERROR_CHECKING'
		,'LUABIND_DONT_COPY_STRINGS'
	}
	files 
	{ 	'luabind/**.hpp',
		'src/**.cpp',
	}
	configuration{'*'}
end

local compare_SWIG = function(config)
	configuration{config}
	defines { 'OOLUA_SWIG_COMPARE' }
	configuration{'*'}
end

local compare_Luabridge = function(config)
	configuration{config}
	defines{'OOLUA_LUABRIDGE_COMPARE'}
	files 
	{ 	
		'LuaBridge.h'
		,'RefCountedObject.h'
		,'RefCounterPtr.h'
		,'./detail/.*h'
	}
	includedirs {'./detail/'}
	configuration{'*'}
end


files 
{ 
	'*.h',
	'*.cpp',
}

includedirs 
{
	root .. 'profile/'
	,root .. 'include/'
	,'./'
	,'include/lua'
} 

defines 
{	
--	'OOLUA_UD_SAVE_BYTES=1',
--	'OOLUA_WORK_DSL',
--tests
	'MFUNC_TEST',
	'VFUNC_TEST',
	'CLASS_PARAM_IMPLICIT_CAST_TEST',
}

links
{
	"oolua"
}

	configuration { "vs*"}
		links{"lua51"}

	configuration { "gmake or linux or macosx or xcode3 or codeblocks"}
		links{"lua"}

	jit_rebase( os.getenv('LUAJIT_REBASE') and '*' or 'JIT_REBASE')
	compare_SLB3( os.getenv('OOLUA_SLB3') and '*' or 'SLB3_COMPARE')
	compare_Luabind( os.getenv('OOLUA_LUABIND') and '*' or 'LUABIND_COMPARE')
	compare_SWIG( os.getenv('OOLUA_SWIG') and '*' or 'SWIG_COMPARE')
	compare_Luabridge( os.getenv('OOLUA_LUABRIDGE') and '*' or 'LUABRIDGE_COMPARE')


	configuration{'*'}
	if os.getenv('NO_USERDATA_CHECKS') then
		defines
		{
			'OOLUA_CHECK_EVERY_USERDATA_IS_CREATED_BY_OOLUA=0'
			,'LUABIND_DISABLE_UD_CHECK=1'
		}
	else
		defines
		{
			'OOLUA_CHECK_EVERY_USERDATA_IS_CREATED_BY_OOLUA=1'
			,'OOLUA_USERDATA_OPTIMISATION=1'
		}
	end
