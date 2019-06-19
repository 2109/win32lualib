local event = require "event"
local readline = require "readline"

local _M = {}

function _M.quit()
	os.exit(0)
end

_M.q = _M.quit

function _M.test(channelObj,...)
	table.print(channelObj:call(...))
end

function _M.sync_time(channelObj)
	-- for i = 1,1 do
	-- 	local now = event.now()
	-- 	local time = channelObj:call("sync_time","sync_time",now)
	-- 	print(time - now)
	-- 	event.sleep(1)
	-- end

	local now = event.now()
	for i = 1, 10 do
		event.sys_sleep(2)
		local time = event.now()
		print(time  - now)
		now = time
	end
end

local ip,port = ...

local client_channel = event.channel:inherit()

function client_channel:dispatch(file,method,...)
	print(file,method,...)
end

event.fork(function ()

	local channelObj,err = event.connect(ip,tonumber(port),client_channel)
	if not channelObj then
		print(err)
		os.exit(1)
	end

	while true do
		local ok,err = xpcall(readline,debug.traceback,">>",nil,_M,channelObj)
		if not ok then
			print(err)
		end
	end
end)

event.dispatch()

