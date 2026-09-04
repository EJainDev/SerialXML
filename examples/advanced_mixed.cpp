import std;
import serial_xml;

// A complex example combining multiple features:
// attributes, named tags, iteration, nesting, and escaping.

struct Address {
  std::string street;
  [[= serial_xml::name{"city_name"}]] std::string city;
  int zip_code;
};

struct Employee {
  [[ = serial_xml::name{"emp_id"}, = serial_xml::attribute ]] int id;
  std::string name;
  Address address;
  [[= serial_xml::iter{"skill", "skills"}]] std::vector<std::string> skills;
};

struct Department {
  [[ = serial_xml::name{"dept_name"}, = serial_xml::attribute ]] std::string name;
  [[= serial_xml::name{"team"}]] std::vector<Employee> employees;
};

int main() {
  std::vector<Employee> team = {
      Employee{1, "Alice", Address{"123 Main St", "Boston", 2128}, {"C++", "Python"}},
      Employee{2, "Bob", Address{"456 Oak Ave", "Cambridge", 2140}, {"Rust", "Go"}}};

  Department dept{"Engineering", team};

  std::print("Combined complex example:\n");
  std::print("  {}\n\n", serial_xml::to_xml(dept));
}
