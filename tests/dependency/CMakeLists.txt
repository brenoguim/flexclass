add_executable(test.dependency main.test.cpp)
target_link_libraries(test.dependency PUBLIC flexclass test_infra)
set_target_properties(test.dependency PROPERTIES INTERFACE_COMPILE_FEATURES cxx_std_17)

add_test(
  NAME test.dependency
  COMMAND ./test.dependency)
