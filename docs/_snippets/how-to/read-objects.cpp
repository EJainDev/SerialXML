import std;
import serial_xml;

struct Person {
  [[= serial_xml::attribute]] int age;
  std::string name;
  [[= serial_xml::optional]] std::string nickname = "unknown";
};

int main() {
  const std::string initial_xml = "<Person age='21'><name>Alex</name></Person>";
  auto person = serial_xml::from_xml<Person>(initial_xml);
  std::println("{}: age {}; nickname {}", person.name, person.age, person.nickname);

  person.nickname = "Ace";
  const std::string replacement_xml = "<Person age='22'><name>Sam</name></Person>";
  serial_xml::from_xml(person, replacement_xml);
  std::println("{}: age {}; nickname {}", person.name, person.age, person.nickname);

  auto replacement = serial_xml::from_xml<Person>(initial_xml);
  person = std::move(replacement);
  std::println("Replaced: {}; nickname {}", person.name, person.nickname);
}
