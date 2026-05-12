cmake_minimum_required(VERSION 3.10...4.0)

project(LuaJsonInternal VERSION 1.1.1 LANGUAGES C)

# 1. --- Configuration Options (Overridable via CLI/OpenWrt) ---
set(LUA_VERSION "5.4" CACHE STRING "Lua version to build for (5.1, 5.3, or 5.4)")
option(USE_SYSTEM_MONGOOSE "Force linking against system mongoose library" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 2. --- Build Flags ---
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror -O3 -flto)
endif()

# 3. --- Version-Dependent Definitions & Path Detection ---
if(LUA_VERSION STREQUAL "5.4")
    add_definitions(-DLUA_VERSION_NUM=504 -DLUA_COMPAT_5_3)
    message(STATUS "Build Mode: LUA 5.4 Profile")
    # FIX: Remove the if(NOT) blocks to allow OpenWrt options to overwrite the cache dynamically!
    find_path(LUA_INTERNAL_INC lua.h PATH_SUFFIXES lua5.4 lua54)
    find_library(LUA_INTERNAL_LIB NAMES lua5.4 lua54 liblua5.4 liblua54)

elseif(LUA_VERSION STREQUAL "5.3")
    add_definitions(-DLUA_VERSION_NUM=503 -DLUA_COMPAT_5_2)
    message(STATUS "Build Mode: LUA 5.3 Profile")
    find_path(LUA_INTERNAL_INC lua.h PATH_SUFFIXES lua5.3 lua53)
    find_library(LUA_INTERNAL_LIB NAMES lua5.3 lua53 liblua5.3 liblua5.3)

else()
    add_definitions(-DLUA_VERSION_NUM=501)
    message(STATUS "Build Mode: LUA 5.1 Profile")
    find_path(LUA_INTERNAL_INC lua.h PATH_SUFFIXES lua5.1 lua51 lua)
    find_library(LUA_INTERNAL_LIB NAMES lua5.1 lua51 liblua5.1 liblua51 lua)
endif()

# Fallback assignments: Use OpenWrt command-line definitions first, then fallback to local lookups
if(NOT LUA_INCLUDE_DIR)
    set(LUA_INCLUDE_DIR ${LUA_INTERNAL_INC})
endif()
if(NOT LUA_LIBRARIES)
    set(LUA_LIBRARIES ${LUA_INTERNAL_LIB})
endif()

message(STATUS "Targeting Lua version: ${LUA_VERSION}")
message(STATUS "Lua Headers: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua Libs: ${LUA_LIBRARIES}")

# 4. --- Mongoose Multi-Route Setup ---
if(USE_SYSTEM_MONGOOSE)
    find_library(MONGOOSE_LIB NAMES mongoose REQUIRED)
    message(STATUS "Mongoose Engine: External Shared Lib (${MONGOOSE_LIB})")
    set(MONGOOSE_SOURCES "")
else()
    message(STATUS "Mongoose Engine: Internal Batteries Included")
    set(MONGOOSE_SOURCES 
        includes/mongoose/base64.c
        includes/mongoose/json.c
        includes/mongoose/str.c)
endif()

# Core translation components list
set(CORE_SOURCES
    lua_json.c
    lua_json_lua.c
    lua_json_array.c
    lua_json_object.c
    lua_json_marshal.c
    lua_json_elm_event.c
    ${MONGOOSE_SOURCES}
)

# ==============================================================================
# TARGET 1: Native Lua Extension Module (JSON.so)
# ==============================================================================
add_library(JSON SHARED ${CORE_SOURCES})
target_include_directories(JSON PRIVATE ${LUA_INCLUDE_DIR} includes/mongoose)
target_link_libraries(JSON PRIVATE ${LUA_LIBRARIES})

if(USE_SYSTEM_MONGOOSE AND MONGOOSE_LIB)
    target_link_libraries(JSON PRIVATE ${MONGOOSE_LIB})
endif()

set_target_properties(JSON PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# ==============================================================================
# TARGET 2: Standalone C API Shared Library (libluajson.so)
# ==============================================================================
add_library(luajson SHARED 
    ${CORE_SOURCES}
    luajson_api.c
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(luajson PRIVATE "-Wl,-Bsymbolic-functions")
endif()

target_include_directories(luajson 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${LUA_INCLUDE_DIR} includes/mongoose
)
target_link_libraries(luajson PRIVATE ${LUA_LIBRARIES})

if(USE_SYSTEM_MONGOOSE AND MONGOOSE_LIB)
    target_link_libraries(luajson PRIVATE ${MONGOOSE_LIB})
endif()

# ==============================================================================
# 7. --- Native Installation Destructuring ---
# ==============================================================================
install(TARGETS JSON luajson
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

