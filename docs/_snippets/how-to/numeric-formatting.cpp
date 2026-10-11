import std;
import serial_xml;

struct FormattedAttribute {
  [[ = serial_xml::attribute, = serial_xml::format{"03d"} ]] int x;
};

int main() {
  const FormattedAttribute original{42};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", xml);
  const auto restored = serial_xml::from_xml<FormattedAttribute>(xml);
  std::println("Restored value: {}", restored.x);
}
