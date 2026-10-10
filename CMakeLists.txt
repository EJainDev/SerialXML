cmake_minimum_required(VERSION 4.3.3)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_VERSION VERSION_LESS 4.4)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL 4.4)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()
set(CMAKE_CXX_MODULE_STD 1)

project(
  SerialXML
  VERSION 1.0.0
  LANGUAGES CXX)

include(FetchContent)

find_package(StructuralTuple QUIET)

if(NOT StructuralTuple_FOUND)
  FetchContent_Declare(
    structural_tuple
    GIT_REPOSITORY https://github.com/EJainDev/StructuralTuple.git
    GIT_TAG 7ad88be28ceece1d6d41da72754c6cf2c969fd6d)

  FetchContent_MakeAvailable(structural_tuple)
endif()

option(BUILD_TESTS "Enable building of tests" OFF)
option(BUILD_EXAMPLES "Enable building of examples" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_BENCHMARKS "Enable building of benchmarks" ${PROJECT_IS_TOP_LEVEL})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(BUILD_TESTS)
  include(CTest)
  enable_testing()
endif()

# Library

# StructuralTuple's current release sets reflection flags only in its directory.
# Export them on the fetched target too, including its standard-module
# dependency.
if(TARGET structural_tuple AND NOT StructuralTuple_FOUND)
  target_compile_options(structural_tuple PUBLIC -freflection)
endif()

add_library(serial_xml STATIC)
add_library(serial_xml::serial_xml ALIAS serial_xml)
target_compile_options(serial_xml PUBLIC -freflection)
target_sources(
  serial_xml
  PUBLIC FILE_SET
         modules
         TYPE
         CXX_MODULES
         BASE_DIRS
         "${CMAKE_CURRENT_SOURCE_DIR}/include"
         FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/include/serial_xml.cxx")
target_include_directories(
  serial_xml PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:include>)
if(TARGET structural_tuple::structural_tuple)
  target_link_libraries(serial_xml PUBLIC structural_tuple::structural_tuple)
else()
  target_link_libraries(serial_xml PUBLIC structural_tuple)
endif()
target_compile_features(serial_xml PUBLIC cxx_std_26)

# Installation configuration

install(
  TARGETS serial_xml
  EXPORT serial_xml_targets
  FILE_SET modules
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/serial_xml
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(SerialXML_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/SerialXML")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SerialXMLConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/SerialXMLConfig.cmake"
  INSTALL_DESTINATION "${SerialXML_INSTALL_CMAKEDIR}")

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SerialXMLConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion)

install(
  EXPORT serial_xml_targets
  NAMESPACE serial_xml::
  DESTINATION "${SerialXML_INSTALL_CMAKEDIR}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SerialXMLConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/SerialXMLConfigVersion.cmake"
        DESTINATION "${SerialXML_INSTALL_CMAKEDIR}")

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
