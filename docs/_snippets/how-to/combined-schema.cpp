import std;
import serial_xml;

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
  const Employee employee{1, "Alice", {"123 Main St", "Boston", 2128}, {"C++", "Python"}};
  const Department original{"Engineering", {employee}};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", serial_xml::prettify(xml));
  const auto restored = serial_xml::from_xml<Department>(xml);
  const auto& first = restored.employees.front();
  std::println("Department: {}; employee: {}; city: {}; skills: {}", restored.name, first.name,
               first.address.city, first.skills.size());
}
