# Single source-tree entry point for the plugin sources.
#
# src/core/ is vendored from the atkAudio Plugin for OBS (AGPLv3) and is kept as close to
# upstream as possible so that future upstream fixes can be diffed in.

file(GLOB_RECURSE ECHO_PLUGIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${ECHO_PLUGIN_SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/core ${CMAKE_BINARY_DIR}
)

# JUCE is consumed as modules only; no juce_add_* helper targets are needed.
set(JUCE_MODULES_ONLY ON CACHE BOOL "Only build JUCE modules" FORCE)

# Persist JUCE across build-directory resets by reusing a checked-out source tree.
set(_juce_source_dir "${CMAKE_SOURCE_DIR}/_deps/juce-src")
if(EXISTS "${_juce_source_dir}/CMakeLists.txt")
  set(FETCHCONTENT_SOURCE_DIR_JUCE "${_juce_source_dir}" CACHE PATH "Source directory for JUCE" FORCE)
  message(STATUS "Using existing JUCE checkout at ${FETCHCONTENT_SOURCE_DIR_JUCE}")
endif()

include(FetchContent)
FetchContent_Declare(
  juce
  EXCLUDE_FROM_ALL
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 9.0.2
  GIT_SHALLOW TRUE
  PATCH_COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/cmake/patches/apply-juce-patches.cmake"
)
FetchContent_MakeAvailable(juce)

echo_suppress_juce_target_warnings(${PROJECT_NAME})
echo_apply_juce_recommended_flags(${PROJECT_NAME})
echo_apply_msvc_juce_warning_overrides(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE juce::juce_audio_utils juce::juce_gui_extra juce::juce_dsp)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_WIN_PER_MONITOR_DPI_AWARE=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    $<$<AND:$<CONFIG:RelWithDebInfo>,$<OR:$<BOOL:$ENV{CI}>,$<BOOL:$ENV{GITHUB_ACTIONS}>>>:JUCE_DISABLE_ASSERTIONS>
)

set_target_properties(${PROJECT_NAME} PROPERTIES JUCE_NEEDS_WEB_BROWSER FALSE JUCE_NEEDS_CURL FALSE)

# ASIO support needs the Steinberg ASIO SDK, which cannot be redistributed. Point
# ASIO_SDK_DIR (or the ASIOSDK_DIR environment variable) at a directory containing
# common/iasiodrv.h to build with ASIO; otherwise Windows falls back to WASAPI/DirectSound.
if(WIN32)
  if(NOT ASIO_SDK_DIR AND DEFINED ENV{ASIOSDK_DIR})
    set(ASIO_SDK_DIR "$ENV{ASIOSDK_DIR}" CACHE PATH "Path to the Steinberg ASIO SDK")
  endif()

  if(ASIO_SDK_DIR AND EXISTS "${ASIO_SDK_DIR}/common/iasiodrv.h")
    message(STATUS "ASIO SDK found at ${ASIO_SDK_DIR}; enabling ASIO support")
    target_include_directories(${PROJECT_NAME} PRIVATE "${ASIO_SDK_DIR}/common")
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_ASIO=1)
  else()
    message(STATUS "ASIO SDK not found (set ASIO_SDK_DIR to enable ASIO); building without ASIO support")
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_ASIO=0)
  endif()
endif()

# The plugin target compiles JUCE's bundled third-party sources (FLAC, HarfBuzz, ...),
# which do not build warning-free. Keeping warnings-as-errors on for them would break the
# build on every JUCE or toolchain update, so it is disabled for this target only.
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_WARNING_AS_ERROR OFF)

if(NOT WIN32)
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-newline-eof
  )
endif()
