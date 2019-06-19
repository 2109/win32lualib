local linenoise = require "linenoise"

local function split( str,reps )
    local result = {}
    string.gsub(str,'[^'..reps..']+',function ( w )
        table.insert(result,w)
    end)
    return result
end

return function (prompt,history,list,...)

    local line = linenoise(prompt or ">>",history,function (str)
        local matchs = {}
        for word,func in pairs(list) do
            if  word:find(str) == 1 then
                table.insert(matchs, word)
            end
        end
        return matchs
    end)

    if not line then
        return
    end

    local word_list = {}
    for word in string.gmatch(line,"%S+") do
        table.insert(word_list,word)
    end

    local name = word_list[1]
    if list[name] then
        list[name](...,table.unpack(word_list,2))
        return
    end

    return line
end
