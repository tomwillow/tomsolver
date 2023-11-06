
file(GLOB TEST_CODE
	*.cpp
	)

add_executable(Example_Solve2 ${TEST_CODE})

target_include_directories(Example_Solve2 PUBLIC
	../../single/include
	)
