cmake_minimum_required(VERSION 3.20)

project(
  fastlex
  VERSION 0.1.0
  DESCRIPTION "Branchless constexpr ASCII character classification"
  LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(FASTLEX_BUILD_TESTS "Build fastlex tests" ${PROJECT_IS_TOP_LEVEL})
option(FASTLEX_BUILD_BENCHMARKS "Build fastlex benchmarks" ${PROJECT_IS_TOP_LEVEL})
option(FASTLEX_BUILD_EXAMPLES "Build fastlex examples" ${PROJECT_IS_TOP_LEVEL})

add_library(fastlex INTERFACE)
add_library(fastlex::fastlex ALIAS fastlex)

target_include_directories(
  fastlex
  INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(fastlex INTERFACE cxx_std_23)

if(FASTLEX_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(FASTLEX_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(FASTLEX_BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()

install(
  TARGETS fastlex
  EXPORT fastlexTargets
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT fastlexTargets
  FILE fastlexTargets.cmake
  NAMESPACE fastlex::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fastlex)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/fastlexConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  cmake/fastlexConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/fastlexConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fastlex)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/fastlexConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/fastlexConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fastlex)
