cmake_minimum_required(VERSION 3.20)
project(entropy-bpd VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find zstd
find_library(ZSTD_LIBRARY NAMES zstd libzstd PATHS
    "$ENV{ZSTD_ROOT}/lib" "$ENV{ZSTD_ROOT}/dll"
)
find_path(ZSTD_INCLUDE_DIR NAMES zstd.h PATHS
    "$ENV{ZSTD_ROOT}/include"
)

if(NOT ZSTD_LIBRARY OR NOT ZSTD_INCLUDE_DIR)
    message(FATAL_ERROR "zstd not found. Set ZSTD_ROOT or install libzstd-dev.")
endif()

message(STATUS "ZSTD: ${ZSTD_LIBRARY}")

# Header-only algorithm library
add_library(bpd INTERFACE)
target_include_directories(bpd INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Tools
add_executable(bpd_analyze tools/bpd_analyze.cpp)
target_link_libraries(bpd_analyze bpd)

add_executable(acf_analyze tools/acf_analyze.cpp)
target_link_libraries(acf_analyze bpd)

add_executable(gap_measure tools/gap_measure.cpp)
target_link_libraries(gap_measure bpd)
target_include_directories(gap_measure PRIVATE ${ZSTD_INCLUDE_DIR})
target_link_libraries(gap_measure ${ZSTD_LIBRARY})

# Data generator
add_executable(generate_sensor_log data/generate_sensor_log.cpp)

add_executable(convert_intel_lab data/convert_intel_lab.cpp)

add_executable(extract_itch_adds data/extract_itch_adds.cpp)

# Windows: copy zstd DLL
if(WIN32)
    find_file(ZSTD_DLL NAMES libzstd.dll zstd.dll PATHS
        "$ENV{ZSTD_ROOT}/dll")
    if(ZSTD_DLL)
        add_custom_command(TARGET gap_measure POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ZSTD_DLL}" "$<TARGET_FILE_DIR:gap_measure>")
    endif()
endif()
