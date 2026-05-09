local JSON = require "JSON"

local red = "\27[31m"
local green = "\27[32m"
local yellow ="\27[33m"
local blue ="\27[34m"
local violet = "\27[35m"
local cyan = "\27[36m"
local reset = "\27[0m"


local tests_passed = 0
local tests_total = 0

local color = function(text, color) return string.format("%s %s %s", color, text, reset) end

function assert_equal(a, b, msg)
    tests_total = tests_total + 1
    if a == b then
        tests_passed = tests_passed + 1
        print(color(" PASS: ", green) .. msg)
    else
        print(color(" FAIL: ", red) .. msg .. " (expected " .. tostring(b) .. ", got " .. tostring(a) .. ")")
    end
end

function assert_json_equal(a, b, msg)
    local json_a = a:tojson()
    local json_b = b:tojson()
    assert_equal(json_a, json_b, msg)
end

o = JSON:object("name", "teddy", "age", 53, "foreman", false, "hours", 23.5);

print(color(string.format("Create an Object ... \n%s --> %s\n", tostring(o), (o:tojson())),red))
 --> object: 0x59772d2f1eb0 --> {"name":"teddy","age":53,"foreman":false,"hours":23.5}

print("----------------------------------------------------- object reverse -------------------------------------------------------------------------\n")
print(string.format('Reverse "age" and "hours" using keys --> o:reverse("age", "hours")'))
o:reverse("age", "hours")
res = o:tojson();
expected = '{"name":"teddy","hours":23.5,"foreman":false,"age":53}'
assert_equal(res, expected, 'Reverse "age" and "hours" using keys --> o:reverse("age", "hours")')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

print(string.format('Reverse "name" and "hours" using "names" index --> o:reverse(0, "hours")'));
o:reverse(0, "hours");
res = o:tojson();
expected = '{"hours":23.5,"name":"teddy","foreman":false,"age":53}'
assert_equal(res, expected, 'Reverse "name" and "hours" using "names" index --> o:reverse(0, "hours")')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

print(string.format('Reverse object from 0 to 2 --> o:reverse(0, 2)'));
o:reverse(0, 2);
res = o:tojson();
expected = '{"foreman":false,"name":"teddy","hours":23.5,"age":53}'
assert_equal(res, expected, 'Reverse object from 0 to 2 --> o:reverse(0, 2)')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

print(string.format('Reverse using o.env (1 based index) ... \n'));

print(string.format('Reverse object from 1 to 4 --> object.env:reverse(1,4)'));
o.env:reverse(1,4);
res = o.env:tojson();
expected = '{"age":53,"hours":23.5,"name":"teddy","foreman":false}'
assert_equal(res, expected, 'Reverse object from 1 to 4 --> object.env:reverse(1,4)')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

print(string.format('Reverse object from 2 and 3 --> object.env:reverse(2,3)'));
o.env:reverse(2,3);
res = o.env:tojson();
expected = '{"age":53,"name":"teddy","hours":23.5,"foreman":false}'
assert_equal(res, expected, 'Reverse object from 2 and 3 --> object.env:reverse(2,3)')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

print(string.format('Reverse object from "age" to "hours"  --> object.env:reverse("age","hours")'));
o.env:reverse("age","hours");
res = o.env:tojson();
expected = '{"hours":23.5,"name":"teddy","age":53,"foreman":false}'
assert_equal(res, expected, 'Reverse object from "age" to "hours"  --> object.env:reverse("age","hours")')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

o2 = JSON:object("obby", "data", "nully", null, "arr", JSON:array(1,2,3,4,5))
o.obj=o2
o.obj = nil

o.obj=o2
print(color(o:tojson(), green))


print("----------------------------------------------------- move -------------------------------------------------------------------------")
o = JSON:object("name", "teddy", "age", 53, "foreman", false, "hours", 23.5);
o:move(2,0)
res = o:tojson()
expected = '{"foreman":false,"name":"teddy","age":53,"hours":23.5}' 
assert_equal(res, expected, 'Move idx 2 -->"foreman"  to idx0 --> "name"  --> object:move(2,0)')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

o.env:move(1,3)
res = o:tojson()
expected = '{"name":"teddy","age":53,"foreman":false,"hours":23.5}' 
assert_equal(res, expected, 'Move "foreman"  back to idx 3 using env (1 based ) --> object.env:move(1,3)')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

o:move("hours", "age")
res = o:tojson()
expected = '{"name":"teddy","hours":23.5,"age":53,"foreman":false}' 
assert_equal(res, expected, 'Move idx 3 -->"hours" to idx1 --> "age" using keys --> object:move("hours","age")')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

o.env:move("foreman", "hours")
res = o:tojson()
expected = '{"name":"teddy","foreman":false,"hours":23.5,"age":53}' 
assert_equal(res, expected, 'Move idx 3 -->"foreman" to idx1 --> "hours" using keys --> object:move("foreman","hours")')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

print("----------------------------------------------------- insert -------------------------------------------------------------------------")

o = JSON:object("name", "teddy", "age", 53, "foreman", false, "hours", 23.5);

o:insert(1, "new", "val")
res = o:tojson()
expected = '{"name":"teddy","new":"val","age":53,"foreman":false,"hours":23.5}' 
assert_equal(res, expected, 'Insert "new", "val" at idx 1 -->  object:insert(1, "new", "val")')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

o.env:insert(4, "new2", "val2")
res = o:tojson()
expected = '{"name":"teddy","new":"val","age":53,"new2":"val2","foreman":false,"hours":23.5}' 
assert_equal(res, expected, 'Insert "new2", "val2" at idx 4 using env (1 based)" -->  --> object.env:insert(4, "new2", "val2")')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

o:insert("age", "time", 10345)
res = o:tojson()
expected = '{"name":"teddy","new":"val","time":10345,"age":53,"new2":"val2","foreman":false,"hours":23.5}' 
assert_equal(res, expected, 'Insert "time", 10345 at "age" -->  object:insert("age", "time", 10345)')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())));

o.env:insert("time", "root", false)
res = o:tojson()
expected = '{"name":"teddy","new":"val","root":false,"time":10345,"age":53,"new2":"val2","foreman":false,"hours":23.5}' 
assert_equal(res, expected, 'Insert "root", false at "time" using env (1 based) -->  object.env:insert("time", "root", false)')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())))

print("----------------------------------------------------- pop -------------------------------------------------------------------------")

p = o:pop()
res = o:tojson()
expected = 23.5 
assert_equal(p, expected, 'Pop "hours" into p -->  p = object:pop()')
expected = '{"name":"teddy","new":"val","root":false,"time":10345,"age":53,"new2":"val2","foreman":false}'
assert_equal(res, expected, 'Pop "hours" into p -->  p = object:pop()')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())))

p = o:pop()
res = o:tojson()
expected = false 
assert_equal(p, expected, 'Pop "foreman" into p -->  p = object:pop()')
expected = '{"name":"teddy","new":"val","root":false,"time":10345,"age":53,"new2":"val2"}'
assert_equal(res, expected, 'Pop "hours" into p -->  p = object:pop()')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())))

print("----------------------------------------------------- push -------------------------------------------------------------------------")

o = JSON:object("name", "teddy", "age", 53, "foreman", false, "hours", 23.5);

o:push("arr", JSON:array(1,2,3,4,5))
res = o:tojson()
expected = '{"name":"teddy","age":53,"foreman":false,"hours":23.5,"arr":[1,2,3,4,5]}' 
assert_equal(res, expected, 'Push an array --> object:push("arr", JSON:array(1,2,3,4,5))')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())))

o.env:push("obj", JSON:object("testy", "metoo", "date", "05-09-26"))
res = o:tojson()
expected = '{"name":"teddy","age":53,"foreman":false,"hours":23.5,"arr":[1,2,3,4,5],"obj":{"testy":"metoo","date":"05-09-26"}}' 
assert_equal(res, expected, 'Push an object using env --> object.env:push("obj", JSON:object("testy", "metoo", "date", "05-09-26"))')
print(string.format("%s --> %s\n", tostring(o), (o:tojson())))

print(string.format("%s\t%s\t%s", color("This is an ERROR !!!", red), color("This is a WARNING !!", yellow), color("This is all GOOD !", violet)))

print("----------------------------------------------------- shift -------------------------------------------------------------------------")
o = JSON:object("name", "teddy", "age", 53, "foreman", false, "hours", 23.5);

n = o:shift()
res = o:tojson()
expected = '{"age":53,"foreman":false,"hours":23.5}'
assert_equal(n, "teddy", 'Shift name into n --> n = object:shift()')
assert_equal(res, expected, 'Shift name into n --> n = object:shift()')

a = o.env:shift()
res = o:tojson()
expected = '{"foreman":false,"hours":23.5}'
assert_equal(a, 53, 'Shift age into a using env (1 based) --> a = object.env:shift()')
assert_equal(res, expected, 'Shift age into a using env (1 based) --> a = object.env:shift()')

print("----------------------------------------------------- unshift -------------------------------------------------------------------------")

o:unshift("name", "teddy", "age", 35)
res = o:tojson()
expected = '{"name":"teddy","age":35,"foreman":false,"hours":23.5}'
assert_equal(res, expected, 'unshift name and age back onto object --> object:unshift("name", "teddy", "age", 35)')

o.env:unshift("time", 10345, "root", true)
res = o:tojson()
expected = '{"time":10345,"root":true,"name":"teddy","age":35,"foreman":false,"hours":23.5}'
assert_equal(res, expected, 'unshift time and root onto object ussing env (1 based) --> object.env:unshift("time", 10345, "root", true)')

print("============================")
print("Tests completed: " .. tests_passed .. "/" .. tests_total)
if tests_passed == tests_total then
    print("All tests PASSED! ✅")
else
    print("Some tests FAILED! ❌")
end