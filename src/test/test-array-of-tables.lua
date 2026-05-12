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

