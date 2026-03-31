require "JSON"
o = JSON:object("test","me")
 print(o:tojson())

p = o:props()
p.dom_id="lua-el-1"
print(p)
print(p.dom_id)

p.tags = { "div", "img", "body" }

for i,v in pairs(p.tags) do
    print(v)
end
