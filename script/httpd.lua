local event = require "event"


event.httpd("0.0.0.0",1989,function (_,response,method,path,query,header,body)
	print(method,path,query)

	for k,v in pairs(header) do
		print(k,v)
	end

	print(body)

	response:set_header("Content-Type","text/html; charset=utf-8")
	response:reply(200,"fudsfsdfsasdfck")
end)

event.dns("www.baidffu.com",function (ok,err)
	print(ok,err)
	for k,v in pairs(ok) do
		print(v)
	end
end)

print("!!!")
event.dispatch()