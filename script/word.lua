local event = require "event"
local readline = require "readline"
local trie_core = require "trie"


local FILE = io.open("word.txt","r")
local content = FILE:read("*a")
FILE:close()

local data = load(content)()

local trie = trie_core.create()
for _,w in pairs(data.ForBiddenCharInName) do
	trie:add(w)
end

for _,w in pairs(data.Forbidden) do
	trie:add(w)
end

local _M = {}

function _M.q()
	os.exit(0)
end

_M.quit = _M.q

function _M.add(_,word)
	trie:add(word)
end

function _M.del(_,word)
	trie:delete(word)
end

function _M.dump()
	trie:dump("./dump.txt")
end

function _M.search(_,word)
	local list = trie:search(word)
	for _,w in pairs(list) do
		print(w)
	end
end

collectgarbage("collect")
while true do
	local line = readline(">>",nil,_M)
	if line then
		print(trie:filter(string.lower(line)))
	end
end