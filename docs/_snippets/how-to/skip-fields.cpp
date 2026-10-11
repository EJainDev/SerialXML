import std;
import serial_xml;

struct Record {
  int value;
  [[= serial_xml::skip]] int cached_value = -1;
};

int main() {
  const Record original{42, 100};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", xml);
  auto restored = serial_xml::from_xml<Record>(xml);
  std::println("New object: value {}; cache {}", restored.value, restored.cached_value);
  restored.cached_value = 250;
  serial_xml::from_xml(restored, xml);
  std::println("Updated object: cache {}", restored.cached_value);
}
