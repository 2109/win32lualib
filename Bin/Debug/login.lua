

AttachInfo = {
	{
		type = pto.INT,
		array = false,
		name = "Key"
	},
	{
		type = pto.INT,
		array = true,
		name = "Value"
	},
}


LoginPlayerInfo = {
	{
		type = pto.INT,
		array = false,
		name = "Uid"
	},
	{
		type = pto.STRING,
		array = false,
		name = "Name"
	},
	{
		type = pto.INT,
		array = false,
		name = "Level"
	},
	{
		type = pto.INT,
		array = false,
		name = "Job"
	},
	{
		type = pto.INT,
		array = false,
		name = "ShapeId"
	},
	{
		type = pto.PROTOCOL,
		array = true,
		name = "Attach",
		pto = AttachInfo,
	},
	{
		type = pto.INT,
		array = false,
		name = "CreateTime"
	}
}

s2c_login_role_list = {
	{
		type = pto.PROTOCOL,
		array = true,
		name = "PlayerInfoList",
		pto = LoginPlayerInfo,
	},
	{
		type = pto.INT,
		array = false,
		name = "LastLoginUid",
	}
}
