local compare = require'compare'

	local _factory =
	{
		OOLua = function() return Set_get.new() end
		,SWIG = function() return swig_profile.Set_get() end
		,LuaBind = function() return Set_get() end
		,SLB3 = function() return  Set_get() end
		,LuaBridge = function() return Set_get() end
	}
local do_mfunc = function(name)
	local factory =
	{
		OOLua = function() return Set_get.new() end
		,SWIG = function() return swig_profile.Set_get() end
		,LuaBind = function() return Set_get() end
		,SLB3 = function() return  Set_get() end
		,LuaBridge = function() return Set_get() end
	}

	local self, cached = 'unavailable','unavailable'

	local creator = factory[name]
	if creator then 
		cached = compare.mfunc_cached( creator() )
		self = compare.mfunc_self( creator() )
	end
	print("| | | |")
	print("| " .. name .. " mfunc | " .. cached .. " | " .. self.. " |")
	return {name,'mfunc',cached,self}
end


local do_vfunc = function(name)
	local factory =
	{
		OOLua = function() return ProfileMultiBases.new() end
		,SWIG = function() return swig_profile.ProfileMultiBases() end
		,LuaBind = function() return ProfileMultiBases() end
		,SLB3 = function() return  ProfileMultiBases() end
	}
	local self, cached = 'unavailable','unavailable'

	local creator = factory[name]
	if creator then 
		cached = compare.vfunc_cached( creator() )
		self = compare.vfunc_self( creator() )
	end
	print("| " .. name .. " vfunc | " .. cached .. " | " .. self .. " |")
	return {name,'vfunc',cached,self}
end

vfunc = function(name) 
	do_vfunc(name) 
end

mfunc = function(name) 
	do_mfunc(name) 
end


class_param = function(name)
	local factory =
	{
		OOLua = function() return ProfileDerived.new() end
		,SWIG = function() return swig_profile.ProfileDerived() end
		,LuaBind = function() return ProfileDerived() end
		,SLB3 = function() return  ProfileDerived() end
		,LuaBridge = function() return ProfileDerived() end
	}
	local self, cached = 'unavailable','unavailable'
	local creator = factory[name]
	if creator then
		cached = compare.increment_a_base_cached(creator(),creator())
		self = compare.increment_a_base_self(creator(),creator())
	end
	print("| " .. name .. " class param | ".. cached .." | " .. self .. " |")
	return {name,'class param',cached,self}
end
