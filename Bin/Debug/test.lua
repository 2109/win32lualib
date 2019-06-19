require "login"
local common = require "common"

local pto = pto.Create()

pto:Import(1, "login", s2c_login_role_list)

for i = 1, 1 do
	local test = {
		PlayerInfoList = {},
		LastLoginUid = 1,
	}

	for i = 1,3 do
		local info = {Uid = i, Name = "mrq@"..i, Level = 50 + i,Job = 100 + i,ShapeId = 0,CreateTime = os.time(),Attach = {}}

		for i = 1, 10 do
			table.insert(info.Attach, {Key = i, Value = i})
		end

		table.insert(test.PlayerInfoList, info)
	end

	local data = pto:Encode(1, test)
	local tbl = pto:Decode(1, data)

	common.Dump(tbl)
end