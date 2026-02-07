local JSON = require "JSON"

local a = JSON:array(1,2,3)
print(a:tojson())
a:insert(1,9)
print(a:tojson())
a:unshift(9,8,7)
print(a:tojson())

--o = JSON:object("some", "obj", "arr", JSON:array(1,2,3,4), "another", "val", "new_obj", JSON:object("this", "is", "cool", true, "nully", null))
o = JSON:object("some", "obj")
o.obj= JSON:object("new","obby")
o.obj2= JSON:object("neew","obb2y")
o.arr = JSON:array(1,2,3,4)
print(o:tojson())
print(o:len())
o.obj=nil
print(o:len())
print(#o)
print(o:tojson())
print(o:get_root())
print(o.obj2:get_root())
