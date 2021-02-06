cmake_minimum_required(VERSION 3.9)

include(cmake/Deploy.cmake)
include(cmake/Test.cmake)

project(Flexclass CXX)

option(FLEXCLASS_BUILD_TESTS "Whether to build the tests" ON)
option(FLEXCLASS_BUILD_PERF_TESTS "Whether to build the performance tests" OFF)
option(FLEXCLASS_CODE_COVERAGE "Enable coverage reporting" OFF)

# Controls whether the library is bundled into a single header.
# This is disabled when coverage is enabled because would break the processing of
# coverage statistics. Coverage requires keeping sources on their original
# locations in the repository so that we can integrate results with online # platforms.
include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(FLEXCLASS_BUNDLE "Whether to bundle the library into a single header"
                       ON "NOT FLEXCLASS_CODE_COVERAGE" OFF)

set(FLEXCLASS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/flexclass")

set(FLEXCLASS_VERSION "0.1.0")

message(STATUS "Configuring Flexclass ${FLEXCLASS_VERSION}")

add_header_library(flexclass "${FLEXCLASS_INCLUDE_DIR}/flexclass.hpp" "LICENSE")
set_target_properties(flexclass PROPERTIES
    INTERFACE_LIB_VERSION ${FLEXCLASS_VERSION}
    INTERFACE_COMPILE_FEATURES cxx_std_17
)

deploy_header_library(flexclass)

if(FLEXCLASS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
