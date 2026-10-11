import std;
import serial_xml;

struct Address {
  std::string street;
  std::string city;
  int zip;
};

struct Person {
  std::string name;
  Address address;
};

int main() {
  const Person original{"Alice", {"123 Main St", "Boston", 2128}};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", serial_xml::prettify(xml));
  const auto restored = serial_xml::from_xml<Person>(xml);
  std::println("{} lives in {} ({})", restored.name, restored.address.city, restored.address.zip);
}
