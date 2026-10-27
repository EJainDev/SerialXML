cmake_minimum_required(VERSION 4.3.3)
if(NOT SERIAL_XML_BINARY_DIR)
  message(FATAL_ERROR "Set SERIAL_XML_BINARY_DIR")
endif()
set(diagnostics
    "Cannot have both serial_xml::attribute and serial_xml::unpack"
    "Cannot have both serial_xml::attribute and serial_xml::cdata"
    "Cannot have both serial_xml::attribute and serial_xml::raw"
    "Cannot have both serial_xml::cdata and serial_xml::raw"
    "Cannot have both serial_xml::cdata and serial_xml::unpack"
    "Cannot have both the serial_xml::cdata annotation and iteration"
    "serial_xml::iter annotation can only be applied to range types"
    "No accessible field or const getter for mock member"
    "Ambiguous mock member"
    "Mock member must match a field or a const getter"
    "serial_xml::setter requires exactly one parameter"
    "serial_xml::setter requires a non-static method"
    "serial_xml::setter requires exactly one parameter"
    "Cannot modify XML data member"
    "Ambiguous setter for"
    "No accessible method for mock setter"
    "Invalid setter attribute annotations"
    "Invalid setter CDATA annotations"
    "serial_xml::setter requires a member function"
    "Deserialization requires an owning, writable container"
    "Ambiguous mock setter"
    "Implement serial_xml::from_string<T>(std::string_view) for this leaf type")
file(MAKE_DIRECTORY "${SERIAL_XML_BINARY_DIR}/tests/schema-diagnostics")
foreach(case RANGE 1 22)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${SERIAL_XML_BINARY_DIR}" --target
            "invalid_schema_${case}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    TIMEOUT 120)
  file(WRITE "${SERIAL_XML_BINARY_DIR}/tests/schema-diagnostics/${case}.log"
       "${output}${errors}")
  math(EXPR index "${case} - 1")
  list(GET diagnostics ${index} diagnostic)
  string(FIND "${output}${errors}" "${diagnostic}" found)
  if(NOT result MATCHES "^[1-9][0-9]*$" OR found LESS 0)
    message(
      FATAL_ERROR
        "Schema ${case}: expected compile failure containing '${diagnostic}'. Result: ${result}. See tests/schema-diagnostics/${case}.log"
    )
  endif()
endforeach()
message(STATUS "All 22 invalid schemas rejected with the expected diagnostics")
