set(UNIT_TEST_LIST
    basic
    sanitizers
)
 
add_library(test_infra
    STATIC
    main.cpp
    allocation_tracker.cpp)

set_target_properties(test_infra PROPERTIES
    INTERFACE_COMPILE_FEATURES cxx_std_17
)

target_include_directories(test_infra
    PUBLIC ../external/catch2/)

foreach(TARGET_NAME IN LISTS UNIT_TEST_LIST)

    add_executable(${TARGET_NAME} 
        ${TARGET_NAME}.test.cpp)
     
    target_link_libraries(${TARGET_NAME}
        PUBLIC flexclass)

    target_link_libraries(${TARGET_NAME}
        PUBLIC test_infra)

    set_target_properties(${TARGET_NAME} PROPERTIES
        INTERFACE_COMPILE_FEATURES cxx_std_17)
     
    target_include_directories(${TARGET_NAME}
        PUBLIC ../external/catch2/)
     
    add_test(
      NAME ${TARGET_NAME}
      COMMAND ${TARGET_NAME} -o report.xml -r junit)

endforeach()

