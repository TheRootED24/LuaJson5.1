require "JSON"
 o = JSON:object("test","me","age",99,"isRoot", false)
 a = JSON:array(1,2,3,4)
 a.env[#a.env]="test"
a.env[#a.env]="t2"
print(a:tojson())

a2 = JSON:array(1,2,3,4)
a2[4]=a
print(a2:tojson())

a2[5]="rlen++"
print(a2:tojson())

o = JSON:object("test","me","age",99,"isRoot", false)

print(o:tojson())

print(o:len())

o.env.new="test"
print(o:tojson())

print(o:len())

o.arr = a2
print(o:len())



print(#a)

a.env[3]=true
print(o:tojson())