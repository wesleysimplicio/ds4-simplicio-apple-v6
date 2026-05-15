find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
  find_package(GTest QUIET)
endif()

if(NOT GTest_FOUND)
  message(
    FATAL_ERROR
      "US4_BUILD_TESTS=ON requires an installed GTest package. "
      "Install GoogleTest for your toolchain or configure with -DUS4_BUILD_TESTS=OFF."
  )
endif()

add_subdirectory(unit)
