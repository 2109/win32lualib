local readline = require "readline"
local event = require "event"



local bash_channel = event.channel:inherit()

function bash_channel:data()
	print(self:read())
	local line = self:read_line()
	if line then
		print(line)
	end
end

local proxy_channel = event.channel:inherit()

function proxy_channel:data()
	print(self:read())
	local line = self:read_line()
	if line then
		print(line)
	end
end

event.fork(function ()
	local channelObj,err = event.connect("192.168.100.55",1989,bash_channel)
	if not channelObj then
		print(err)
		os.exit(1)
	end
	channelObj:write("ls -l\r\n")
end)

event.dispatch()