# for FetchContent
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(tomsolver)

# C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	add_compile_options(/W4 /WX)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# =====================================
option(BUILD_TESTS "Build tests" ON)
message(STATUS "Option: BUILD_TESTS=${BUILD_TESTS}")

option(BUILD_EXAMPLES "Build examples" ON)
message(STATUS "Option: BUILD_EXAMPLES=${BUILD_EXAMPLES}")

# =====================================
if(BUILD_TESTS)
	# 添加参数：是否从镜像站下载googletest
	option(USE_MIRROR_GTEST_REPO "Use mirror google test repository at gitcode.net" OFF)

	if(USE_MIRROR_GTEST_REPO)
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

	add_subdirectory(tests)
	add_subdirectory(single/test)
endif()

# =====================================
file(GLOB_RECURSE SOURCE_CODE
	src/*.cpp
	src/*.h
)

add_library(tomsolver STATIC ${SOURCE_CODE})

target_include_directories(tomsolver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>  
    $<INSTALL_INTERFACE:include/>
)

# =====================================
if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# =====================================
# Install the library and its headers
install(TARGETS tomsolver
        EXPORT tomsolver_targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY src/
		DESTINATION include/
		FILES_MATCHING PATTERN "*.h"
		)

# Generate and install *-targets.cmake 
install(EXPORT tomsolver_targets
        FILE tomsolver-targets.cmake
        NAMESPACE tomsolver::
        DESTINATION share/tomsolver)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/tomsolver-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/tomsolver-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tomsolver-config.cmake"
        DESTINATION share/tomsolver)
