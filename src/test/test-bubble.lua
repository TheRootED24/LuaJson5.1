local JSON = require "JSON"

-- 1. Create the Hierarchy
local root = JSON:object("name", "Root")
local branch = JSON:object("name", "Branch")
local leaf = JSON:object("name", "Leaf")
local leaf2 = JSON:object("name", "Leaf2")

-- 2. Nest them: Root -> Branch -> Leaf
root.child = branch
branch.child = leaf
leaf.child = leaf2

local initial_root_len = root:len()
print("Initial Root Length: " .. initial_root_len)

-- 3. Trigger the Reactive Change at the very bottom
print("\n--- Modifying Leaf ---")
leaf.child.status = "Active"  -- This adds roughly 18 bytes ( "status":"Active", )

-- 4. Check the results
print("New Leaf2 Length:   " ..leaf2:len())
print("New Leaf Length:   " .. leaf:len())
print("New Branch Length: " .. branch:len())
print("New Root Length:   " .. root:len())


-- 5. The Moment of Truth
if root:len() > initial_root_len then
    print("\n✅ SUCCESS: Change bubbled all the way to the Root!")
else
    print("\n❌ FAIL: Change stopped at the Branch. No bubbling.")
end

root:tojson()

print("New Leaf2 Length:   " ..leaf2:len())
print("New Leaf Length:   " .. leaf:len())
print("New Branch Length: " .. branch:len())
print("New Root Length:   " .. root:len())
leaf.child=nil

print(root:tojson())
print(root:len())
print(#root, #leaf)