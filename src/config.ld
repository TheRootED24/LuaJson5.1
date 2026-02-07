-- config.ld
project = "LuaJson5.1"
file = {'lua_json.h', 'lua_json_array.h', 'lua_json_lua.h', 'lua_json_object.h'}
add_language_extension('c', 'c')
merge = true  -- Allows combining docs from multiple files into one module
