add_executable(dense_baseline
  dense_baseline.cpp
)

target_link_libraries(dense_baseline PRIVATE us4_runtime_core)

target_include_directories(dense_baseline
  PRIVATE
    ${CMAKE_SOURCE_DIR}/runtime
    ${CMAKE_BINARY_DIR}/generated
)
