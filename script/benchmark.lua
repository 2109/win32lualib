lskiplist = require "skiplist"

function comp(a, b, scoreA, scoreB, pdiff)
	if scoreA ~= scoreB then
		return scoreA - scoreB
	end
	if a ~= b then
		return a - b
	end
	return pdiff
end

local map = {}

function benchmark(f, ...)
	local s = os.clock()
	f(...)
	return os.clock() - s
end

local sl = lskiplist.new(comp)

function insert(len)
	for i=1, len do
		sl:insert(i, map[i])
	end
end

function update(len)
	for i=1, len do
		sl:update(i, map[i])
	end
end

function rank_range(count, map_len, range)
	for j = 1, count do
		local s = math.random(1, map_len - range)
		local list = sl:rank_range(s, s + range)
	end
end

function rank_of(test_count, map_len)
	for i = 1, test_count do
		local s = math.random(1, map_len)
		sl:rank_of(s)
	end
end

function get_by_rank(test_count, map_len)
	for i = 1, test_count do
		local r = math.random(1, map_len)
		sl:get_by_rank(r)
	end
end

function delete()
	local len = #map
	for i=1, len do
		sl[i] = nil
	end
end

function printf(fmt, ...)
	print(string.format(fmt, ...))
end

function testqsort(len)
	local list = {}
	for i =1,len do
		table.insert(list,math.random(1,10000000))
		table.sort(list,function (l,r)
			return l < r
		end)
	end
end

function main(args)
	local map_len = tonumber(args and args[1]) or 1e5
	for i = 1, map_len do
		map[i] = math.random()
	end
	printf("insert, create() size=%d, time=%.5f", map_len, benchmark(insert, map_len))
	printf("update, sl:size() == %d, update cnt=%d time=%.5f", map_len, map_len, benchmark(update, map_len))
	printf("rank_range sl:size() == %d,cnt=%d(x, x+50) time=%.5f", map_len, map_len, benchmark(rank_range, map_len, map_len, 50))
	printf("rank_of sl:size() == %d,cnt=%d,time=%.5f", map_len, map_len, benchmark(rank_of, map_len, map_len))
	printf("get_by_rank sl:size() == %d,cnt=%d,time=%.5f", map_len, map_len, benchmark(get_by_rank, map_len, map_len))
	printf("delete sl:size() == %d,cnt=%d,time=%.5f", map_len, map_len, benchmark(delete))
	printf("delete sl:size() == %d,cnt=%d,time=%.5f", map_len, map_len, testqsort(map_len))
end

main(arg)
