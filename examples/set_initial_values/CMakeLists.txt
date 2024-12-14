
file(GLOB TEST_CODE
	*.cpp
	)

add_executable(Example_SetInitialValues ${TEST_CODE})

target_include_directories(Example_SetInitialValues PUBLIC
	../../single/include
	)
