cmake_minimum_required(VERSION 3.9)

include(cmake/Deploy.cmake)

project(Flexclass CXX)

option(FLEXCLASS_BUILD_TESTS "Whether to build the tests" ON)

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
