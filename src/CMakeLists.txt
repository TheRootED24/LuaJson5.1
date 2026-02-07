cmake_minimum_required(VERSION 3.1...3.27)

#-------------------------------------------------------------------------------
# Build Environment
#-------------------------------------------------------------------------------

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-fpic -W )
  add_compile_options(-W -Wall -Wextra -O3)
endif()

project(
  LuaJson
  VERSION 5.1
  LANGUAGES C
  )

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(JSON SHARED lua_json.c
                        lua_json_array.c
                        lua_json_object.c
                        lua_json_lua.c
                        includes/libjson/json.c
                        includes/libjson/base64.c
                        includes/libjson/lstr.c)
                        
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
target_link_libraries(JSON liblua5.1.so)
#target_link_libraries(JSON libjson.so)
target_link_libraries(JSON m)
install(TARGETS JSON DESTINATION /usr/lib/lua/LuaJson/)
