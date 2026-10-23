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

add_executable(serial_xml_tests test_serial_xml.cpp)
target_link_libraries(serial_xml_tests PRIVATE annotest::annotest serial_xml)
annotest_discover_tests(serial_xml_tests)
