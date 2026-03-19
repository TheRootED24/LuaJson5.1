local JSON = require "JSON"

local o = JSON:object("test", "me", "age", 99);
local a = JSON:array(1,2,3,"test",true, null)

o.arr = a--:unref()
a[#a] = o:unref()

print("object to json .............")
print(o:tojson())
print("o len: ", o:len())
print()
print("array to json .............")
print(a:tojson())
print("a len: ", a:len())
print("o len: ", o:len())
a[#a]="some long string"

a[#a]=JSON:array(1,2,3,4,5)
--a[#a-1].test="new"
o.arr=nil
--print(o:tojson())
print("a len: ", a:len(), #a)
print(o:tojson())
print(a:tojson())

