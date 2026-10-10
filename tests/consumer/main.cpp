import std;
import serial_xml;

struct Person {
  int age;
  std::string favorite_food;
};

int main() {
  const auto xml = serial_xml::to_xml(Person{3, "pizza"});
  std::print("{}", xml);
  const auto person = serial_xml::from_xml<Person>(xml);
  return person.age == 3 && person.favorite_food == "pizza" ? 0 : 1;
}
