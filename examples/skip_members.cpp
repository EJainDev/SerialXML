import std;
import serial_xml;

// The [[= serial_xml::skip]] annotation hides a member
// from the XML output entirely.

struct SkipChild {
  int x;
  [[= serial_xml::skip]] int y;
};

struct SkipAttribute {
  [[ = serial_xml::attribute, = serial_xml::skip ]] int x;
};

struct SkipNamedChild {
  [[ = serial_xml::name{"MyChild"}, = serial_xml::skip ]] int x;
};

int main() {
  std::print("Skip child (y is omitted):\n  {}\n", serial_xml::to_xml(SkipChild{42, 100}));
  std::print("Skip attribute (x is omitted):\n  {}\n", serial_xml::to_xml(SkipAttribute{42}));
  std::print("Skip named child:\n  {}\n", serial_xml::to_xml(SkipNamedChild{42}));
}
