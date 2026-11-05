cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(ImageColorReplacer)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(WGPU_NATIVE_LIBRARY
    NAMES wgpu_native
    REQUIRED
)
find_path(WGPU_NATIVE_INCLUDE_DIR
    NAMES wgpu.h
    REQUIRED
)

add_executable(ImageColorReplacer
    src/main/imageColorReplace.cpp
    src/file-io/ppmIO.cpp
    src/terminal-io/errors.cpp
    src/graphics/graphicsUtils.cpp
    src/graphics/graphics.cpp
)
target_compile_features(ImageColorReplacer PRIVATE cxx_std_20)
target_include_directories(ImageColorReplacer PRIVATE
    "${WGPU_NATIVE_INCLUDE_DIR}"
)
target_link_libraries(ImageColorReplacer PRIVATE
    "${WGPU_NATIVE_LIBRARY}"
)