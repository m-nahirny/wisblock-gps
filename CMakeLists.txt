cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(wisblock_gps C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(wisblock_gps
        wisblock_gps.c
)

pico_enable_stdio_usb(wisblock_gps 0)
pico_enable_stdio_uart(wisblock_gps 1)
        
# create map/bin/hex file etc.
pico_add_extra_outputs(wisblock_gps)

# add url via pico_set_program_url
#example_auto_set_url(blink)

# pull in common dependencies
target_link_libraries(wisblock_gps pico_stdlib)
