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