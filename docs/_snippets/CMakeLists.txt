cmake_minimum_required(VERSION 4.3.3)
# import std is experimental; its opt-in key depends on the CMake version.
if(CMAKE_VERSION VERSION_LESS 4.4)
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
else()
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_MODULE_STD ON)
project(my_app LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    serial_xml
    GIT_REPOSITORY https://github.com/EJainDev/SerialXML.git
    GIT_TAG main
)
FetchContent_MakeAvailable(serial_xml)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE serial_xml::serial_xml)
