cmake_minimum_required(VERSION 3.20)
project(emu_morpheus VERSION 0.2.0 LANGUAGES CXX)

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Moira is an MIT-licensed Motorola 68k CPU core. Pin an exact upstream
# revision so builds are reproducible. Only its CPU library is added; the
# upstream test runner and third-party test dependencies are not included.
FetchContent_Declare(
    moira
    GIT_REPOSITORY https://github.com/dirkwhoffmann/Moira.git
    GIT_TAG ce28239b0507ebfe1bf2ad2b9c9252dc2f2cc031
    GIT_SHALLOW FALSE
)
FetchContent_GetProperties(moira)
if(NOT moira_POPULATED)
    FetchContent_Populate(moira)
endif()
add_subdirectory(${moira_SOURCE_DIR}/Moira ${moira_BINARY_DIR}/Moira)

# Enable Moira's bus-delegate API so the Morpheus memory map can be supplied
# by our machine implementation instead of being compiled into the CPU core.
target_compile_definitions(moira PUBLIC MOIRA_VIRTUAL_API=1)
target_include_directories(moira PUBLIC ${moira_SOURCE_DIR}/Moira)

add_library(morpheus_core
    src/core/MorpheusCore.cpp
    src/cpu/Cpu68020.cpp
)
target_include_directories(morpheus_core PUBLIC include)
target_link_libraries(morpheus_core PUBLIC moira)
target_compile_features(morpheus_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(morpheus_core PRIVATE /W4)
else()
    target_compile_options(morpheus_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(morpheus_core_tests tests/CoreTests.cpp)
    target_link_libraries(morpheus_core_tests PRIVATE morpheus_core)
    add_test(NAME morpheus_core_tests COMMAND morpheus_core_tests)
endif()
