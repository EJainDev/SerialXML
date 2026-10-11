import std;
import serial_xml;

struct VectorExample {
  std::vector<int> values;
};

struct ExcludeOnEmpty {
  [[= serial_xml::exclude_on_empty]] std::vector<int> values;
};

struct NoIter {
  [[= serial_xml::no_iter]] std::vector<int> values;
};

int main() {
  const auto iterated = serial_xml::to_xml(VectorExample{{1, 2, 3}}, false);
  const auto empty = serial_xml::to_xml(VectorExample{{}}, false);
  const auto omitted = serial_xml::to_xml(ExcludeOnEmpty{{}}, false);
  const auto text = serial_xml::to_xml(NoIter{{1, 2, 3}}, false);
  std::println("{}", iterated);
  std::println("{}", empty);
  std::println("{}", omitted);
  std::println("{}", text);
  const auto restored = serial_xml::from_xml<VectorExample>(iterated);
  const auto restored_text = serial_xml::from_xml<NoIter>(text);
  const auto restored_empty = serial_xml::from_xml<ExcludeOnEmpty>(omitted);
  std::println("Items: {}; text items: {}; omitted items: {}", restored.values.size(),
               restored_text.values.size(), restored_empty.values.size());
}
