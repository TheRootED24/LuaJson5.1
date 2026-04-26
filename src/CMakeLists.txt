cmake_minimum_required(VERSION 3.1...3.27)

#-------------------------------------------------------------------------------
# Build Environment
#-------------------------------------------------------------------------------

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-fPIC -W -s)
  add_compile_options(-W -Wall -Wextra -Werror)
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
                        #includes/mongoose/base64.c
                        #includes/mongoose/json.c
                        #includes/mongoose/str.c)   
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -s")
target_link_libraries(JSON PRIVATE mongoose)
target_link_libraries(JSON PRIVATE liblua5.1.so)
#find_package(PkgConfig REQUIRED)
# 2. Search for the luajit library
#pkg_check_modules(LUAJIT REQUIRED IMPORTED_TARGET luajit)
#target_link_libraries(JSON PRIVATE PkgConfig::LUAJIT)

install(TARGETS JSON DESTINATION /usr/lib/lua/LuaJson/)
