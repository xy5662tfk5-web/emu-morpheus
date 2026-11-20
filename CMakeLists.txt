cmake_minimum_required(VERSION 3.20)
project(emu_morpheus VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(morpheus_core
    src/core/MorpheusCore.cpp
)
target_include_directories(morpheus_core PUBLIC include)
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
