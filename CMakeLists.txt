cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

# Keep FetchContent downloads and source trees under the repository's _deps directory so
# they survive build-directory resets.
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/_deps" CACHE PATH "FetchContent download directory" FORCE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

include(compilerconfig)
include(defaults)
include(helpers)
include(target_policies)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

# The frontend API is required: both filters follow scene/transition state.
find_package(obs-frontend-api REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENABLE_FRONTEND_API)

# Qt is required: JUCE windows are parented to the OBS main window and the JUCE message
# loop is pumped from a Qt timer.
find_package(Qt6 COMPONENTS Widgets Core REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
target_compile_options(
  ${CMAKE_PROJECT_NAME}
  PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
)
set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

file(READ "${CMAKE_SOURCE_DIR}/buildspec.json" buildspec)
string(JSON PLUGIN_DISPLAY_NAME GET ${buildspec} displayName)
string(JSON PLUGIN_OBS_VERSION_REQUIRED GET ${buildspec} dependencies obs-studio version)
string(TIMESTAMP PLUGIN_YEAR "%Y")

configure_file(src/config.h.in config.h @ONLY)

add_subdirectory(src)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
