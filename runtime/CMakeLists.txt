add_library(us4_runtime_core
  core/runtime_mode.cpp
  core/hardware_probe.cpp
  core/runtime_context.cpp
  telemetry/telemetry_types.cpp
  telemetry/telemetry_sink.cpp
)

target_include_directories(us4_runtime_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../generated
)

target_compile_features(us4_runtime_core PUBLIC cxx_std_20)

if(US4_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
