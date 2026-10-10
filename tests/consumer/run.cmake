# Run after a library build; all smoke artifacts stay in its ignored build
# directory.
cmake_minimum_required(VERSION 4.3.3)
if(NOT SERIAL_XML_SOURCE_DIR OR NOT SERIAL_XML_BINARY_DIR)
  message(FATAL_ERROR "Set SERIAL_XML_SOURCE_DIR and SERIAL_XML_BINARY_DIR")
endif()
function(run)
  execute_process(COMMAND ${ARGV} RESULT_VARIABLE result)
  if(NOT result STREQUAL "0")
    message(FATAL_ERROR "Consumer check failed: ${ARGV}")
  endif()
endfunction()
set(smoke "${SERIAL_XML_BINARY_DIR}/consumer-smoke")
set(dependency_args)
if(EXISTS "${SERIAL_XML_BINARY_DIR}/_deps/structural_tuple-src")
  list(
    APPEND
    dependency_args
    "-DFETCHCONTENT_SOURCE_DIR_STRUCTURAL_TUPLE=${SERIAL_XML_BINARY_DIR}/_deps/structural_tuple-src"
  )
endif()
run("${CMAKE_COMMAND}"
    -S
    "${SERIAL_XML_SOURCE_DIR}/tests/consumer"
    -B
    "${smoke}/fetch"
    -G
    Ninja
    "-DSERIAL_XML_SOURCE_DIR=${SERIAL_XML_SOURCE_DIR}"
    ${dependency_args})
run("${CMAKE_COMMAND}" --build "${smoke}/fetch" --target consumer)
run("${smoke}/fetch/consumer")
foreach(case RANGE 1 6)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${smoke}/fetch" --target
            invalid_name_${case}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors)
  if(result STREQUAL "0" OR NOT "${output}${errors}" MATCHES "Invalid XML")
    message(
      FATAL_ERROR
        "Expected invalid XML name diagnostic for case ${case}: ${output}${errors}"
    )
  endif()
endforeach()
run("${CMAKE_COMMAND}" --install "${SERIAL_XML_BINARY_DIR}" --prefix
    "${smoke}/prefix")
run("${CMAKE_COMMAND}"
    -S
    "${SERIAL_XML_SOURCE_DIR}/tests/consumer"
    -B
    "${smoke}/installed"
    -G
    Ninja
    "-DCMAKE_PREFIX_PATH=${smoke}/prefix")
run("${CMAKE_COMMAND}" --build "${smoke}/installed" --target consumer)
run("${smoke}/installed/consumer")

# Compile the published Quick Start and FetchContent setup verbatim, using local
# sources.
file(READ "${SERIAL_XML_SOURCE_DIR}/README.md" readme)
string(REGEX MATCH "```cpp\n// main.cpp\n([^`]*)```" quick_start "${readme}")
if(NOT quick_start)
  message(FATAL_ERROR "README Quick Start code block not found")
endif()
file(WRITE "${smoke}/readme/main.cpp" "${CMAKE_MATCH_1}")
string(REGEX MATCH "```cmake\n(cmake_minimum_required[^`]*)```" cmake_example
             "${readme}")
if(NOT cmake_example)
  message(FATAL_ERROR "README CMake code block not found")
endif()
file(WRITE "${smoke}/readme/CMakeLists.txt" "${CMAKE_MATCH_1}")
run("${CMAKE_COMMAND}"
    -S
    "${smoke}/readme"
    -B
    "${smoke}/readme-build"
    -G
    Ninja
    "-DFETCHCONTENT_SOURCE_DIR_SERIAL_XML=${SERIAL_XML_SOURCE_DIR}"
    ${dependency_args})
run("${CMAKE_COMMAND}" --build "${smoke}/readme-build" --target my_app)
run("${smoke}/readme-build/my_app")
