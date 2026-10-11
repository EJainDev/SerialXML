import std;
import serial_xml;

struct AttributeAndChild {
  [[= serial_xml::attribute]] int x;
  int y;
};

int main() {
  const AttributeAndChild original{4, 5};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", xml);
  const auto restored = serial_xml::from_xml<AttributeAndChild>(xml);
  std::println("x: {}; y: {}", restored.x, restored.y);
}
