import std;
import serial_xml;

struct[[= serial_xml::name{"record"}]] Record {
  int value;
};

int main() {
  const Record record{42};
  const auto document = serial_xml::to_xml(record);
  const auto fragment = serial_xml::to_xml(record, false);
  const auto named = serial_xml::to_xml(record, false, "entry");
  std::println("Document:\n{}", document);
  std::println("Fragment:\n{}", fragment);
  std::println("Named root:\n{}", named);
  std::println("For display:\n{}", serial_xml::prettify(named));
  const auto restored = serial_xml::from_xml<Record>(named, "entry");
  std::println("Restored value: {}", restored.value);
}
