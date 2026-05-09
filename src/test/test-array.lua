local JSON = require "JSON"



function elm_copy(a)

    local copy = a:unref()
    --a:unref();
    print(copy, a)
end



print("Create an array ..\n");



local a = JSON:array(1,2,3,4,"test",true,null);

print(a ,"\n");
print("Array: lua ipairs loop ..\n")
for i,v in ipairs(a.env) do
    print(i,v);
end
print(string.format("Array Size: %d", #a))

print(string.format("\nArray: stringify as json --> %s\n",  a:tojson()))

print(string.format("Array Json Render Length: %d", a:len()))

print(string.format("\nArray: stringify as escaped json --> %s\n",  a:tojson(true)))

print(string.format("Array Esc Json Render Length: %d", a:rlen(0, true)))

print(string.format("\nArray: stringify as lua --> %s\n",  a:tolua()))
print(a)
print(string.format("Array Json Render Length: %d", a:rlen(1, false)))

print(string.format("\nArray: stringify as escaped json --> %s\n",  a:tolua(true)))

print(string.format("Array Esc Json Render Length: %d", a:rlen(1, true)))

print(string.format("call function with a and create local copy ..\n"))
elm_copy(a)


print("Create an Object ..");

local o = JSON:object("test","object", "age",99, "isRoot",false, "nully",null);

print(o:tojson());

arr = JSON:parse('[1,2,3,4,5]')
arr[#arr] = o
print(arr:tojson())
arr=nil

arr = JSON:parse([[
	{
		"ip":"192.168.1.1",
		"name":"teds",
		"expires":19087,
		"hobbies":[
					"fish", 
					"hunt", 
					"run"
		]
	}
]])

print(arr.hobbies[0])