add_custom_target(test.performance)

set(PERF_TEST_LIST
    graph
)

function(make_perf_test _target_name)
    set(target_name ${_target_name})
    add_executable(${target_name} ${_target_name}.test.cpp)
    target_link_libraries(${target_name} PUBLIC flexclass)
    set_target_properties(${target_name} PROPERTIES INTERFACE_COMPILE_FEATURES cxx_std_17)
    target_include_directories(${target_name} PUBLIC ../../external/catch2/)
    add_test(
      NAME performance.${target_name}
      COMMAND ./${target_name} -o report.xml -r junit)

    add_dependencies(test.performance ${target_name})
endfunction()

foreach(TARGET_NAME IN LISTS PERF_TEST_LIST)
    make_perf_test(${TARGET_NAME})
endforeach()
