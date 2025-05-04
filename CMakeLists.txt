cmake_minimum_required(VERSION 3.21...3.27)

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

include(ExternalProject)
include(GNUInstallDirs)

set(PAHO_INSTALL_DIR ${CMAKE_BINARY_DIR}/_install/paho_mqtt_c)
set(PAHO_CPP_INSTALL_DIR ${CMAKE_BINARY_DIR}/_install/paho_mqtt_cpp)

# ---------------------------------------
# Build Paho MQTT C
# ---------------------------------------
ExternalProject_Add(paho_mqtt_c
    GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
    GIT_TAG        v1.3.13
    UPDATE_DISCONNECTED 1
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" <SOURCE_DIR>
        -DCMAKE_INSTALL_PREFIX=${PAHO_INSTALL_DIR}
        -DPAHO_WITH_SSL=FALSE
        -DPAHO_BUILD_STATIC=TRUE
        -DPAHO_BUILD_SHARED=FALSE
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
    INSTALL_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target install
)

# ---------------------------------------
# Build Paho MQTT C++
# ---------------------------------------
ExternalProject_Add(paho_mqtt_cpp
    GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
    GIT_TAG        v1.2.0
    UPDATE_COMMAND ""
    DEPENDS paho_mqtt_c
    CMAKE_ARGS
        -DPAHO_MQTT_C_PATH=${PAHO_INSTALL_DIR}
        -DPAHO_MQTT_C_INCLUDE_DIRS=${PAHO_INSTALL_DIR}/include
        -DPAHO_MQTT_C_LIBRARIES=${PAHO_INSTALL_DIR}/lib/paho-mqtt3a.lib
        -DCMAKE_INSTALL_PREFIX=${PAHO_CPP_INSTALL_DIR}
        -DCMAKE_PREFIX_PATH=${PAHO_INSTALL_DIR}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
    INSTALL_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target install
)

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
add_dependencies(myPlugIn paho_mqtt_cpp)

# ---------------------------------------
# Include and Link
# ---------------------------------------
target_include_directories(myPlugIn PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/include
    ${PAHO_INSTALL_DIR}/include
    ${PAHO_CPP_INSTALL_DIR}/include
)

target_link_directories(myPlugIn PRIVATE
    ${PAHO_INSTALL_DIR}/lib
    ${PAHO_CPP_INSTALL_DIR}/lib
    ${CMAKE_SOURCE_DIR}/external/lib
)

target_link_libraries(myPlugIn
    EuroScopePlugInDLL.lib
    ws2_32
    crypt32
    shlwapi
    ${PAHO_INSTALL_DIR}/lib/paho-mqtt3a-static.lib
    ${PAHO_CPP_INSTALL_DIR}/lib/paho-mqttpp3-static.lib
)

# ---------------------------------------
# Definitions
# ---------------------------------------
add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -DSQLITE_THREADSAFE=0
    -DSQLITE_DEFAULT_FILE_FORMAT=4
    -DSQLITE_DEFAULT_SYNCHRONOUS=0
    -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=0
    -DSQLITE_WIN32_MALLOC
)