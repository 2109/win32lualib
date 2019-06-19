
--[[
/** [LuaComparisonFile]*/
]]
local clock = os.clock
local N = 10
local times = 1000000

return
{
	vfunc_self = function(object) 
		local ave = 0
		if not object.virtual_func then return -1 end
		for i = 0, N do
			local t0 = clock()
			for i=1,times do
				object:virtual_func()
			end
			local dt = clock()-t0
			if i~=0 then
				ave = ave + dt
			end
		end
		return (ave/N)/times
	end

	,vfunc_cached = function(object) 
		local ave = 0
		if not object.virtual_func then return -1 end
		for i = 0, N do
			local cached_vfunc = object.virtual_func
			local t0 = clock()
			for i=1,times do
				cached_vfunc(object)
			end
			local dt = clock()-t0
			if i~=0 then
			 ave = ave + dt
			end
		end
		return (ave/N)/times
	end

	,mfunc_self = function(object)
		local ave = 0
		for i = 0, N do
			local t0 = clock()
			for i=1,times do
				object:get()
			end
			local dt = clock()-t0
			if i~=0 then
			 ave = ave + dt
			end
		end
		return (ave/N)/times
	end
	--/**[mfuncCachedExample]*/
	,mfunc_cached = function(object)
		local ave = 0
		local func = object.get
		for i = 0, N do
			local t0 = clock()
			for i=1,times do
				func(object)
			end
			local dt = clock()-t0
			if i~=0 then
		 	ave = ave + dt
			end
		end
		return (ave/N)/times
	end
	--/**[mfuncCachedExample]*/
	
	,increment_a_base_self = function(object,param)
		local ave = 0
		for i = 0, N do
			local t0 = clock()
			for i=1,times do
				object:increment_a_base(param)
			end
			local dt = clock()-t0
			if i~=0 then
			 ave = ave + dt
			end
		end
		return (ave/N)/times
	end

	,increment_a_base_cached = function(object,param)
		local ave = 0
		local func = object.increment_a_base
		for i = 0, N do
			local t0 = clock()
			for i=1,times do
				func(object,param)
			end
			local dt = clock()-t0
			if i~=0 then
		 	ave = ave + dt
			end
		end
		return (ave/N)/times
	end
}

--[[
/** [LuaComparisonFile]*/
]]
