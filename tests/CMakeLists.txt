add_subdirectory(unit)
add_subdirectory(formatting)

add_custom_target(tests DEPENDS
    test.unit
    test.formatting
)
