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