add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/tensorflow_src/tensorflow/lite 
  ${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite EXCLUDE_FROM_ALL
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tensorflow_src/lite)