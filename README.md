# LuaJson - Native Json types and json handling for lua

# Features 
- 0 based indexing for arrays and objects, can also be set to 1 based via :json_index(false) method
- Lua safe for all lua types, non printable types(functions, threads, userdatas..etc)  are silently ignored by renderer, tables are rendered
- Memory safe and valgrind approved. Check for yourself ie ..`valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes lua test/<script_name>.lua`
- Full event system, nested parent elements (object/arrays) subscribe to their childrens events, all events bubble to root
- The marshal can marshal as json (arrays/objects) or lua tables via the respective methods :tojson() and :tolua() or create a new table from the element via :totable()
- Length, #keys, #quoted strings are tracked, the :len() method guarantees an accurate render length, There is a cost for nested tables and accesses from lua to elm.env see documentation
  for more details of cost
- Arrays and Objects can be treated as regualar tables in lua by accessing the elm.env ie ..for k,v in pairs(elm.env) do --lua stuff end
- Lightweight (64kb) with no external dependencies. The parser currently uses mongoose json for tokenization (batteries included) or link to mongoose if using it or the lua mongoose bindings, in which case the binary is only 60kb
- Objects can be treated as an array of key value pairs, they share all the some methods as arrays..and then some. AKA accessed like obj:move(0,6) or obj:move("this_key", "to_this_key")
  or print(obj[0]) print(obj.key)
- Since the underlaying data structure is a table, O/1 hash lookups remain for objects, arrays O/n when index is unknown.
- Object mtutation (reverse, move, insert, push, pop ..etc) does not moddify the actual table data, just the lookup table, Thus they are extremely memory efficent for these operations compared to most JS implimentations where the data itself is shifted.
- The total overhead memory cost of an empty object / array is about 90 bytes.
-  Untested tho logic would suggest luaJson should compile for anything that compiles lua.
-  Parse large(30mb) json payload --> 1000 cycles: Total Parse Time: 4795.156 ms  Per Full Array Parse: (29939 bytes) 4.7952 ms (luajit would liekly speed it up more yet 

# API Reference
https://therooted24.github.io/LuaJson/

# Demos 

parsing mixed table to json 
```lua
local JSON = require "JSON"

-- create a mixed table
local t = {1,2,3,4, some="data", age=99, root=false}
print(t) --> table: 0x4c14380

-- parse t with no args, defaults to type with most items (array in this case --> arr 4 | obj 3)
local arr = JSON.parse_table(arr, t) --> alt syntax JSON:parse_table(t)
print(arr) --> array: 0x4c14f00
print(arr:tojson()) --> [1,2,3,4,{"root":false,"age":99,"some":"data"}]

-- parse t as an object
local obj = JSON.parse_table(obj, t, "-o") --> alt syntax JSON:parse_table(t, "-o")
print(obj) --> object: 0x4c17aa0
print(obj:tojson()) --> {"root":false,"age":99,"some":"data","mixed_keys":[1,2,3,4]}

-- parse t as an object, user supplied key for mixed items ( "arr" in this case )
local obj = JSON.parse_table(obj, t, "-o", "arr") --> alt syntax JSON:parse_table(t, "-o")
print(obj) --> object: 0x4c1a790
print(obj:tojson()) --> {"root":false,"age":99,"some":"data","arr":[1,2,3,4]}

-- parse t as an array, set verbose output
arr = JSON.parse_table(arr, t, "-a-v") --> alt syntax JSON:parse_table(t, "-a-v")
--> Result:[ type: array | object len: 3 | array len: 4 | mixed: true | fixups: 0 ]
print(arr) --> array: 0x4c1a6d0
print(arr:tojson()) --> [1,2,3,4,{"root":false,"age":99,"some":"data"}]

-- parse as array with no mixed
arr = JSON.parse_table(arr, t, "-a", true) --> alt syntax JSON:parse_table(t, "-a", true)
print(arr) --> array: 0x4c1c200
print(arr:tojson()) --> [1,2,3,4]

-- parse t as an object no mixed
local obj = JSON.parse_table(obj, t, "-o", true) --> alt syntax JSON:parse_table(t, "-o", true)
print(obj) --> object: 0x4c1cbf0
print(obj:tojson()) --> {"root":false,"age":99,"some":"data"}
```

some native json tricks
```lua
local JSON = require "JSON"

-- parse some serialized data 
local user = JSON:parse('{"id":0,"name":"Elijah","city":"Austin","age":78,"friends":[{"name":"Michelle","hobbies":["Watching Sports","Reading","Skiing & Snowboarding"]},{"name":"Robert","hobbies":["Traveling","Video Games"]}]}')

print(user.name) --> Elijah
print(user.friends[0].name) --> Michelle
print(user.friends[0].hobbies[1]) --> Reading

-- notice order is perserved !!
print(user:tojson()) --> {"id":0,"name":"Elijah","city":"Austin","age":78,"friends":[{"name":"Michelle","hobbies":["Watching Sports","Reading","Skiing & Snowboarding"]},{"name":"Robert","hobbies":["Traveling","Video Games"]}]}

-- make a 
local obj = JSON:parse([[
	{
		"ip":"192.168.1.1",
		"name":"ted",
		"expires":19087,
		"hobbies":[
					"fishing", 
					"hunting", 
					"running",
                    "read"
		]
	}
]])

print(obj.name) --> ted 
print(obj.hobbies[0]) --> fishing

-- access as a table
print(obj.hobbies.env[1]) --> fishing

-- use ipairs on obj.hobbies env table 
for i,v in ipairs(obj.hobbies.env) do 
 print(i, v)
end

--output
--[[
1       fishing
2       hunting
3       running
4       read
]]--

-- notice order is perserved !!
print(obj:tojson()) --> {"ip":"192.168.1.1","name":"ted","expires":19087,"hobbies":["fishing","hunting","running","read"]}

-- pasre json with some emojis
local emo = JSON:parse([[
    {
        "status": "All systems operational 🟢",
        "rating": "⭐⭐⭐⭐⭐",
        "categories": [
                        "🍕 Food", 
                        "🎵 Music", 
                        "💻 Tech"
                      ],
        "mood": "😴"
    }
]])

print(emo:tojson()) --> {"status":"All systems operational 🟢","rating":"⭐⭐⭐⭐⭐","categories":["🍕 Food","🎵 Music","💻 Tech"],"mood":"😴"}

-- move "categories" array to start (index 0) 
emo:move(2,0)
print(emo:tojson()) --> {"categories":["🍕 Food","🎵 Music","💻 Tech"],"status":"All systems operational 🟢","rating":"⭐⭐⭐⭐⭐","mood":"😴"}

-- move it back to index 2 using keys in place of indexes
emo:move("categories","rating")
print(emo:tojson()) --> {"status":"All systems operational 🟢","rating":"⭐⭐⭐⭐⭐","mood":"😴","categories":["🍕 Food","🎵 Music","💻 Tech"]}

-- unshift a few k/v pairs to the start of the object
emo:unshift("new", "value", "age", 100, "root", true)
print(emo:tojson()) --> {"new":"value","age":100,"root":true,"status":"All systems operational 🟢","rating":"⭐⭐⭐⭐⭐","mood":"😴","categories":["🍕 Food","🎵 Music","💻 Tech"]}

shifted = {}
-- shift them off and stick them into a table 
for i = 1, 3 do
    shifted[i] = emo:shift()
end

print(emo:tojson()) --> {"status":"All systems operational 🟢","rating":"⭐⭐⭐⭐⭐","mood":"😴","categories":["🍕 Food","🎵 Music","💻 Tech"]}

for i,v in pairs(shifted) do
    print(i,v)
end --[[
    1       value
    2       100
    3       true
]]

-- insert another object at index 2
emo:insert(2, "new_obj", JSON:object("new", "data", "time", 0100, "on-duty", false))

print(emo:tojson()) --> {"status":"All systems operational 🟢","rating":"⭐⭐⭐⭐⭐","new_obj":{"new":"data","time":100,"on-duty":false},"mood":"😴","categories":["🍕 Food","🎵 Music","💻 Tech"]}

-- reverse the entire object 
emo:reverse()
print(emo:tojson()) --> {"mood":"😴","categories":["🍕 Food","🎵 Music","💻 Tech"],"new_obj":{"new":"data","time":100,"on-duty":false},"rating":"⭐⭐⭐⭐⭐","status":"All systems operational 🟢"}

-- reverse from "mood" to "new_obj"
emo:reverse("mood", "new_obj")
print(emo:tojson()) --> {"new_obj":{"new":"data","time":100,"on-duty":false},"categories":["🍕 Food","🎵 Music","💻 Tech"],"mood":"😴","rating":"⭐⭐⭐⭐⭐","status":"All systems operational 🟢"}

-- print the current rendder length
print(emo:len()) --> 186

-- delete the "new_obj" and eval the length is correct afterwards
emo.new_obj = nil

-- check the length updated correctly 
print(emo:len()) --> 134

-- check object is still correct order 
print(emo:tojson()) --> {"categories":["🍕 Food","🎵 Music","💻 Tech"],"mood":"😴","rating":"⭐⭐⭐⭐⭐","status":"All systems operational 🟢"}

-- reverse "categories" and "mood"
emo:reverse(0,1)
print(emo:tojson()) --> {"mood":"😴","categories":["🍕 Food","🎵 Music","💻 Tech"],"rating":"⭐⭐⭐⭐⭐","status":"All systems operational 🟢"}

-- now reverse entiree object .. back to where we started :) 
emo:reverse()
print(emo:tojson()) --> {"status":"All systems operational 🟢","rating":"⭐⭐⭐⭐⭐","categories":["🍕 Food","🎵 Music","💻 Tech"],"mood":"😴"}
```

an array of tables 
```lua
local JSON = require "JSON"

local t1 = {1,2,3,4,5}
local t2 = {4,5,6,7,8}
local t3 = {"nine", "ten", "eleven", "twelve"}

local t_arr = JSON:array(t1, t2, t3)

print(t_arr:tojson()) --> [[1,2,3,4,5],[4,5,6,7,8],["nine","ten","eleven","twelve"]]

t_arr:move(0,1) 
print(t_arr:tojson()) --> [[4,5,6,7,8],[1,2,3,4,5],["nine","ten","eleven","twelve"]]

t_arr:reverse(1,2)
print(t_arr:tojson()) --> [[4,5,6,7,8],["nine","ten","eleven","twelve"],[1,2,3,4,5]]

t_arr:reverse(0,1)
print(t_arr:tojson()) --> [["nine","ten","eleven","twelve"],[4,5,6,7,8],[1,2,3,4,5]]

t_arr:reverse()
print(t_arr:tojson()) --> [[1,2,3,4,5],[4,5,6,7,8],["nine","ten","eleven","twelve"]]
```

As of version 1.1.2, luaJson can produce valid bash associative arrays (dictionarys) and standard indexed arrays
```bash
#!/bin/bash

echo "=== 1. Invoking C/Lua Engine for Nested Payload ==="
# Call luajson with args and request an object -o with a nested array -a
parent_obj=$(./luajson -bash -o "test" "me" "age" 99 "root" false "arr" -a "🍕 Food" "🎵 Music" "💻 Tech")
# Capture the output from script.

echo "Generated Bash Payload:"
echo "$parent_obj"
echo "--------------------------------------------------------"

echo -e "\n=== 2. Parsing the Parent Object ==="
# Declare an associative array (dictionary) for the parent object
declare -A my_obj

# Evaluate the payload directly into native Bash memory
eval "my_obj=$parent_obj"

# Print an item from the main Object
echo "Successfully extracted from Object -> [test]: ${my_obj[test]}"
echo "Successfully extracted from Object -> [age]:  ${my_obj[age]}"

echo -e "\n=== 3. Unpacking and Parsing the Nested Array ==="
# Extract the nested string payload out of the parent object's "arr" key
nested_arr_str="${my_obj[arr]}"
echo "Extracted Nested String: $nested_arr_str"

# Declare a standard indexed array for the child list
declare -a child_arr

# Evaluate the inner string structure into the new indexed array
eval "child_arr=$nested_arr_str"

# Print an item from the nested Array
echo "Successfully extracted from Child Array -> Index 0: ${child_arr[0]}"
echo "Successfully extracted from Child Array -> Index 1: ${child_arr[1]}"
```

bash output
```bash
hostle@hostle-Inc:~/luajson-c-api/src$ ./test-bash.bash
=== 1. Invoking C/Lua Engine for Nested Payload ===
Generated Bash Payload:
([age]=99 [root]="false" [arr]="(\"🍕 Food\" \"🎵 Music\" \"💻 Tech\")" [test]="me")
--------------------------------------------------------

=== 2. Parsing the Parent Object ===
Successfully extracted from Object -> [test]: me
Successfully extracted from Object -> [age]:  99

=== 3. Unpacking and Parsing the Nested Array ===
Extracted Nested String: ("🍕 Food" "🎵 Music" "💻 Tech")
Successfully extracted from Child Array -> Index 0: 🍕 Food
Successfully extracted from Child Array -> Index 1: 🎵 Music
```
