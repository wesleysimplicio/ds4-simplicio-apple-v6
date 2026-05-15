add_executable(us4-cli
  cli/main.cpp
)

target_link_libraries(us4-cli PRIVATE us4_runtime_core)

target_include_directories(us4-cli
  PRIVATE
    ${CMAKE_SOURCE_DIR}/runtime
    ${CMAKE_BINARY_DIR}/generated
)

set_target_properties(us4-cli PROPERTIES OUTPUT_NAME "us4-cli")
