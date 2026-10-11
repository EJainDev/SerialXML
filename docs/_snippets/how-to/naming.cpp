import std;
import serial_xml;

struct[[= serial_xml::name{"person"}]] Person {
  [[= serial_xml::attribute]] int age;
  [[= serial_xml::name{"food"}]] std::string favorite_food;
};

int main() {
  const Person original{3, "pizza"};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", xml);
  const auto restored = serial_xml::from_xml<Person>(xml);
  std::println("Age: {}; food: {}", restored.age, restored.favorite_food);
}
