cmake_minimum_required(VERSION 3.1...3.27)

#-------------------------------------------------------------------------------
# Build Environment
#-------------------------------------------------------------------------------

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-fpic -W -s)
  add_compile_options(-W -Wall -Wextra -Os -s)
endif()

project(
  LuaJson
  VERSION 5.1
  LANGUAGES C
  )

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(JSON SHARED lua_json.c
                        lua_json_lua.c
                        lua_json_array.c
                        lua_json_object.c
                        lua_json_marshal.c
                        lua_json_elm_event.c)   
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
target_link_libraries(JSON mongoose)

install(TARGETS JSON DESTINATION /usr/lib/lua/LuaJson/)
