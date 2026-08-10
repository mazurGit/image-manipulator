cmake_minimum_required(VERSION 3.20)

project(image_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(VIPS REQUIRED vips-cpp)

add_library(
    image_engine
    STATIC
    src/color-accumulator.cpp
    src/image.cpp
    src/pixel-view.cpp
    src/timer.cpp
)

add_library(
    image_engine::image_engine
    ALIAS
    image_engine
)

target_include_directories(
    image_engine
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)
target_include_directories(image_engine SYSTEM PUBLIC ${VIPS_INCLUDE_DIRS})
target_compile_options(image_engine PRIVATE ${VIPS_CFLAGS_OTHER})
target_link_directories(image_engine PUBLIC ${VIPS_LIBRARY_DIRS})
target_link_libraries(image_engine PUBLIC ${VIPS_LIBRARIES})
target_link_options(image_engine PUBLIC ${VIPS_LDFLAGS_OTHER})

add_executable(playground
    example/main.cpp
)

target_link_libraries(playground PRIVATE image_engine::image_engine)
