cmake_minimum_required(VERSION 3.14)

project(euroscope-mqtt VERSION "1.0.0")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# --- Platform config ---
if (MSVC)
    set(CMAKE_GENERATOR_PLATFORM Win32)
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif ()
    if (NOT CMAKE_CXX_FLAGS MATCHES "/MP")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
    endif ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /sdl /permissive- /DNOMINMAX")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /sdl /permissive- /DNOMINMAX")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFESTUAC:NO /ignore:4099")
    add_definitions(/D_USRDLL)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD=1)
endif()

# --- Version header ---
configure_file(
    ${CMAKE_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_BINARY_DIR}/Version.h
)

# --- Add MQTT-C library ---
add_subdirectory(external/mqtt-c)

# --- Plugin sources ---
set(SOURCE_FILES
    src/plugin.cpp
    src/plugin.h
    src/main.cpp
    ${CMAKE_BINARY_DIR}/Version.h
)

add_library(myPlugIn SHARED ${SOURCE_FILES})
set_target_properties(myPlugIn PROPERTIES OUTPUT_NAME "euroscope-mqtt")

# --- Include directories ---
include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/include             # EuroScopePlugInDLL headers
    ${CMAKE_SOURCE_DIR}/external/mqtt-c             # Include MQTT-C headers
    ${CMAKE_SOURCE_DIR}
)

# --- Link libraries ---
link_directories(${CMAKE_SOURCE_DIR}/external/lib)

target_link_libraries(myPlugIn
    ${CMAKE_SOURCE_DIR}/external/lib/EuroScopePlugInDLL.lib
    ${CMAKE_SOURCE_DIR}/external/mqtt-c/build/libmqtt.a # Link the MQTT-C static library
    ws2_32
    crypt32
    Shlwapi
)

# --- Preprocessor definitions ---
add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -DSQLITE_THREADSAFE=0
    -DSQLITE_DEFAULT_FILE_FORMAT=4
    -DSQLITE_DEFAULT_SYNCHRONOUS=0
    -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=0
    -DSQLITE_WIN32_MALLOC
    -DSQLITE_THREADSAFE=0
)
