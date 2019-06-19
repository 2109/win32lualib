local event_core = require "event.core"
local serialize = require "serialize"

table.encode = table.encode or serialize.pack
table.decode = table.decode or serialize.unpack

local _event

local _co_pool = {}
local _fork_queue = {}
local _wakeup_queue = {}
local _wait_co = {}

local _session = 1
local _main_co = coroutine.running()

local EV_ERROR = 0
local EV_TIMEOUT = 1
local EV_ACCEPT = 2
local EV_CONNECT = 3
local EV_DATA = 4
local EV_HTTP = 5
local EV_DNS = 6

local _listener_ctx = setmetatable({},{__mode = "k"})
local _channel_ctx = setmetatable({},{__mode = "k"})
local _timer_ctx = setmetatable({},{__mode = "k"})
local _httpd_ctx = setmetatable({},{__mode = "k"})
local _dns_ctx = setmetatable({},{__mode = "k"})

local _M = {}


_M.CO_STATE = {
	EXIT = 1,
	WAIT = 2	
}

local STATE = {HEAD = 1,BODY = 2}

local channel = {}

function channel:inherit()
	local children = setmetatable({},{__index = self})
	return children
end

function channel:new(channel_buff,ip,port)
	local ctx = setmetatable({},{__index = self})
	ctx.channel_buff = channel_buff
	ctx.ip = ip or "unknown"
	ctx.port = port or "unknown"
	ctx.state = STATE.HEAD
	ctx.head = 2
	ctx.need = ctx.head
	ctx.session_ctx = {}
	return ctx
end

function channel:init(...)

end

function channel:disconnect()
	local list = {}
	for session in pairs(self.session_ctx) do
		table.insert(list,session)
	end

	table.sort(list,function (l,r)
		return l < r
	end)

	for _,session in pairs(list) do
		_M.wakeup(session,false,"channel_buff closed")
	end
	self.session_ctx = {}
end

function channel:read(num)
	return self.channel_buff:read(num)
end

function channel:read_line()
	return self.channel_buff:read_line()
end

function channel:dispatch(file,method,...)
	print(file,method,...)
end

function channel:data()
	while true do
		if self.state == STATE.HEAD then
			local data = self:read(self.need)
			if data then
				local patt = string.format("I%d",self.head)
				self.need = string.unpack(patt,data)
				self.need = self.need - self.head
				self.state = STATE.BODY
			else
				break
			end
		elseif self.state == STATE.BODY then
			local data = self:read(self.need)

			if data then
				self.need = self.head
				self.state = STATE.HEAD
				local message = table.decode(data)
				if message.ret then
					_M.wakeup(message.session,message.ok,table.unpack(message.args))
					self.session_ctx[message.session] = nil
				else
					_M.fork(function ()
						local result = {xpcall(self.dispatch,debug.traceback,self,message.file,message.method,table.unpack(message.args))}
						if not result[1] then
							if message.session ~= 0 then
								self:ret(message.session,false,result[2])
							end
						else
							self:ret(message.session,true,table.unpack(result,2))
						end
					end)
				end
			else
				break
			end
		end
	end
end

local function pack_table(channel_obj,tbl)
	local str = table.encode(tbl)
	local pat = string.format("I%dc%d",channel_obj.head,str:len())
	return string.pack(pat,str:len()+channel_obj.head,str)
end

function channel:write(str)
	--FIXME
	self.channel_buff:write(str)
end

function channel:send(file,method,...)
	local str = pack_table(self,{file = file,method = method,session = 0,args = {...}})
	self:write(str)
end

function channel:call(file,method,...)
	local session = _M.gen_session()
	self.session_ctx[session] = true
	local str = pack_table(self,{file = file,method = method,session = session,args = {...}})

	self:write(str)

	local result = {_M.wait(session)}

	if not result[1] then
		error(result[2])
	end
	return table.unpack(result,2)
end

function channel:ret(session,ok,...)
	local str = pack_table(self,{ret = true,ok = ok,session = session,args = {...}})
	self:write(str)
end

function channel:close()
	self.channel_buff:close(0)
end

function channel:close_immediately()
	self.channel_buff:close(1)
end

_M.channel = channel

local function co_create(func)
	local co = table.remove(_co_pool)
	if co == nil then
		co = coroutine.create(function(...)
			func(...)
			while true do
				func = nil
				_co_pool[#_co_pool+1] = co
				func = coroutine.yield(_M.CO_STATE.EXIT)
				func(coroutine.yield())
			end
		end)
	else
		coroutine.resume(co, func)
	end
	return co
end

local function co_monitor(co,ok,state,session)
	if ok then
		if state == _M.CO_STATE.WAIT then
			_wait_co[session] = co
		else
			assert(state == _M.CO_STATE.EXIT)
		end
	else
		io.stderr:write(debug.traceback(co,tostring(state)))
	end
end

local function run_fork()
	for i,info in ipairs(_fork_queue) do
		_fork_queue[i] = nil
		local co = co_create(function (...)
			info.func(table.unpack(info.args))
		end)
		co_monitor(co,coroutine.resume(co))
	end
	_fork_queue = {}
end

local function run_wakeup()
	for _,info in ipairs(_wakeup_queue) do
		local co = _wait_co[info.session]
		_wait_co[info.session] = nil
		if co then
			co_monitor(co,coroutine.resume(co,table.unpack(info.args)))
		else
			print(string.format("error wakeup:session:%s not found",info.session))
		end
	end
	_wakeup_queue = {}
end

local function create_channel(channel_class,channel_buff,ip,port)
	local channel_obj = channel_class:new(channel_buff,ip,port)
	channel_obj:init()
	_channel_ctx[channel_buff] = channel_obj
	return channel_obj
end

function _M.listen(ip,port,callback,channel_class)
	local listener = _event:listen(false,{ip = ip,port = port})
	if not listener then
		return false
	end
	_listener_ctx[listener] = {callback = callback,channel_class = channel_class or channel}
	return listener
end

function _M.httpd(ip,port,callback)
	local httpd = _event:httpd(ip,port)
	if not httpd then
		return false
	end
	_httpd_ctx[httpd] = {callback = callback}
	return httpd
end

function _M.dns(host,callback)
	local dns = _event:dns(host)
	if not dns then
		return dns
	end
	_dns_ctx[dns] = callback
	return true
end

function _M.connect(ip,port,channel_class)
	local co = coroutine.running()
	assert(co ~= _main_co,string.format("cannot connect in main co"))
	local session = _M.gen_session()
	local ok,err = _event:connect(session,{ip = ip,port = port})
	if not ok then
		return false,err
	end
	local result,error_or_buffer = _M.wait(session)
	if result then
		return create_channel(channel_class or channel,error_or_buffer,ip,port)
	else
		return false,error_or_buffer
	end
end

function _M.bind(fd,channel_class)
	local channel_buff = _event:bind(fd)
	return create_channel(channel_class or channel,channel_buff)
end

function _M.sleep(ti)
	local session = _M.gen_session()
	local timer = _event:timer(ti)
	_timer_ctx[timer] = {session = session}
	_M.wait(session)
end

function _M.sys_sleep(ti)
	_event:sleep(ti)
end

function _M.timer(ti,callback)
	local timer = _event:timer(ti)
	_timer_ctx[timer] = {callback = callback}
	return timer
end

function _M.fork(func,...)
	table.insert(_fork_queue,{func = func,args = {...}})
end

function _M.wakeup(session,...)
	table.insert(_wakeup_queue,{session = session,args = {...}})
end

function _M.wait(session)
	local co = coroutine.running()
	assert(co ~= _main_co,string.format("cannot sleep in main co,wait op should run in fork"))
	return coroutine.yield(_M.CO_STATE.WAIT,session)
end

function _M.gen_session()
	if _session >= math.maxinteger then
		_session = 1
	end
	local session = _session
	_session = _session + 1
	return session
end

function _M.co_clean()
	_co_pool = {}
end

function _M.dispatch()
	run_wakeup()
	run_fork()

	local code = _event:dispatch()
	
	for timer in pairs(_timer_ctx) do
		if timer:alive() then
			timer:cancel()
		end
	end

	for listener in pairs(_listener_ctx) do
		if listener:alive() then
			listener:close()
		end
	end

	for httpd in pairs(_httpd_ctx) do
		if httpd:alive() then
			httpd:close()
		end
	end

	for channel_buff in pairs(_channel_ctx) do
		if channel_buff:alive() then
			channel_buff:close_immediately()
		end
	end
	
	_event:release()
	return code
end

function _M.breakout()
	_event:breakout()
end

function _M.error(...)
	print(...)
end

function _M.now()
	return _event:now()
end

local EV = {}

EV[EV_TIMEOUT] = function (timer)
	local info = _timer_ctx[timer]
	if info.callback then
		info.callback(timer)
	else
		timer:cancel()
		_timer_ctx[timer] = nil
		_M.wakeup(info.session)
	end
end

EV[EV_ACCEPT] = function (listener,channel_buff,ip,port)
	local info = _listener_ctx[listener]
	local channel_obj = create_channel(info.channel_class,channel_buff,ip,port)
	info.callback(listener,channel_obj,ip,port)
end

EV[EV_CONNECT] = function (...)
	_M.wakeup(...)
end

EV[EV_DATA] = function (channel_buff)
	local channel = _channel_ctx[channel_buff]
	channel:data()
end

EV[EV_HTTP] = function (httpd,...)
	local info = _httpd_ctx[httpd]
	info.callback(httpd,...)
end

EV[EV_ERROR] = function (channel_buff)
	local channel = _channel_ctx[channel_buff]
	channel:disconnect()
end

EV[EV_DNS] = function (dns,...)
	local callback = _dns_ctx[dns]
	callback(...)
end

local function event_dispatch(ev,...)
	local ev_func = EV[ev]
	if not ev_func then
		io.stderr:write(string.format("no such ev:%d",ev))
		return
	end
	local ok,err = xpcall(ev_func,debug.traceback,...)
	if not ok then
		io.stderr:write(err)
	end
	run_wakeup()
	run_fork()
end

_event = event_core.new(event_dispatch)

return _M
