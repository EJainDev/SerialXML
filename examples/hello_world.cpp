import std;
import serial_xml;

struct Person {
  int age;
  std::string favorite_food;
};

int main() {
  const auto xml = serial_xml::to_xml(Person{3, "pizza"});
  std::print("{}\n", serial_xml::prettify(xml));
}
