# AnnoTest uses SerialXML for its reports. Redirect its package lookup to the
# library under test so we neither fetch a second copy nor test a pinned
# release.
file(WRITE "${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/serialxml-config.cmake"
     "set(SerialXML_FOUND TRUE)\n")

# AnnoTest and its dependencies share these option names with SerialXML.
block(SCOPE_FOR VARIABLES)
set(BUILD_TESTS OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_BENCHMARKS OFF)
FetchContent_Declare(
  annotest
  EXCLUDE_FROM_ALL
  GIT_REPOSITORY https://github.com/EJainDev/AnnoTest.git
  GIT_TAG 12a883f5c66d17c27a51b01f1c7eef832b1a474b)
FetchContent_MakeAvailable(annotest)
endblock()

find_package(Threads REQUIRED)
add_executable(serial_xml_tests test_serial_xml.cpp)
target_link_libraries(serial_xml_tests PRIVATE annotest::annotest serial_xml
                                               Threads::Threads)
annotest_discover_tests(serial_xml_tests)

option(SERIAL_XML_TEST_COVERAGE "Instrument the library and tests for gcov" OFF)
if(SERIAL_XML_TEST_COVERAGE)
  target_compile_options(serial_xml PRIVATE --coverage -fprofile-update=atomic)
  target_compile_options(serial_xml_tests PRIVATE --coverage
                                                  -fprofile-update=atomic)
  target_link_options(serial_xml_tests PRIVATE --coverage)
endif()

foreach(case RANGE 1 22)
  add_executable(invalid_schema_${case} EXCLUDE_FROM_ALL invalid_schema.cpp)
  target_compile_definitions(invalid_schema_${case}
                             PRIVATE INVALID_SCHEMA_CASE=${case})
  target_link_libraries(invalid_schema_${case} PRIVATE serial_xml)
endforeach()
add_test(NAME serial_xml_compile_contracts
         COMMAND ${CMAKE_COMMAND} -DSERIAL_XML_BINARY_DIR=${PROJECT_BINARY_DIR}
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/check_invalid_schemas.cmake)
set_tests_properties(serial_xml_compile_contracts PROPERTIES RUN_SERIAL TRUE
                                                             TIMEOUT 600)
