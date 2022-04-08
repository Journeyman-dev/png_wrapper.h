cmake_minimum_required(VERSION 3.19)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
endif()

option(PNGW_BUILD_EXAMPLES "Build the example projects" OFF)

PROJECT(png_wrapper.h
	VERSION 0.0
	DESCRIPTION "A header only libpng wrapper library."
	LANGUAGES C
)

set(PNGW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)

if (PNGW_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()