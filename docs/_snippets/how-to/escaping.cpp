import std;
import serial_xml;

struct EscapeChild {
  std::string text;
};

struct EscapeAttribute {
  [[= serial_xml::attribute]] std::string text;
};

struct EscapeRaw {
  [[= serial_xml::raw]] std::string text;
};

int main() {
  const std::string special = "<>&'\"";
  const auto child = serial_xml::to_xml(EscapeChild{special}, false);
  const auto attribute = serial_xml::to_xml(EscapeAttribute{special}, false);
  const auto raw = serial_xml::to_xml(EscapeRaw{special}, false);
  std::println("{}", child);
  std::println("{}", attribute);
  std::println("{}", raw);
  const auto restored = serial_xml::from_xml<EscapeChild>(child);
  std::println("Restored text: {}", restored.text);
}
