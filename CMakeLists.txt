cmake_minimum_required(VERSION 3.20)
project(entropy-bpd VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Fetch and build zstd from source
include(FetchContent)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB  OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC   ON  CACHE BOOL "" FORCE)
FetchContent_Declare(zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG        v1.5.7
    SOURCE_SUBDIR  build/cmake
)
FetchContent_MakeAvailable(zstd)
set_target_properties(libzstd_static PROPERTIES FOLDER "external/zstd")
set(ZSTD_LINK_TARGET libzstd_static)

# Header-only algorithm library
add_library(bpd INTERFACE)
target_include_directories(bpd INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Tools
add_executable(bpd_analyze tools/bpd_analyze.cpp)
target_link_libraries(bpd_analyze bpd)

add_executable(acf_analyze tools/acf_analyze.cpp)
target_link_libraries(acf_analyze bpd)

add_executable(gap_measure tools/gap_measure.cpp)
target_link_libraries(gap_measure bpd ${ZSTD_LINK_TARGET})

# Data generator
add_executable(generate_sensor_log data/generate_sensor_log.cpp)

add_executable(convert_intel_lab data/convert_intel_lab.cpp)

add_executable(extract_itch_adds data/extract_itch_adds.cpp)

# Install
install(TARGETS bpd_analyze acf_analyze gap_measure
    RUNTIME DESTINATION bin)
install(TARGETS generate_sensor_log convert_intel_lab extract_itch_adds
    RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
