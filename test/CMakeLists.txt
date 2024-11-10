# =====================================
file(GLOB TEST_CODE
    *.h
    *.cpp
)

add_executable(TomSolverTest ${TEST_CODE})

target_include_directories(TomSolverTest PUBLIC
    ../src
)

target_link_libraries(TomSolverTest PUBLIC
    TomSolver
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(TomSolverTest)