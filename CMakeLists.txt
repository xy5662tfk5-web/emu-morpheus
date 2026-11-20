cmake_minimum_required(VERSION 3.20)
project(emu_morpheus VERSION 0.3.0 LANGUAGES C CXX)

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Moira is an MIT-licensed Motorola 68k CPU core. Pin an exact upstream
# revision so builds are reproducible.
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

target_compile_definitions(moira PUBLIC MOIRA_VIRTUAL_API=1)
target_include_directories(moira PUBLIC ${moira_SOURCE_DIR}/Moira)

add_library(morpheus_core
    src/core/MorpheusCore.cpp
    src/core/SysExPresetBank.cpp
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

    add_executable(morpheus_sysex_tests tests/SysExPresetBankTests.cpp)
    target_link_libraries(morpheus_sysex_tests PRIVATE morpheus_core)
    add_test(NAME morpheus_sysex_tests COMMAND morpheus_sysex_tests)
endif()

# JUCE is used only for the host-facing AU shell. It does not emulate E-MU
# hardware; the current sound is an explicitly labelled MIDI test oscillator.
if(APPLE)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)

    juce_add_plugin(MorpheusLab
        COMPANY_NAME "Open Research"
        BUNDLE_ID "org.openresearch.morpheuslab"
        PLUGIN_MANUFACTURER_CODE ORes
        PLUGIN_CODE Mrph
        PRODUCT_NAME "Morpheus Lab"
        FORMATS AU
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
    )

    target_sources(MorpheusLab PRIVATE
        plugin/PluginProcessor.cpp
        plugin/PluginEditor.cpp
    )
    juce_generate_juce_header(MorpheusLab)

    target_compile_definitions(MorpheusLab PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
    )
    target_link_libraries(MorpheusLab PRIVATE
        morpheus_core
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )
endif()
