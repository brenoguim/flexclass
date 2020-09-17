set(UNIT_TEST_LIST
  flexclass)
 
foreach(NAME IN LISTS UNIT_TEST_LIST)
  list(APPEND UNIT_TEST_SOURCE_LIST
    ${NAME}.test.cpp)
endforeach()
 
set(TARGET_NAME tests)
 
add_executable(${TARGET_NAME}
  main.cpp
  ${UNIT_TEST_SOURCE_LIST})
 
target_link_libraries(${TARGET_NAME}
  PUBLIC flexclass)

set_target_properties(${TARGET_NAME} PROPERTIES
    INTERFACE_COMPILE_FEATURES cxx_std_20
)

 
target_include_directories(${TARGET_NAME}
  PUBLIC ../external/catch2/)
 
add_test(
  NAME ${TARGET_NAME}
  COMMAND ${TARGET_NAME} -o report.xml -r junit)
