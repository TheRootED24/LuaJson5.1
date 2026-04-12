local JSON = require "JSON"

function print_stats(label, obj)
    local rlen = obj:len()
    local json = obj:tojson()
    local alen = #json
    local diff = rlen - alen
    
    local nk = obj:nkeys()
    local qt = obj:quoted()
    
    print(string.format("--- %s ---", label))
    print(string.format("Render Len (rlen): %d", rlen))
    print(string.format("Actual Len (str):  %d", alen))
    print(string.format("Ghost Bytes:       %d", diff))
    print(string.format("Structure:         Keys: %d | Quoted: %d", nk, qt))
    print(string.format("Payload:           %s", json))
    print("")
    return {len=rlen, nk=nk, qt=qt}
end

-- 1. Setup
print("=== STEP 1: INIT ===")
local o = JSON:object("test","me")
local o2 = JSON:object("mee","too")
o.obj = o2

local start_o = print_stats("Parent (o) Start", o)

-- 2. The Modification
print("=== STEP 2: MODIFY CHILD ===")
o2.new = "tester" 

-- 3. Verification
local end_o2 = print_stats("Child (o2) End", o2)
local end_o = print_stats("Parent (o) End", o)

-- 4. The Autopsy
local delta_len = end_o.len - start_o.len
local delta_nk  = end_o.nk - start_o.nk
local delta_qt  = end_o.qt - start_o.qt

print("=== AUTOPSY ===")
print(string.format("Parent Length Growth: %d (Expected 15)", delta_len))
print(string.format("Parent Key Growth:    %d (Expected 0 - Keys are local)", delta_nk))
print(string.format("Parent Quote Growth:  %d (Expected 0 - Quotes are local)", delta_qt))

if delta_len == 29 then
    print("\n🚨 PATTERN CONFIRMED: Delta (15) + Payload (14) = 29")
end