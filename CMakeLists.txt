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

set(HEADER_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(HEADER_DEST_FILE "${HEADER_DEST_DIR}/stb_image.h")
set(HEADER_DEST_FILE_WRITE "${HEADER_DEST_DIR}/stb_image_write.h")

if(NOT EXISTS "${HEADER_DEST_FILE}")
    message(STATUS "Downloading single header from GitHub...")
    file(DOWNLOAD 
        "https://raw.githubusercontent.com/nothings/stb/4e9b368bfc071e97206ea5c6adba68dd0cbf8897/stb_image.h" 
        "${HEADER_DEST_FILE}"
        SHOW_PROGRESS
        STATUS download_status
    )
    
    # Check for download errors
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        list(GET download_status 1 status_string)
        message(FATAL_ERROR "Failed to download header: ${status_string}")
    endif()
endif()

if(NOT EXISTS "${HEADER_DEST_FILE_WRITE}")
    message(STATUS "Downloading single header from GitHub...")
    file(DOWNLOAD 
        "https://raw.githubusercontent.com/nothings/stb/4e9b368bfc071e97206ea5c6adba68dd0cbf8897/stb_image_write.h" 
        "${HEADER_DEST_FILE_WRITE}"
        SHOW_PROGRESS
        STATUS download_status
    )
    
    # Check for download errors
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        list(GET download_status 1 status_string)
        message(FATAL_ERROR "Failed to download header: ${status_string}")
    endif()
endif()

add_executable(ImageColorReplacer
    src/main/imageColorReplace.cpp
    src/file-io/pngIO.cpp
    src/terminal-io/errors.cpp
    src/graphics/graphicsUtils.cpp
    src/graphics/graphics.cpp
    src/terminal-io/input.cpp
)
target_compile_features(ImageColorReplacer PRIVATE cxx_std_20)
target_include_directories(ImageColorReplacer PRIVATE
    "${WGPU_NATIVE_INCLUDE_DIR}"
)
target_link_libraries(ImageColorReplacer PRIVATE
    "${WGPU_NATIVE_LIBRARY}"
)

target_include_directories(ImageColorReplacer PRIVATE "${HEADER_DEST_DIR}")