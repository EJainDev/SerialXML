add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE serial_xml)

add_executable(attributes attributes.cpp)
target_link_libraries(attributes PRIVATE serial_xml)

add_executable(named_tags named_tags.cpp)
target_link_libraries(named_tags PRIVATE serial_xml)

add_executable(skip_members skip_members.cpp)
target_link_libraries(skip_members PRIVATE serial_xml)

add_executable(nested_structs nested_structs.cpp)
target_link_libraries(nested_structs PRIVATE serial_xml)

add_executable(stl_containers stl_containers.cpp)
target_link_libraries(stl_containers PRIVATE serial_xml)

add_executable(raw_cdata raw_cdata.cpp)
target_link_libraries(raw_cdata PRIVATE serial_xml)

add_executable(format_specifiers format_specifiers.cpp)
target_link_libraries(format_specifiers PRIVATE serial_xml)

add_executable(iteration iteration.cpp)
target_link_libraries(iteration PRIVATE serial_xml)

add_executable(escaping escaping.cpp)
target_link_libraries(escaping PRIVATE serial_xml)

add_executable(advanced_mixed advanced_mixed.cpp)
target_link_libraries(advanced_mixed PRIVATE serial_xml)

add_executable(optional_types optional_types.cpp)
target_link_libraries(optional_types PRIVATE serial_xml)
