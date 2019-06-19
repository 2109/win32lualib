local event = require "event"

local client_channel = event.channel:inherit()

function client_channel:dispatch(file,method,...)
	print(file,method,...)
end

event.fork(function ()

	local channelObj,err = event.connect("127.0.0.1",1989,client_channel)
	if not channelObj then
		print(err)
		os.exit(1)
	end

	
end)

event.dispatch()