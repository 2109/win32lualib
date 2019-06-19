local _M = {}

local function GetTag( t )
    local str = type(t)
    return string.sub(str, 1, 1)..":"
end

function _M.Dump(data, prefix, depth, output, record)
    record = record or {}

    depth = depth or 1

    if output == nil then
        output = _G.print
    end

    local tab = string.rep("\t", depth)
    if prefix ~= nil then
        tab = prefix .. tab
    end

    if data == nil then
        output(tab.." nil")
        return
    end

    if type(data) ~= "table" then
        output(tab..GetTag(data)..tostring(data))
        return
    end

    if record[data] then
        output(tab.." {}")
	   return
    end
    --assert(record[data] == nil)
    record[data] = true

    local count = 0
    for k,v in pairs(data) do
        local str_k = GetTag(k)
        if type(v) == "table" then
            output(tab..str_k..tostring(k).." -> ")
            _M.Dump(v, prefix, depth + 1, output, record)
        elseif k ~= "_id" then
            output(tab..str_k..tostring(k).." -> ".. GetTag(v)..tostring(v))
        end
        count = count + 1
    end

    if count == 0 then
        output(tab.." {}")
    end
end

return _M