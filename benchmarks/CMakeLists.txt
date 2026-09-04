find_package(Boost REQUIRED COMPONENTS serialization)
find_package(cereal CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
  pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml
  GIT_TAG v1.16)

FetchContent_MakeAvailable(pugixml)

add_executable(xml_serialization_benchmarks xml_serialization_benchmarks.cpp)
target_link_libraries(
  xml_serialization_benchmarks PRIVATE serial_xml Boost::serialization
                                       cereal::cereal pugixml::pugixml)
