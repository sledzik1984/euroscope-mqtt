cmake_minimum_required(VERSION 3.14)

project(euroscope-mqtt VERSION "1.0.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------
# 32-bit build (EuroScope is 32-bit)
# ---------------------------------------
if (MSVC)
    set(CMAKE_GENERATOR_PLATFORM Win32)
    add_definitions(/D_USRDLL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP /sdl /permissive- /DNOMINMAX")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /W4 /MP /sdl /permissive- /DNOMINMAX")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFESTUAC:NO /ignore:4099")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD=1)
endif()

# ---------------------------------------
# Version header
# ---------------------------------------
configure_file(
    ${CMAKE_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_BINARY_DIR}/Version.h
)

# ---------------------------------------
# Plugin sources
# ---------------------------------------
set(SOURCE_FILES
    src/plugin.cpp
    src/plugin.h
    src/main.cpp
    ${CMAKE_BINARY_DIR}/Version.h
)

add_library(myPlugIn SHARED ${SOURCE_FILES})
set_target_properties(myPlugIn PROPERTIES OUTPUT_NAME "euroscope-mqtt")

# ---------------------------------------
# Include directories
# ---------------------------------------
target_include_directories(myPlugIn PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/include          # EuroScopePlugInDLL headers
)

# ---------------------------------------
# Add and configure Paho MQTT C
# ---------------------------------------
set(PAHO_BUILD_STATIC TRUE CACHE INTERNAL "Build static library only")
set(PAHO_WITH_SSL FALSE CACHE INTERNAL "Disable SSL")
set(PAHO_BUILD_DOCUMENTATION FALSE CACHE INTERNAL "")
set(PAHO_ENABLE_TESTING FALSE CACHE INTERNAL "")
set(PAHO_ENABLE_CPACK FALSE CACHE INTERNAL "")
set(PAHO_BUILD_SAMPLES FALSE CACHE INTERNAL "")
set(PAHO_BUILD_SHARED FALSE CACHE INTERNAL "")
set(PAHO_BUILD_DEMOS FALSE CACHE INTERNAL "")
set(PAHO_BUILD_TOOLS FALSE CACHE INTERNAL "")
set(PAHO_BUILD_MQTT3ASYNC TRUE CACHE INTERNAL "Enable async lib")

add_subdirectory(external/paho.mqtt.c)


# ---------------------------------------
# Add and build Paho MQTT C++ (manual)
# ---------------------------------------
file(GLOB PAHO_CPP_SOURCES
    external/paho.mqtt.cpp/src/*.cpp
)

add_library(paho-mqttpp3 STATIC ${PAHO_CPP_SOURCES})
target_include_directories(paho-mqttpp3 PUBLIC
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/src
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/include
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.c/src
)
target_link_libraries(paho-mqttpp3 PRIVATE paho-mqtt3a)

# ---------------------------------------
# Link everything into plugin
# ---------------------------------------
target_link_libraries(myPlugIn
    ${CMAKE_SOURCE_DIR}/external/lib/EuroScopePlugInDLL.lib
    ws2_32
    crypt32
    shlwapi
    paho-mqttpp3
    paho-mqtt3a
)

# ---------------------------------------
# Preprocessor definitions
# ---------------------------------------
add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -DSQLITE_THREADSAFE=0
    -DSQLITE_DEFAULT_FILE_FORMAT=4
    -DSQLITE_DEFAULT_SYNCHRONOUS=0
    -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=0
    -DSQLITE_WIN32_MALLOC
)
