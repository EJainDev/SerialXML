import std;
import serial_xml;

// [[= serial_xml::format{"..."}]] passes a format specifier
// to std::format for that member's value.

struct FormattedAttribute {
  [[ = serial_xml::attribute, = serial_xml::format{"03d"} ]] int x;
};

struct FormattedChild {
  [[= serial_xml::format{"03d"}]] int x;
};

struct FormattedRaw {
  [[ = serial_xml::raw, = serial_xml::format{"03d"} ]] int x;
};

struct FormattedStringChild {
  [[= serial_xml::format{"*^12"}]] std::string text;
};

int main() {
  std::print("Formatted attribute:\n  {}\n", serial_xml::to_xml(FormattedAttribute{42}));
  std::print("Formatted child:\n  {}\n", serial_xml::to_xml(FormattedChild{42}));
  std::print("Formatted raw:\n  {}\n", serial_xml::to_xml(FormattedRaw{42}));
  std::print("Formatted string (padded):\n  {}\n",
             serial_xml::to_xml(FormattedStringChild{"text"}));
}
