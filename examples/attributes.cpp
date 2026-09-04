import std;
import serial_xml;

// Attributes are rendered as XML attributes (e.g. x="4")
// instead of child tags (e.g. <x>4</x>).
struct SingleAttribute {
  [[= serial_xml::attribute]] int x;
};

struct MultipleAttributes {
  [[= serial_xml::attribute]] int x;
  [[= serial_xml::attribute]] int y;
};

struct AttributeAndChild {
  [[= serial_xml::attribute]] int x;
  int y;
};

int main() {
  std::print("Single attribute:\n  {}\n", serial_xml::to_xml(SingleAttribute{4}));
  std::print("Multiple attributes:\n  {}\n", serial_xml::to_xml(MultipleAttributes{4, 5}));
  std::print("Attribute + child:\n  {}\n", serial_xml::to_xml(AttributeAndChild{4, 5}));
}
