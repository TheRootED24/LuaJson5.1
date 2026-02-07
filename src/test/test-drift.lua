--[[local JSON = require "JSON"

-- 1. Create a simple array
local a = JSON:array(1) 
local initial_len = a:len()
print("Initial RLEN: " .. initial_len) -- Should be around 4 ( [ 1 ] \0 )

-- 2. Perform 10 replacements of the SAME size
-- We replace index 1 (the number 1) with another number (2)
-- If accounting is perfect, RLEN should NOT change.
print("\n--- Starting 10 Replacement Strikes ---")
for i = 1, 10 do
    a.env[1] = 2
    print("Iteration " .. i .. " RLEN: " .. a:len())
end

-- 3. The Result
if a:len() == initial_len then
    print("\n✅ SUCCESS: Accounting is Symmetric. No drift.")
else
    print("\n❌ FAIL: RLEN Drifted! Current: " .. a:len() .. " vs Expected: " .. initial_len)
    print("Drift per strike: " .. (a:len() - initial_len) / 10 .. " bytes")
end
]]--

local JSON = require "JSON"

-- 1. Create a Root and a Child
local root = JSON:object("child", JSON:array(1,2,3))
print("Initial Root RLEN: " .. root:len())

-- 2. THE STRIKE: Replace the Child (Userdata) with a Boolean (Scalar)
print("\n--- Replacing Nested Array with Boolean ---")
root.child = true 

-- 3. THE RENDER: This usually triggers the traverse/free crash
print("New Root RLEN: " .. root:len())
print("JSON: " .. root:tojson())