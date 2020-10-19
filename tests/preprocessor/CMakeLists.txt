# Creating a custom command to preprocess the library.
# The source file includes all the code we want to preprocess (for later size measurement).
add_custom_command(
  OUTPUT flexclass.i
  COMMAND "${CMAKE_CXX_COMPILER}" -E -P ${CMAKE_CURRENT_SOURCE_DIR}/include_flexclass.cpp -I${CMAKE_CURRENT_SOURCE_DIR}/../../include > flexclass.i
  MAIN_DEPENDENCY include_flexclass.cpp
  COMMENT "Preprocessing include_flexclass.cpp"
  VERBATIM)

add_custom_target(
  preprocess
  DEPENDS flexclass.i
  COMMENT "Preprocessing flexclass library"
  VERBATIM)

