import std;
import serial_xml;

// Use [[= serial_xml::name{...}]] to control tag names.
// You can name a struct's root element, attributes, and children.

struct[[= serial_xml::name{"person"}]] Person {
  [[ = serial_xml::name{"age"}, = serial_xml::attribute ]] int age;
  [[= serial_xml::name{"food"}]] std::string favorite_food;
};

struct NamedAttributeAndChild {
  [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute ]] int x;
  [[= serial_xml::name{"MyChild"}]] int y;
};

int main() {
  std::print("Named struct:\n  {}\n", serial_xml::to_xml(Person{3, "pizza"}));
  std::print("Named attribute + child:\n  {}\n", serial_xml::to_xml(NamedAttributeAndChild{4, 5}));
}
