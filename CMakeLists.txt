cmake_minimum_required(VERSION 3.9)

include(cmake/Deploy.cmake)
#include(cmake/Test.cmake)

project(Flexclass CXX)

#option(FLEXCLASS_BUILD_EXAMPLES "Whether to build the examples" ON)
#option(FLEXCLASS_BUILD_TESTS "Whether to build the tests" ON)

set(FLEXCLASS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/flexclass")

#file(STRINGS "${FLEXCLASS_INCLUDE_DIR}/metal/config/version.hpp"
#    FLEXCLASS_CONFIG_VERSION_HPP REGEX "FLEXCLASS_[A-Z]+ [0-9]+" LIMIT_COUNT 3
#)

#list(GET FLEXCLASS_CONFIG_VERSION_HPP 0 FLEXCLASS_MAJOR)
#list(GET FLEXCLASS_CONFIG_VERSION_HPP 1 FLEXCLASS_MINOR)
#list(GET FLEXCLASS_CONFIG_VERSION_HPP 2 FLEXCLASS_PATCH)

#string(REGEX REPLACE ".*MAJOR ([0-9]+).*" "\\1" FLEXCLASS_MAJOR "${FLEXCLASS_MAJOR}")
#string(REGEX REPLACE ".*MINOR ([0-9]+).*" "\\1" FLEXCLASS_MINOR "${FLEXCLASS_MINOR}")
#string(REGEX REPLACE ".*PATCH ([0-9]+).*" "\\1" FLEXCLASS_PATCH "${FLEXCLASS_PATCH}")

#set(FLEXCLASS_VERSION "${FLEXCLASS_MAJOR}.${FLEXCLASS_MINOR}.${FLEXCLASS_PATCH}")
set(FLEXCLASS_VERSION "0.1.0")

message(STATUS "Configuring Flexclass ${FLEXCLASS_VERSION}")

add_header_library(Flexclass "${FLEXCLASS_INCLUDE_DIR}/flexclass.hpp" "LICENSE")
set_target_properties(Flexclass PROPERTIES
    INTERFACE_LIB_VERSION ${FLEXCLASS_VERSION}
    INTERFACE_COMPILE_FEATURES cxx_std_20
)

deploy_header_library(Flexclass)

#if(FLEXCLASS_BUILD_DOC)
#    add_subdirectory(doc)
#endif()
#if (FLEXCLASS_BUILD_EXAMPLES)
#    add_subdirectory(example)
#endif()
#if(FLEXCLASS_BUILD_TESTS)
#    add_subdirectory(test)
#endif()
