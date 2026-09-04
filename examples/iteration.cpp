import std;
import serial_xml;

// [[= serial_xml::iter{"single", "multiple"}]] lets you customise
// the tag names when iterating over ranges.

struct IterSTL {
  [[= serial_xml::iter{"c_val", "container"}]] std::vector<int> values;
};

struct SingleCharIter {
  [[= serial_xml::iter{"v", "vals"}]] std::vector<int> values;
};

// You can iterate over a range of structs too.

struct InnerItem {
  int x;
};

struct StructInRangeOuter {
  [[= serial_xml::iter{"inner", "inners"}]] std::vector<InnerItem> inners;
};

int main() {
  std::print("Custom iter names:\n  {}\n", serial_xml::to_xml(IterSTL{{1, 2, 3}}));
  std::print("Single-char iter names:\n  {}\n", serial_xml::to_xml(SingleCharIter{{1, 2, 3}}));
  std::print("Structs in a range:\n  {}\n",
             serial_xml::to_xml(StructInRangeOuter{{{1}, {2}, {3}}}));
}
