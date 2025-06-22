cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

project(remoteprog-test C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(remoteprog-test test.c)

# pull in common dependencies
target_link_libraries(remoteprog-test pico_stdlib pico_time hardware_spi)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(remoteprog-test pico_cyw43_arch_none)
endif()

# create map/bin/hex file etc.
pico_add_extra_outputs(remoteprog-test)
