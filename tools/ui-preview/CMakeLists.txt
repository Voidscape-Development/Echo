# Standalone offscreen renderer for the Device IO windows. Kept out of the plugin build on
# purpose: it is a development tool, not something that ships.
#
#   cmake -S tools/ui-preview -B build-ui-preview -G Ninja
#   cmake --build build-ui-preview
#   ./build-ui-preview/ui-preview_artefacts/ui-preview <output-dir>

cmake_minimum_required(VERSION 3.28)
project(ui-preview VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_MODULES_ONLY OFF CACHE BOOL "" FORCE)

# Reuse the JUCE checkout the plugin build already fetched when it is there.
set(_juce_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../_deps/juce-src")

if(EXISTS "${_juce_dir}/CMakeLists.txt")
  message(STATUS "ui-preview: using JUCE from ${_juce_dir}")
  add_subdirectory("${_juce_dir}" juce-build EXCLUDE_FROM_ALL)
else()
  include(FetchContent)
  FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.2
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
  )
  FetchContent_MakeAvailable(juce)
endif()

juce_add_console_app(ui-preview)

target_include_directories(ui-preview PRIVATE ../../src)

target_sources(ui-preview PRIVATE preview.cpp ../../src/ui/EchoLookAndFeel.cpp)

target_compile_definitions(
  ui-preview
  PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_JACK=0
    JUCE_ALSA=0
)

target_link_libraries(
  ui-preview
  PRIVATE juce::juce_audio_utils juce::juce_gui_extra juce::juce_recommended_config_flags
)
