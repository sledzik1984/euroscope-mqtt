cmake_minimum_required(VERSION 3.14)

project(euroscope-mqtt VERSION "1.0.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force 32-bit (EuroScope is 32-bit)
if (MSVC)
    set(CMAKE_GENERATOR_PLATFORM Win32)
    add_definitions(/D_USRDLL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP /sdl /permissive- /DNOMINMAX")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /W4 /MP /sdl /permissive- /DNOMINMAX")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFESTUAC:NO /ignore:4099")
endif()

include(ExternalProject)

# --- Build paho.mqtt.c ---
ExternalProject_Add(paho_mqtt_c
    GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
    GIT_TAG        master
    CMAKE_ARGS     -DPAHO_WITH_SSL=FALSE -DPAHO_BUILD_SHARED=FALSE -DPAHO_BUILD_STATIC=TRUE -DPAHO_ENABLE_TESTING=FALSE -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/_install/paho_mqtt_c
    UPDATE_COMMAND ""
    INSTALL_COMMAND ${MAKE} install
    STEP_TARGETS build
)

# --- Build paho.mqtt.cpp ---
ExternalProject_Add(paho_mqtt_cpp
    GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
    GIT_TAG        master
    CMAKE_ARGS     -DPAHO_MQTT_C_PATH=${CMAKE_BINARY_DIR}/_install/paho_mqtt_c -DPAHO_BUILD_STATIC=TRUE -DPAHO_BUILD_SHARED=FALSE -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/_install/paho_mqtt_cpp
    DEPENDS        paho_mqtt_c
    UPDATE_COMMAND ""
    INSTALL_COMMAND ${MAKE} install
    STEP_TARGETS build
)

# --- Version header ---
configure_file(
    ${CMAKE_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_BINARY_DIR}/Version.h
)

# --- Plugin sources ---
set(SOURCE_FILES
    src/plugin.cpp
    src/plugin.h
    src/main.cpp
    ${CMAKE_BINARY_DIR}/Version.h
)

add_library(myPlugIn SHARED ${SOURCE_FILES})
set_target_properties(myPlugIn PROPERTIES OUTPUT_NAME "euroscope-mqtt")

target_include_directories(myPlugIn PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_BINARY_DIR}/_install/paho_mqtt_cpp/include
    ${CMAKE_BINARY_DIR}/_install/paho_mqtt_c/include
)

target_link_directories(myPlugIn PRIVATE
    ${CMAKE_BINARY_DIR}/_install/paho_mqtt_cpp/lib
    ${CMAKE_BINARY_DIR}/_install/paho_mqtt_c/lib
)

target_link_libraries(myPlugIn
    ${CMAKE_SOURCE_DIR}/external/lib/EuroScopePlugInDLL.lib
    ws2_32
    crypt32
    shlwapi
    paho-mqttpp3
    paho-mqtt3as
)

add_dependencies(myPlugIn paho_mqtt_cpp paho_mqtt_c)

add_definitions(
    -D_CRT_SECURE_NO_WARNINGS
    -DSQLITE_THREADSAFE=0
    -DSQLITE_DEFAULT_FILE_FORMAT=4
    -DSQLITE_DEFAULT_SYNCHRONOUS=0
    -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=0
    -DSQLITE_WIN32_MALLOC
)
