#!/usr/bin/env lua

-- Comprehensive test suite for LuaJson library

local JSON = require "JSON"

local tests_passed = 0
local tests_total = 0

function assert_equal(a, b, msg)
    tests_total = tests_total + 1
    if a == b then
        tests_passed = tests_passed + 1
        print("PASS: " .. msg)
    else
        print("FAIL: " .. msg .. " (expected " .. tostring(b) .. ", got " .. tostring(a) .. ")")
    end
end

function assert_json_equal(a, b, msg)
    local json_a = a:tojson()
    local json_b = b:tojson()
    assert_equal(json_a, json_b, msg)
end

print("Starting LuaJson Test Suite")
print("============================")

-- Test 1: Basic array creation
local arr = JSON:array(1, 2, 3, 4, 5)
assert_equal(arr:tojson(), "[1,2,3,4,5]", "Basic array creation")

-- Test 2: Basic object creation
local obj = JSON:object("name", "test", "value", 42)
assert_equal(obj:tojson(), '{"name":"test","value":42}', "Basic object creation")

-- Test 3: Array size
assert_equal(#arr, 5, "Array size operator")

-- Test 3: Array size
assert_equal(#obj, 2, "Object size operator")

-- Test 3: Array size
print(arr:tojson())
assert_equal(arr:len(), 12, "Array length")

-- Test 3: Array size
print(obj:tojson())
assert_equal(obj:len(), 27, "Object length")

-- Test 4: Object access
assert_equal(obj.name, "test", "Object property access")
assert_equal(obj.value, 42, "Object property access")

-- Test 5: Array indexing
assert_equal(arr[0], 1, "Array indexing")
assert_equal(arr[#arr-1], 5, "Array indexing")

-- Test 6: Array push
arr:push(6)
assert_equal(arr:tojson(), "[1,2,3,4,5,6]", "Array push")

-- Test 7: Object push
obj:push("new", "data")
assert_equal(obj:tojson(), '{"name":"test","value":42,"new":"data"}', "Object push")

-- Test 8: Array pop
local popped = arr:pop()
assert_equal(popped, 6, "Array pop return value")
assert_equal(arr:tojson(), "[1,2,3,4,5]", "Array pop")

-- Test 9: Object pop
local popped = obj:pop()
assert_equal(popped, "data", "Object pop return value")
assert_equal(obj:tojson(), '{"name":"test","value":42}', "Object pop")

-- Test 8: Array shift
local shifted = arr:shift()
assert_equal(shifted, 1, "Array shift return value")
assert_equal(arr:tojson(), "[2,3,4,5]", "Array shift")

-- Test 8: Object shift
local shifted = obj:shift()
assert_equal(shifted, "test", "Object shift return value")
assert_equal(obj:tojson(), '{"value":42}', "Object shift")

-- Test 9: Array unshift
arr:unshift(1)
assert_equal(arr:tojson(), "[1,2,3,4,5]", "Array unshift")

-- Test 10: Object unshift
obj:unshift("name", "test")
assert_equal(obj:tojson(), '{"name":"test","value":42}', "Object unshift")

-- Test 11: Array insert
arr:insert(2, "test")
assert_equal(arr:tojson(), '[1,2,"test",3,4,5]', "Array insert")

-- Test 12: Object insert
obj:insert(1, "new", true)
assert_equal(obj:tojson(), '{"name":"test","new":true,"value":42}', "Object insert")

-- Test 13: Array delete
arr:del(2)
assert_equal(arr:tojson(), "[1,2,3,4,5]", "Array delete")

-- Test 13: Object delete
obj[1] = nil
assert_equal(obj:tojson(), '{"name":"test","value":42}', "Object delete")

-- Test 14: Array move
arr:move(0,2)
assert_equal(arr:tojson(), "[2,3,1,4,5]", "Array move")

-- Test 15: Object delete
obj.new = "val"
obj:move("new", "value")
assert_equal(obj:tojson(), '{"name":"test","new":"val","value":42}', "Object move")
obj.new = nil

-- Test 16: Nested structures
local nested = JSON:object("array", JSON:array(1, 2, 3), "object", JSON:object("nested", true))
assert_equal(nested:tojson(), '{"array":[1,2,3],"object":{"nested":true}}', "Nested structures")

-- Test 17: JSON parsing
local parsed = JSON:parse('[1,2,3]')
assert_json_equal(parsed, JSON:array(1, 2, 3), "JSON parsing array")

-- Test 18: Object parsing
local parsed_obj = JSON:parse('{"a":1,"b":2}')
assert_json_equal(parsed_obj, JSON:object("a", 1, "b", 2), "JSON parsing object")

-- Test 15: Lua marshalling
local lua_str = arr:tolua()
if _VERSION == "Lua 5.1" then
    assert_equal(type(loadstring("return " .. lua_str)()), "table", "Lua marshalling")
else
    assert_equal(type(load("return " .. lua_str)()), "table", "Lua marshalling")
end
-- Test 16: Escaped JSON
local escaped = JSON:array("test\n", "quote\"")
assert_equal(escaped:tojson(), '["test\n","quote""]', "Escaped JSON")

-- Test 17: Length calculations
assert_equal(arr:len(), arr:rlen(0, false), "Length consistency")

-- Test 18: Object key deletion
obj.name = nil
assert_equal(obj:tojson(), '{"value":42}', "Object key deletion")

-- Test 19: Mixed types
local mixed = JSON:array(1, "string", true, null, JSON:object("nested", "value"))
assert_equal(mixed:tojson(), '[1,"string",true,null,{"nested":"value"}]', "Mixed types")


-- Test 20: Large array performance
local large_arr = JSON:array()
for i = 0, 1000 do
    large_arr:push(i)
end
assert_equal(#large_arr-1, 1000, "Large array creation")
assert_equal(large_arr[#large_arr-1], 1000, "Large array access")

-- Test 21: Circular/Recursive Structures
local o = JSON:object("name", "root")
local a = JSON:array(1, 2)

o.arr = a:unref()
--a:push(o) -- Create the circular loop

local status, result = pcall(function() return o:tojson() end)
assert_equal(status, true, "Unref structure handling (no crash)")
-- Verify truncation: It should contain the first level but not infinite nesting
local expected = '{"name":"root","arr":[1,2]}'
assert_equal(result, expected, "Unref structure pruning at boundary")

local arr = JSON:array(1,2,3,4,5);

arr.env[5]=6;
local res = arr:tojson()
local expected = '[1,2,3,4,6]'
assert_equal(res, expected, "Array env update value using 1 based index")

arr.env[#arr+1]=7
local res = arr:tojson()
local expected = '[1,2,3,4,6,7]'
assert_equal(res, expected, "Array env set value using 1 based index")

arr.env[#arr]=nil
local res = arr:tojson()
local expected = '[1,2,3,4,6]'
assert_equal(res, expected, "Array env (nil) rem value using 1 based index")

arr.env:pop()
local res = arr:tojson()
local expected = '[1,2,3,4]'
assert_equal(res, expected, "Array env pop value")

arr.env:push(5)
local res = arr:tojson()
local expected = '[1,2,3,4,5]'
assert_equal(res, expected, "Array env push value")

local n = arr.env:shift()
local an = 1
local res = arr:tojson()
local expected = '[2,3,4,5]'
assert_equal(res, expected, "Array env shift value")


local n = arr.env:unshift(-2, -1, 0, 1)
local an = #arr
local res = arr:tojson()
local expected = '[-2,-1,0,1,2,3,4,5]'
assert_equal(res, expected, "Array env unshift 2 values")

if _VERSION == "Lua 5.1" then
    local at = {1,2,3,4,5}
    local ta = JSON:parse_table(at)
    local res = ta:tojson()
    local expected = '[1,2,3,4,5]'
    assert_equal(res, expected, "Array inline table args")

    local mt = {1,2,3,4,5, test="this", age=99}
    local ma = JSON:parse_table(mt)
    local res = ma:tojson()
    local expected = '[1,2,3,4,5,{"test":"this","age":99}]'
    assert_equal(res, expected, "Array inline mixed table parsed as an array")

    local mnt = {1,2,3,4,5, test="this", age=99,nested={more="data", name="teddy"}}
    local mna = JSON:parse_table(mnt)
    local res = mna:tojson()
    local expected = '[1,2,3,4,5,{"test":"this","age":99,"nested":{"more":"data","name":"teddy"}}]'
    assert_equal(res, expected, "Array inline mixed table with nested object parsed as an array")

    local mnt = {1,2,3,4,5,{6,7,8,9},test="this", age=99, nested={more="data", name="teddy"}}
    local mna = JSON:parse_table(mnt, "-a", "arr")
    local res = mna:tojson()
    local expected = '[1,2,3,4,5,[6,7,8,9],{"test":"this","age":99,"nested":{"more":"data","name":"teddy"}}]'
    assert_equal(res, expected, "Parse inline mixed table user supplied name for nested elmenets")

    local mnt = {1,2,3,4,5,{6,7,8,9},test="this", age=99, nested={more="data", name="teddy"}}
    local mna = JSON:parse_table(mnt, "-a", "arr")
    local res = mna:tojson()
    local expected = '[1,2,3,4,5,[6,7,8,9],{"test":"this","age":99,"nested":{"more":"data","name":"teddy"}}]'
    assert_equal(res, expected, "Parse inline mixed table user override element type -a ")

    local mnt = {1,2,3,4,5,{6,7,8,9},test="this", age=99, nested={more="data", name="teddy"}}
    local mna = JSON:parse_table(mnt, "-a", true)
    local res = mna:tojson()
    local expected = '[1,2,3,4,5,[6,7,8,9]]'
    assert_equal(res, expected, "Parse inline mixed table user override mixed parsing")
end
print("============================")
print("Tests completed: " .. tests_passed .. "/" .. tests_total)
if tests_passed == tests_total then
    print("All tests PASSED! ✅")
else
    print("Some tests FAILED! ❌")
end

