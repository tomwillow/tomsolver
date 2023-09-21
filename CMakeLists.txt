# for FetchContent
cmake_minimum_required (VERSION 3.11 FATAL_ERROR)

# 设置策略CMP0091为NEW，新策略
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

project(TomSolver)

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

if (ENABLE_UNIT_TESTS)
	include(FetchContent)
	FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG release-1.12.1
	)
	FetchContent_MakeAvailable(googletest)
	
	enable_testing()
endif()

file(GLOB SOURCE_CODE
    src/*.cpp
    src/*.h
    )

add_library(TomSolver ${SOURCE_CODE})

# =====================================
file(GLOB TEST_CODE
	test/*.h
	test/*.cpp
	)

add_executable(TomSolverTest ${TEST_CODE})

target_include_directories(TomSolverTest PUBLIC
	src
	)

target_link_libraries(TomSolverTest PUBLIC
	TomSolver
	gtest_main
	)
	
# 设置MT/MTd
#set_property(TARGET TomSolverTest PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(GoogleTest)
gtest_discover_tests(TomSolverTest)