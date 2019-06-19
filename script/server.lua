local readline = require "readline"
local event = require "event"


local ip,port = ...

local server_channel = event.channel:inherit()

function server_channel:dispatch(file,method,...)
	local M = require(file)
	assert(M ~= nil,string.format("no such file:%s",file))
	local func = M[method]
	assert(M ~= nil,string.format("no such method:%s in %s",method,file))
	return func(...)
end

local ok = event.listen(ip,tonumber(port),function (_,_,ip,port)
	print(string.format("accept client from:%s:%d",ip,port))
end,server_channel)

if ok then
	print(string.format("listen client %s:%s ok",ip,port))
else
	print(string.format("listen client %s:%s fail",ip,port))
	os.exit(1)
end

event.dispatch()