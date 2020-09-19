set(UNIT_TEST_LIST
    basic 
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

function(make_test_lib _target_name _suffix)
    set(target_name ${_target_name}${_suffix})
    add_executable(${target_name} ${_target_name}.test.cpp)
    target_link_libraries(${target_name} PUBLIC flexclass)
    target_link_libraries(${target_name} PUBLIC test_infra)
    set_target_properties(${target_name} PROPERTIES INTERFACE_COMPILE_FEATURES cxx_std_17)
    target_include_directories(${target_name} PUBLIC ../external/catch2/)
    add_test(
      NAME ${target_name}
      COMMAND env ASAN_OPTIONS=print_stacktrace=1:halt_on_error=1 UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ./${target_name} -o report.xml -r junit)
endfunction()

foreach(TARGET_NAME IN LISTS UNIT_TEST_LIST)
    make_test_lib(${TARGET_NAME} _asan)
    make_test_lib(${TARGET_NAME} _ubsan)

    target_link_options(${TARGET_NAME}_ubsan PRIVATE -fsanitize=undefined)
    target_compile_options(${TARGET_NAME}_ubsan PRIVATE -fsanitize=undefined)

    target_link_options(${TARGET_NAME}_asan PRIVATE -fsanitize=address)
    target_compile_options(${TARGET_NAME}_asan PRIVATE -fsanitize=address)
endforeach()

make_test_lib(memory "")
