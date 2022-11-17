cmake_minimum_required(VERSION 3.23)
include(rpi-pico-tools.cmake)
set(PICO_TOOLCHAIN_PATH ${CMAKE_SOURCE_DIR}/toolchain)

set(PICO_SDK_FETCH_FROM_GIT on)

include(pico_sdk_import.cmake)
project(hackatum2022-mqtt)

set(CMAKE_C_STANDARD 11)

set(PICO_BOARD pico_w)
pico_sdk_init()


# Add executable.
add_executable(mqtt_led mqtt_led.c)

pico_set_program_name(mqtt_led "mqtt_led")
pico_set_program_version(mqtt_led "0.1")

# Add the standard library to the build
target_link_libraries(mqtt_led PRIVATE pico_stdlib pico_cyw43_arch_lwip_poll)

#App specific stuff
target_include_directories(mqtt_led PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(mqtt_led PRIVATE ${PICO_SDK_PATH}/lib/lwip/src/apps/mqtt/mqtt.c)

pico_add_extra_outputs(mqtt_led)

