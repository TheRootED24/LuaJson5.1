-- TURN set SUB/UNSUB DEBUG = 1 AND RECOMPILE TO GET MORE DETAILED MESSAGES --

local JSON = require "JSON"

local red = "\27[31m"
local green = "\27[32m"
local yellow ="\27[33m"
local blue ="\27[34m"
local violet = "\27[35m"
local cyan = "\27[36m"
local reset = "\27[0m"

local color = function(text, color) return string.format("%s %s %s", color, text, reset) end

-- create object o
print(color('----- create an object with some data"-----', violet))
local o = JSON:object("test","me", "age", 99, "root", true)
print(o:tojson(), "length ".. o:len())
-- create array a
print(color('\n----- create an array with some data"-----', blue))
local a = JSON:array(1,2,3,"test",true)
print(a:tojson(), "length: "..a:len().."\n")

-- sub o to a 
print(color('\n----- add a to o --> o.arr = a -----', cyan))
o.arr = a
print(o:tojson(), "length: "..o:len())

print(color('\n----- add new string to a, then recheck that o updated its length --> a[#a] = "some string"-----', cyan))
a[#a]="some string"
print(o:tojson(), "length: "..o:len())
-- unsub o from a
print(color('\n----- remove a from o --> a.arr = nil -----', cyan))
o.arr = nil
print(o:tojson(), "length: "..o:len())

-- sub a to o 
print(color('\n----- add o to a --> a[#a] = o -----', cyan))
a[#a] = o
print(a:tojson(), "length: "..a:len())

-- unsub a from o
print(color('\n----- remove o from a --> a[#a - 1] = nil -----', cyan))
a[#a-1] = nil
print(a:tojson(), "length: "..a:len())