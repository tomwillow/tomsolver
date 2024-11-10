# =====================================
file(GLOB TEST_CODE
    *.h
    *.cpp
)

add_executable(tomsolver_tests ${TEST_CODE})

target_include_directories(tomsolver_tests PUBLIC
    ../src
)

target_link_libraries(tomsolver_tests PUBLIC
    tomsolver
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(tomsolver_tests)