add_subdirectory(unit)
add_subdirectory(formatting)
add_subdirectory(dependency)

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
