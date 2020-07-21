cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(CollisionDetectionGJK LANGUAGES CXX)

include(Sanitizers.cmake)

include_directories(include app)
add_compile_options(-Wall -Wextra -Wpedantic)

option(ENABLE_COVERAGE, "Enable coverage." FALSE)

set(CMAKE_CXX_STANDARD 17)

if(ENABLE_COVERAGE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Set the build type." FORCE)
    include(CodeCoverage.cmake)
    append_coverage_compiler_flags()
endif()

add_executable(demo app/demo.cpp app/math.cpp app/rendering.cpp)
add_executable(test_math app/test_math.cpp app/math.cpp)
add_executable(test_gjk app/test_gjk.cpp app/math.cpp)

# Link with GLFW, OpenGL, GLEW
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(demo glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(demo OpenGL::GL)
find_package(GLEW REQUIRED)
target_link_libraries(demo ${GLEW_LIBRARIES})

if(ENABLE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME math_coverage
        EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_math
        EXECUTABLE_ARGS ""
        LCOV_ARGS -rc lcov_branch_coverage=1
        GENHTML_ARGS --branch-coverage
        DEPENDENCIES test_math)

    setup_target_for_coverage_lcov(
        NAME gjk_coverage
        EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/gjk_math
        EXECUTABLE_ARGS ""
        LCOV_ARGS -rc lcov_branch_coverage=1
        GENHTML_ARGS --branch-coverage
        DEPENDENCIES gjk_math)
endif()
