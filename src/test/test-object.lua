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

print("Starting LuaJson Object Test")
print("============================")

local p_obj = JSON.parse([[
	{
		"ip":"192.168.1.1",
		"name":"ted",
		"expires":19087,
		"hobbies":[
					"fish", 
					"hunt", 
					"jog"
		]
	}
]])
print(p_obj:tojson())

local t_obj = {
    ip="192.168.1.1",
    name="ted",
    expires=19087,
    hobbies = {
        "fish",
        "hunt",
        "jog"
    }
}

t_obj = JSON:parse_table(t_obj, "-o")
print(t_obj:tojson())
-- Test 1: Basic object creation
local obj = JSON:object("ip","192.168.1.1","name","ted","expires",19087,"hobbies", {"fish","hunt","jog"})
assert_equal(obj:tojson(), '{"ip":"192.168.1.1","name":"ted","expires":19087,"hobbies":["fish","hunt","jog"]}', "Basic object creation")

-- Test  Object access
assert_equal(obj.name, "ted", "Object property access")
assert_equal(obj.expires, 19087, "Object property access ")


