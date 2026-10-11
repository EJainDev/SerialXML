import std;
import serial_xml;

struct Numbers {
  [[= serial_xml::iter{"number", "numbers"}]] std::vector<int> values;
};

int main() {
  const Numbers original{{1, 2, 3}};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", serial_xml::prettify(xml));
  const auto restored = serial_xml::from_xml<Numbers>(xml);
  for (const int value : restored.values) {
    std::println("Number: {}", value);
  }
}
