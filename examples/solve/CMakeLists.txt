
file(GLOB TEST_CODE
	*.cpp
	)

add_executable(Example_Solve ${TEST_CODE})

target_include_directories(Example_Solve PUBLIC
	../../single/include
	)
