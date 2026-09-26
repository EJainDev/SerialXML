#include <iostream>

import std;

import serial_xml;

// This mock describes the XML view of std::vector without changing std::vector.
// The declarations need no definitions: SerialXML calls the matching functions
// on the actual std::vector instance.
struct vector {
  [[ = serial_xml::attribute, = serial_xml::name{"count"} ]] std::size_t size() const;
  [[= serial_xml::name{"is_empty"}]] bool empty() const;
};

template <typename T, typename Allocator>
struct serial_xml::mock_type<std::vector<T, Allocator>> {
  using type = vector;
};

struct document {
  std::vector<int> values;
};

int main() { std::cout << serial_xml::to_xml(document{{10, 20, 30}}) << '\n'; }
