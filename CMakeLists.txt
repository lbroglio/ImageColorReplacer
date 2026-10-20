cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(ImageColorReplacer)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

FetchContent_Declare(
    webgpu-header
    GIT_REPOSITORY https://github.com/eliemichel/WebGPU-Cpp.git
    GIT_TAG wgpu-v24.0.3.1
)
FetchContent_MakeAvailable(webgpu-header)

# webgpu.hpp expects the C headers beneath a webgpu/ include directory.
set(WEBGPU_HEADERS_DIR "${CMAKE_BINARY_DIR}/_deps/webgpu-headers")
file(MAKE_DIRECTORY "${WEBGPU_HEADERS_DIR}/webgpu")
file(COPY
    "${webgpu-header_SOURCE_DIR}/wgpu-native/webgpu.h"
    "${webgpu-header_SOURCE_DIR}/wgpu-native/wgpu.h"
    DESTINATION "${WEBGPU_HEADERS_DIR}/webgpu"
)

add_library(WebGPU_CPP INTERFACE)
target_include_directories(WebGPU_CPP INTERFACE
    "${webgpu-header_SOURCE_DIR}/wgpu-native"
    ${WEBGPU_HEADERS_DIR}
)

add_executable(ImageColorReplacer
    src/main/imageColorReplace.cpp
    src/file-io/ppmIO.cpp
)
target_compile_features(ImageColorReplacer PRIVATE cxx_std_20)
target_link_libraries(ImageColorReplacer PRIVATE WebGPU_CPP)