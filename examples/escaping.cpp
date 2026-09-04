import std;
import serial_xml;

// SerialXML automatically escapes XML special characters
// (< > & ' ") in all output contexts.

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
  std::string special = "<>&'\"";
  std::print("Escaped child:\n  {}\n", serial_xml::to_xml(EscapeChild{special}));
  std::print("Escaped attribute:\n  {}\n", serial_xml::to_xml(EscapeAttribute{special}));
  std::print("Escaped raw:\n  {}\n", serial_xml::to_xml(EscapeRaw{special}));
}
