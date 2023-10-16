# for FetchContent
cmake_minimum_required (VERSION 3.11 FATAL_ERROR)

project(TomSolver)

# C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/W4 /WX)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
message(STATUS "Enable testing: ${ENABLE_UNIT_TESTS}")

if (ENABLE_UNIT_TESTS)
	# 添加参数：是否从镜像站下载googletest
	option(USE_MIRROR_GTEST_REPO "Use mirror google test repository at gitcode.net" OFF)
	if (USE_MIRROR_GTEST_REPO)
		set(GTEST_URL "https://gitcode.net/mirrors/google/googletest.git")
	else()
		set(GTEST_URL "https://github.com/google/googletest.git")
	endif()
	
	message(STATUS "Fetch googletest from ${GTEST_URL}")
	
	include(FetchContent)
	FetchContent_Declare(
		googletest
		GIT_REPOSITORY ${GTEST_URL}
		GIT_TAG release-1.12.1
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
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

include(GoogleTest)
gtest_discover_tests(TomSolverTest)

# =====================================
add_subdirectory(single)