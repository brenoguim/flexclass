add_subdirectory(unit)
add_subdirectory(formatting)
add_subdirectory(dependency)

add_custom_target(tests DEPENDS
    test.unit
    test.formatting
    test.dependency
)
