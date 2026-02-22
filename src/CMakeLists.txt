cmake_minimum_required(VERSION 3.1...3.27)

#-------------------------------------------------------------------------------
# Build Environment
#-------------------------------------------------------------------------------

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-fPIC -W -s)
  add_compile_options(-W -Wall -Wextra -O3)
endif()

project(
  LuaJson
  VERSION 1.0.1
  LANGUAGES C
  )

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(JSON SHARED lua_json.c
                        lua_json_lua.c
                        lua_json_array.c
                        lua_json_object.c
                        lua_json_marshal.c
                        lua_json_elm_event.c
                        includes/libjson/json.c
                        includes/libjson/lstr.c
                        includes/libjson/base64.c)
                        
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
target_link_libraries(JSON liblua5.1.so)
target_link_libraries(JSON m)

install(TARGETS JSON DESTINATION /usr/lib/lua/LuaJson/)
