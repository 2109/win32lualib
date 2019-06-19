local event = require "event"

local stu = Student.new(30,"hx")


for k,v in pairs(Student) do
	print(k,v)
end



print(stu:getAge())
print(stu:getName())
print(stu:setAge(1989))
print(stu:setName("mrq"))
print(stu:getAge())
print(stu:getName())
print("------------")

local stu0 = Student.new(123456,"mrq")
print(stu0:getAge())
print(stu0:getName())

print("!!!")

local stu1 = Student.new(stu0)
print(stu1:getAge())
print(stu1:getName())

stu1:setFriend(stu)
print("!!2222!")

local count = 1024*1024

local ti1 = event.now()
local getFriendName = Student.getFriendName
for i = 1,count do
	getFriendName(stu1)
end

local ti2 = event.now()
print(ti2-ti1)


for i = 1,count do
	get_test()
end
local ti3 = event.now()
print(ti3-ti2)
event.dispatch()