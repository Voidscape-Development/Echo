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

# ASIO (Windows only). JUCE bundles the ASIO SDK headers it needs
# (modules/juce_audio_devices/native/asio), which Steinberg dual-licenses under the
# proprietary Steinberg ASIO License or GPLv3. Echo uses them under GPLv3: AGPLv3 section
# 13 expressly permits combining an AGPLv3 work with GPLv3-licensed code. No separate SDK
# download is needed.
#
# Set ECHO_ENABLE_ASIO=OFF to build without ASIO. To build against your own SDK checkout
# instead of JUCE's bundled headers, also set ASIO_SDK_DIR to a directory containing
# iasiodrv.h (the SDK ships it in common/).
if(WIN32)
  option(ECHO_ENABLE_ASIO "Build with ASIO audio device support" ON)

  if(ECHO_ENABLE_ASIO)
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_ASIO=1)

    if(NOT ASIO_SDK_DIR AND DEFINED ENV{ASIOSDK_DIR})
      set(ASIO_SDK_DIR "$ENV{ASIOSDK_DIR}" CACHE PATH "Path to an external Steinberg ASIO SDK")
    endif()

    if(ASIO_SDK_DIR)
      find_path(ECHO_ASIO_INCLUDE_DIR NAMES iasiodrv.h HINTS "${ASIO_SDK_DIR}" "${ASIO_SDK_DIR}/common" NO_DEFAULT_PATH)

      if(ECHO_ASIO_INCLUDE_DIR)
        message(STATUS "ASIO: using external SDK headers from ${ECHO_ASIO_INCLUDE_DIR}")
        target_include_directories(${PROJECT_NAME} PRIVATE "${ECHO_ASIO_INCLUDE_DIR}")
        target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_ASIO_USE_EXTERNAL_SDK=1)
      else()
        message(
          FATAL_ERROR
          "ASIO_SDK_DIR was set to '${ASIO_SDK_DIR}' but iasiodrv.h was not found there. "
          "Unset ASIO_SDK_DIR to use the ASIO headers bundled with JUCE."
        )
      endif()
    else()
      message(STATUS "ASIO: using the ASIO SDK headers bundled with JUCE")
    endif()
  else()
    message(STATUS "ASIO: disabled (ECHO_ENABLE_ASIO=OFF)")
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_ASIO=0)
  endif()
endif()

# JACK support on Linux is opt-in in JUCE and needs the JACK headers at build time (the
# library itself is dlopened at runtime, so it stays a soft dependency for users).
if(UNIX AND NOT APPLE)
  find_path(ECHO_JACK_INCLUDE_DIR NAMES jack/jack.h)

  if(ECHO_JACK_INCLUDE_DIR)
    message(STATUS "JACK: headers found, enabling JACK device support")
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_JACK=1)
  else()
    message(STATUS "JACK: jack/jack.h not found, building without JACK device support")
    target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_JACK=0)
  endif()
endif()

if(APPLE)
  # The OBS template drives the macOS build through the Xcode generator, so these have to
  # be Xcode build settings on the target: CMake's COMPILE_WARNING_AS_ERROR property and
  # $<COMPILE_LANGUAGE:OBJCXX> options do not reach Xcode's own Objective-C rules (this
  # project enables only C and CXX as languages).
  #
  # ARC: JUCE's .mm sources use manual reference counting, but cmake/macos/xcode.cmake
  # turns ARC on globally.
  # Warnings as errors: same rationale as COMPILE_WARNING_AS_ERROR below - JUCE's sources
  # trip warnings such as -Warc-repeated-use-of-weak that we do not want failing the build.
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO XCODE_ATTRIBUTE_GCC_TREAT_WARNINGS_AS_ERRORS NO
  )
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:OBJC,OBJCXX>:-fno-objc-arc>)
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
