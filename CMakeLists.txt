cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

# Pico W Stuff
if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping Pico W examples as support is not available")
    else()

        if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
            set(WIFI_SSID $ENV{WIFI_SSID})
            message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
        endif()

        if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
            set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
            message("Using WIFI_PASSWORD from environment")
        endif()

        set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
        set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")
    endif()
endif()

include_directories (${CMAKE_SOURCE_DIR}/src)

add_compile_options(-Wall
    -Wno-format
    -Wno-unused-function
    -Wno-maybe-uninitialized
)

add_executable(server
        src/main.c
        src/server.c
)
target_compile_definitions(server PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)
target_include_directories(server PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)
target_link_libraries(server
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
)

pico_add_extra_outputs(server)
