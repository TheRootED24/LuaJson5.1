-- test_recursion.lua
local json = require "JSON" -- Adjust require string
local root = json:object()
local child = json:object()

child["name"] = "Shared Child"

-- 1. The DAG Test (Diamond)
-- In "Strict Tree" mode, 'child' is marked when 'a' renders.
-- When 'b' tries to render 'child', it sees the mark and aborts SILENTLY.
root["a"] = child
root["b"] = child 

-- 2. The Cycle Test (OOM Death)
-- 'child' points back to 'root'. 
-- The render enters 'child', sees 'root' (marked at start), and aborts.
child["loop"] = root

print("----------- START RENDER -----------")
print(root:tojson()) 
print("----------- END RENDER   -----------")
