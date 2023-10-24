
file(GLOB TEST_CODE
	*.cpp
	)

add_executable(DiffMachine ${TEST_CODE})

target_include_directories(DiffMachine PUBLIC
	../../single/include
	)
