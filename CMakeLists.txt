cmake_minimum_required(VERSION 3.28)
include("cmake/warnings.cmake")

project(craftmine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(CMAKE_BUILD_TYPE MATCHES "Release"
       OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo"
       OR CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    add_compile_definitions(NDEBUG)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION On)
endif()

add_executable(
	craftmine
	"src/scripts/player.cpp"
	"src/world/chunk.cpp"
	"src/application.cpp"
	"src/main_layer.cpp"
	"src/main_scene.cpp"
	"src/assets.cpp"
)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*")
	target_link_libraries(craftmine PRIVATE -lstdc++exp)
endif()

get_compile_warnings(CRAFTMINE_COMPILE_WARNINGS)

target_include_directories(craftmine PRIVATE "src")
target_compile_features(craftmine PRIVATE cxx_std_23)
target_compile_options(craftmine PRIVATE ${CRAFTMINE_COMPILE_WARNINGS})
target_precompile_headers(craftmine PRIVATE "src/pch.hpp")
set_property(TARGET craftmine PROPERTY COMPILE_WARNING_AS_ERROR On)

add_subdirectory("dependencies/Zenith")

target_link_libraries(craftmine PRIVATE zenith)
b_embed(craftmine "assets/textures/blocks.png")
