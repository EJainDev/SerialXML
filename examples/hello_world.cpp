import std;
import serial_xml;

struct Person {
  int age;
  std::string favorite_food;
};

int main() {
  std::print("{}", serial_xml::to_xml(Person{3, "pizza"}));
  std::print("\n");
}
