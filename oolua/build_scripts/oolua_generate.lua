--/*
--[[
	\file oolua_generate.lua
	\brief Lua module for generating required OOLua configurable boilerplate code.
--]]
--*/

local copyright = [[
/*
The MIT License

Copyright (c) 2009 - 2013 Liam Devine

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
]]

--[[This allows doxygen to document the functions as you may expect--]]
--function default_details();
--function defaults();
--function gen(options,path);

--/**
--[[
\addtogroup OOLuaConfig
@{
	\addtogroup OOLuaFileGeneration File Generation
	@{
		\brief Lua module for generating configurable OOLua boilerplate code.
		\details The \ref OOLuaFileGeneration "\"oolua_generate\"" Lua module provides
		information about the default limits and allows generation of boilerplate code
		using user defined limits or regeneration with default values, the details of
		these being :
		\anchor OOLuaConfigLuaParams
		\anchor OOLuaConfigCppParams
		\anchor OOLuaConfigConstructorParams
		\anchor OOLuaConfigClassFunctions
		\snippet oolua_generate.lua GenDefaultDetails

		The most common change to these options is the number of functions which can be
		registered for a proxy class, this limit applies individually to constant and none
		constant functions, base class methods that are registered in a base class do not
		decrease the count for a derived class.\n
		Using the Lua interpreter to regenerate the OOLua files increasing this option
		whilst using default values for the remaining options:
		\code{.lua}lua -e "require'build_scripts.oolua_generate'.gen({class_functions=30},'include/')"\endcode
		<p>
		For convenience you do not need a version of Lua installed on a machine to run this
		module, Premake the project file generator used in OOLua already contains a copy of
		Lua 5.1 (it has some modifications to the core libraries).  To generate the files
		with the same options as above :
		\code{.lua}premake4 --class_functions=30 oolua-gen\endcode
		<p>

		The module returns a table with the following functions \n
		\snippet oolua_generate.lua GenModuleReturn

		\fn function gen(options,path)
		\param options [optional] Defaults to the library \ref defaults
		\param path [optional] Defaults to the current working directory
		\brief Generate boilerplate header files
		\details Generates boilerplate C++ files code required for OOLua using the passed options
		or if an option is not present then the default is used. If Path is not nil then it is
		required to be a string which is slash postfixed.

		\fn function default_details()
		\brief Returns the library defaults and details
		\details Returns a table detailing the library defaults and descriptions
		\snippet oolua_generate.lua GenDefaultDetails
		\returns Table of the format { config_option ={desc='blurb',value=0} }


		\fn function defaults()
		\brief Gets the default options as key(string) and value(number) entries in a table
		\details Modifies the table returned by \ref default_details so the it is formatted correctly
		for any functions it will be passed to.
		\returns Table of the format { config_option = 0 }
		\see default_details
	@}
@}
--]]
--*/

--[[This allows doxygen to document the functions as you may expect--]]
local _default_details= function()
	--/**[GenDefaultDetails]*/
	return
	{
		lua_params =
		{	desc ='Maximum amount of parameters for a call to a Lua function'
			,value=10
		}
		,cpp_params =
		{
			desc='Maximum number of parameters a C++ function can have'
			,value=8
		}
		,constructor_params =
		{
			desc='Maximum amount of parameters for a constructor of a proxied type'
			,value=5
		}
		,class_functions =
		{
			desc='Maximum amount of class functions that can be registered for each proxied type'
			,value=15
		}
	}
	--/**[GenDefaultDetails]*/
end

local default_details= function()
	--[[This allows doxygen to document the functions as you may expect--]]
	return _default_details()
end


local defaults= function()
	local d = default_details()
	for k,v in pairs(d) do d[k]=d[k].value end
	return d
end

local file_create = function(options,path,filename,brief_blurb,detail_blurb)
	local f_ = io.open( (path or '') ..filename,'w')
	f_:write(
	copyright
	..'/** \n'
	..'\t\\file '.. filename ..'\n'
	..'\t\\date ' ..os.date() .. '\n'
	..(brief_blurb and ('\t\\brief \n' .. brief_blurb  ..'\n') or '')
	..'\t\\details \n' .. ( detail_blurb and (detail_blurb..'\n')  or '')
	..'\tConfigurable values as set when generating this file\n'
	)
	local desc = default_details()
	for k,v in pairs(options) do
		f_:write('\t\\li ' .. k .. ' ' ..v .. ' - ' .. desc[k].desc ..'\t(Default '.. desc[k].value ..')\n')
	end

	f_:write([[
	\note Warning this file was generated, edits to the file will not persist if it is regenerated.
*/

]])

	local macro_guard = filename:upper():gsub('%.','_') ..'_'
	f_:write(
	'#ifndef '..macro_guard ..'\n'
	..'#\tdefine '..macro_guard ..'\n'
	..'/** \\cond INTERNAL */\n\n')
	return f_
end

local file_close = function(f,endif_prefix)
	f:write((endif_prefix or '') ..'\n#endif\n')
	f:close()
end








local gen_boilerplate = function(options,path)
	local f = nil

	local generic_write = function(blurb_prefix,start_index,end_index,name,second_prefix)
		second_prefix = second_prefix or ''
		f:write(blurb_prefix)
		if start_index == 0 then f:write('#define ' ..name..'0\n') start_index = start_index+1 end
		if start_index == 1 then f:write('#define ' ..name..'1 ' ..name..'NUM(1)\n') start_index = start_index+1 end
		for i = start_index, end_index do
			f:write('#define '..name..i..' ' ..name..(i-1).. ' ' ..second_prefix .. ' ' .. name ..'NUM('..i..')\n')
		end
		f:write('\n')
	end

	local generic_generate = function(def,defsuffix,defiesuffix,start_index, end_index)
		f:write('#define ' ..def ..defsuffix)
		for i = start_index, end_index do
			if i%3 == 0 then f:write('\\\n\t') end
			f:write(def .. defiesuffix..'('..i..') ')
		end
		--don't leave whitespace at the end of lines
		f:seek(cur,-1);
		local endOfLineChar = f:read(1);
		if f:read(1) == ' ' then f:seek(cur,-1) end
		f:write('\n')
	end

	local export = options.class_functions
	options.class_functions = nil
	f = file_create(options,path,'oolua_boilerplate.h')
	options.class_functions = export
	export = nil

	f:write([[
#	include "oolua_config.h"
#if defined __GNUC__ && defined __STRICT_ANSI__
/*shhhh about va args and c99*/
#	pragma GCC system_header
#endif

]])

	generic_write([[
/*
 Functions which are proxied when parameters have traits allow for a direction
 for the type such that it needs pushing back to the lua stack
 param return macros
*/
#define OOLUA_BACK_INTERNAL_NUM(NUM) \
MSC_PUSH_DISABLE_CONDITIONAL_CONSTANT_OOLUA \
	if( P ## NUM ## _::out ) \
		OOLUA::INTERNAL::Proxy_stack_helper<P ## NUM ##_::traits, P ## NUM ##_::owner>::push(vm, p ## NUM); \
MSC_POP_COMPILER_WARNING_OOLUA
]], 0, options.cpp_params, 'OOLUA_BACK_INTERNAL_' )

	f:write([[
/*
Functions proxied using the following macro may have traits
 and therefore the types have the possiblity of not being on the stack
 it therefore uses a rolling parameter index to index the stack.
*/
#define OOLUA_INTERNAL_PARAM(NUM, PARAM) \
	typedef OOLUA::INTERNAL::param_type<PARAM > P ## NUM ##_; \
	P ## NUM ##_::pull_type p ## NUM; \
	MSC_PUSH_DISABLE_CONDITIONAL_CONSTANT_OOLUA \
	if( P ## NUM ##_::in ) \
		OOLUA::INTERNAL::Proxy_stack_helper<P ## NUM ##_::traits, P ## NUM ##_::owner>::get(rolling_param_index, vm, p ## NUM); \
	MSC_POP_COMPILER_WARNING_OOLUA

#define OOLUA_PARAMS_INTERNAL_0(StackIndex)
#define OOLUA_PARAMS_INTERNAL_1(StackIndex, PARAM1) int rolling_param_index = StackIndex; OOLUA_INTERNAL_PARAM(1, PARAM1)
]])
	local previous = 'StackIndex, PARAM1'
	for i = 2, options.cpp_params do
		local now = previous .. ', PARAM' .. i
		f:write('#define OOLUA_PARAMS_INTERNAL_'..i..'('..now..') OOLUA_PARAMS_INTERNAL_'..(i-1)..'('..previous ..')  OOLUA_INTERNAL_PARAM('..i..', PARAM'..i..')\n')
		previous = now
	end
	f:write('\n')

	f:write([[
/*
macros for when using default traits and deducing the function signature
for these type of proxied functions the parameters can not use traits and
therefore are all on the stack, these then only need an offset for the first
parameters.
*/
#define OOLUA_INTERNAL_DEFAULT_PARAM(NUM, OFFSET) \
	typedef OOLUA::INTERNAL::param_type<P ## NUM > P ## NUM ##_; \
	typename P ## NUM ##_::pull_type p ## NUM; \
	OOLUA::INTERNAL::Proxy_stack_helper<P ## NUM ##_, P ## NUM ##_::owner>::get(vm, NUM + OFFSET, p ## NUM);

#define OOLUA_PARAMS_DEFAULT_INTERNAL_0(OFFSET)
#define OOLUA_PARAMS_DEFAULT_INTERNAL_1(OFFSET) OOLUA_INTERNAL_DEFAULT_PARAM(1, OFFSET)
]])
	for i = 2, options.cpp_params do
		f:write('#define OOLUA_PARAMS_DEFAULT_INTERNAL_'..i..'(OFFSET) OOLUA_PARAMS_DEFAULT_INTERNAL_'..(i-1)..'(OFFSET)  OOLUA_INTERNAL_DEFAULT_PARAM('..i..', OFFSET)\n')
	end
	f:write('\n')

	generic_write([[
#define OOLUA_FUNCTION_PARAMS_TYPES_NUM(NUM) P##NUM##_::type
]], 0, options.cpp_params, 'OOLUA_FUNCTION_PARAMS_TYPES_',',')

	local max = function(a,b) return a>b and a or b end

	generic_write([[
#define OOLUA_COMMA_SEPERATED_TYPES_NUM(NUM) P##NUM
]], 0, max(options.constructor_params,options.cpp_params), 'OOLUA_COMMA_SEPERATED_TYPES_',',')

	generic_write([[
#define OOLUA_COMMA_PREFIXED_TYPENAMES_NUM(NUM), typename P##NUM
]], 0, max(options.lua_params, max(options.constructor_params,options.cpp_params) ),'OOLUA_COMMA_PREFIXED_TYPENAMES_')

	generic_write([[
#define OOLUA_COMMA_PREFIXED_PARAM_TYPES_NUM(NUM), P##NUM##_
]], 0, options.cpp_params, 'OOLUA_COMMA_PREFIXED_PARAM_TYPES_')

	generic_write([[
#define OOLUA_CALL_PARAMS_NUM(NUM) , p##NUM
]], 0, options.cpp_params, 'OOLUA_CALL_PARAMS_')

	generic_write([[
#define OOLUA_CONVERTER_NUM(NUM) OOLUA::INTERNAL::Converter<typename P##NUM::pull_type, typename P##NUM::type> p##NUM##_(p##NUM);
]],0,options.cpp_params,'OOLUA_CONVERTER_')

	generic_write([[
#define OOLUA_CONVERTER_PARAMS_NUM(NUM) p##NUM##_
]],0,max(options.constructor_params,options.cpp_params),'OOLUA_CONVERTER_PARAMS_',',')

	generic_write([[
#define OOLUA_PULL_TYPE_PARAMS_NUM(NUM) , typename P##NUM ::pull_type& p##NUM
]],0,options.cpp_params,'OOLUA_PULL_TYPE_PARAMS_')

	generic_write([[
#define OOLUA_CONSTRUCTOR_PARAM_NUM(NUM) \
	typename P##NUM::pull_type p##NUM; \
	Proxy_stack_helper<typename P##NUM::traits, P##NUM::owner>::get(index, vm, p##NUM); \
	Converter<typename P##NUM::pull_type, typename P##NUM::type> p##NUM##_(p##NUM);
]],1,options.constructor_params,'OOLUA_CONSTRUCTOR_PARAM_')

	generic_write([[
#define OOLUA_CONSTRUCTOR_PARAM_IS_OF_TYPE_NUM(NUM) Param_helper<P##NUM >::param_is_of_type(vm, index)
]],1,options.constructor_params,'OOLUA_CONSTRUCTOR_PARAM_IS_OF_TYPE_','&&')

	f:write([[
#define VA_PARAM_TYPES_NUM(NUM, ...) , INTERNAL::param_type< TYPELIST::At_default< Type_list<__VA_ARGS__ >::type, NUM-1>::Result  >
#define VA_1(...) VA_PARAM_TYPES_NUM(1, __VA_ARGS__)
]])
	for i = 2, options.constructor_params do
		f:write('#define VA_' ..i.. '(...) VA_' ..(i-1)..'(__VA_ARGS__) VA_PARAM_TYPES_NUM('..i..', __VA_ARGS__)\n')
	end
	f:write('\n')

	generic_write([[
#define OOLUA_FCALL_PARAM_NUM(NUM) , P##NUM p##NUM
]],0,options.lua_params,'OOLUA_FCALL_PARAM_')

	generic_write([[
#define OOLUA_FCALL_PUSH_NUM(NUM) && push(m_lua, p##NUM)
]],0,options.lua_params,'OOLUA_FCALL_PUSH_')

	generic_generate('OOLUA_','INTERNAL_CONSTRUCTORS_GEN ','CONSTRUCTOR_GENERATE_NUM',1,options.constructor_params)
	generic_generate('OOLUA_INTERNAL_FUNC_','OPERATORS','OPERATOR',0,options.lua_params)
	generic_generate('OOLUA_INTERNAL_C_CALLER_NO_RETURN_','FUNCS','NUM',0,options.cpp_params)
	generic_generate('OOLUA_INTERNAL_C_CALLER_','FUNCS','NUM',0,options.cpp_params)
	generic_generate('OOLUA_INTERNAL_MEMBER_CALLER_NO_RETURN_','FUNCS','NUM',0,options.cpp_params)
	generic_generate('OOLUA_INTERNAL_MEMBER_CALLER_','FUNCS','NUM',0,options.cpp_params)
	generic_generate('OOLUA_GENERATE_DEFAULT_TRAIT_','CALLERS','CALLER',0,options.cpp_params)

	file_close(f,'/** \\endcond */')
end


--[[

--]]

local gen_exports = function(options,path)

	local f = file_create({class_functions=options.class_functions},path,'proxy_function_exports.h')
	f:write('#define LUA_MEMBER_FUNC_1(Class, func1) {#func1, &Class::func1},\n')
	local previous = 'Class, func1'
	for i = 2, options.class_functions do
		local now = previous .. ', func' .. i
		f:write('#define LUA_MEMBER_FUNC_'..i..'('..now..') LUA_MEMBER_FUNC_'..(i-1)..'('..previous ..')  LUA_MEMBER_FUNC_1(Class, func'..i..')\n')
		previous = now
	end
	f:write('\n')
	f:write([[
/// @def end the assigning of functions to the array
#define CLASS_LIST_MEMBERS_END {0, 0}};}

/// @def define the constants in the class, which are the the class name and the member function array
#define CLASS_LIST_MEMBERS_START_OOLUA_NON_CONST(Class)\
namespace OOLUA { \
char const OOLUA::Proxy_class< Class >::class_name[] = #Class;\
OOLUA::Proxy_class< Class >::Reg_type OOLUA::Proxy_class< Class >::class_methods[]={
// NOLINT

#define CLASS_LIST_MEMBERS_START_OOLUA_CONST(Class)\
namespace OOLUA { \
char const OOLUA::Proxy_class< Class >::class_name_const[] = #Class "_const";\
OOLUA::Proxy_class< Class >::Reg_type_const OOLUA::Proxy_class< Class >::class_methods_const[]={
// NOLINT

]])

	local params = ''
	for i = 0, options.class_functions do
		f:write('#define EXPORT_OOLUA_FUNCTIONS_'..i ..'_(mod, Class' .. params ..')\\\n')
		f:write('\tCLASS_LIST_MEMBERS_START_ ##mod(Class)\\\n')
		if params ~= '' then f:write('\tLUA_MEMBER_FUNC_'..i..'(OOLUA::Proxy_class< Class > '.. params .. ')\\\n') end
		f:write('\tCLASS_LIST_MEMBERS_END\n')
		params = params .. ', p'..i
	end
	f:write('\n')

	params = ''
	for i = 0, options.class_functions do
		f:write('#define EXPORT_OOLUA_FUNCTIONS_' ..i ..'_CONST(Class'..params ..') \\\n\tEXPORT_OOLUA_FUNCTIONS_'..i..'_(OOLUA_CONST, Class'..params ..')\n' )
		f:write('#define EXPORT_OOLUA_FUNCTIONS_' ..i ..'_NON_CONST(Class'..params ..') \\\n\tEXPORT_OOLUA_FUNCTIONS_'..i..'_(OOLUA_NON_CONST, Class'..params ..')\n' )
		params = params .. ', p'..i
	end

	f:write([[
/** \endcond */

/** \addtogroup OOLuaExporting
@{
	\def OOLUA_EXPORT_NO_FUNCTIONS
	\hideinitializer
	\brief Inform that there are no functions of interest
	\param Class
*/
#define OOLUA_EXPORT_NO_FUNCTIONS(Class)\
	EXPORT_OOLUA_FUNCTIONS_0_NON_CONST(Class)\
	EXPORT_OOLUA_FUNCTIONS_0_CONST(Class)

/**@}*/
]])
	file_close(f)
end



local gen = function(options,path)
	if not options then options = defaults()
	else
		local _defaults = defaults()
		for k,v in pairs(_defaults) do
			if options[k] then _defaults[k]=tonumber(options[k]) end
		end
		options = _defaults
	end
	gen_boilerplate(options,path)
	gen_exports(options,path)
end


--/*[GenModuleReturn]*/
return { gen = gen, defaults=defaults, default_details=default_details }
--/*[GenModuleReturn]*/
