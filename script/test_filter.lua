local event = require "event"
local readline = require "readline"
local profiler = require "profiler"
local filter_core = require "filter.core"

local profiler_stop = profiler(0)
local FILE = io.open("word.txt","r")
local content = FILE:read("*a")
FILE:close()

local data = load(content,"word.txt")()

local filter = filter_core.create()
for _,w in pairs(data.ForBiddenCharInName) do
	filter:add(w)
end

local function time_diff(desc,func)
	local now = event.now()
	func()
	print(desc,event.now() - now)
end

local function filter_regex(word)
	for _, str in ipairs(data.ForBiddenCharInName) do

		if string.find(word, str, 1, true) then
			return false
		end
	end
	return true
end

local count = 1024 *3
local content = "mrqwanttofuckucaonimawowozaifuckwow"
time_diff("test1",function ()
	for i = 1,count do
		filter:filter(string.lower(content),0)
	end
end)

time_diff("test2",function ()
	for i = 1,count do
		filter_regex(string.lower(content))
	end
end)

local ti,report = profiler_stop()

for k,v in pairs(report) do
	print(k)
	for k_,v_ in pairs(v) do
		print(k_,v_)
	end
end