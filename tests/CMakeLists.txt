add_subdirectory(unit)
add_subdirectory(formatting)
add_subdirectory(dependency)

add_library(coverage_config INTERFACE)
# Setup code coverage
if(FLEXCLASS_CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(coverage_config INTERFACE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif(FLEXCLASS_CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")


if(FLEXCLASS_BUILD_PERF_TESTS)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(FATAL_ERROR "Performance tests are meaningless in debug mode")
    endif()
    add_subdirectory(performance)
endif()

add_custom_target(tests DEPENDS
    test.unit
    test.formatting
    test.dependency
)
