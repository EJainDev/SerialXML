import std;
import serial_xml;

// STL containers like std::vector are iterated automatically,
// producing <element> tags for each item.

struct VectorExample {
  std::vector<int> values;
};

struct EmptyVector {
  std::vector<int> values;
};

// [[= serial_xml::exclude_on_empty]] omits the entire member
// when the container is empty.

struct ExcludeOnEmpty {
  [[= serial_xml::exclude_on_empty]] std::vector<int> values;
};

// [[= serial_xml::no_iter]] treats the container as a single
// value, calling std::format instead of iterating.

struct NoIter {
  [[= serial_xml::no_iter]] std::vector<int> values;
};

int main() {
  std::print("Vector (auto-iterated):\n  {}\n", serial_xml::to_xml(VectorExample{{1, 2, 3}}));
  std::print("Empty vector:\n  {}\n", serial_xml::to_xml(EmptyVector{{}}));
  std::print("Exclude on empty (empty vector):\n  {}\n", serial_xml::to_xml(ExcludeOnEmpty{{}}));
  std::print("No-iterate (formatted as a whole):\n  {}\n", serial_xml::to_xml(NoIter{{1, 2, 3}}));
}
