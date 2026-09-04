import std;
import serial_xml;

// SerialXML serializes nested structs recursively.
// Inner struct members are unpacked by default unless
// they are formattable (e.g. std::string, std::optional).

struct Address {
  std::string street;
  std::string city;
  int zip;
};

struct Person {
  std::string name;
  Address address;
};

struct NestedAttributesInner {
  [[= serial_xml::attribute]] int x;
  [[= serial_xml::attribute]] int y;
};

struct NestedAttributesOuter {
  NestedAttributesInner inner;
};

int main() {
  std::print("Nested structs:\n  {}\n",
             serial_xml::to_xml(Person{"Alice", {"123 Main St", "Boston", 2128}}));
  std::print("Nested attributes:\n  {}\n", serial_xml::to_xml(NestedAttributesOuter{{42, 100}}));
}
