
file(GLOB TEST_CODE
	include/tomsolver/*.hpp
	test/*.cpp
	)

add_executable(TomSolverSingleTest ${TEST_CODE})

target_include_directories(TomSolverSingleTest PUBLIC
	include
	)

target_link_libraries(TomSolverSingleTest PUBLIC
	gtest_main
	)

include(GoogleTest)
gtest_discover_tests(TomSolverSingleTest)