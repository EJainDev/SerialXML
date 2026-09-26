#include <iostream>

import std;

import serial_xml;

// The mock describes the XML view and points to the actual accessors.
template <typename T>
struct vector_mock {
  [[
    = serial_xml::accessor<&T::size>{}, = serial_xml::attribute,
    = serial_xml::name{"count"}
  ]] std::size_t size() const;
  [[ = serial_xml::accessor<&T::empty>{}, = serial_xml::name{"is_empty"} ]] bool empty() const;
};

struct document {
  [[= serial_xml::mock<vector_mock<std::vector<int>>>{}]] std::vector<int> values;
};

int main() { std::cout << serial_xml::to_xml(document{{10, 20, 30}}) << '\n'; }
