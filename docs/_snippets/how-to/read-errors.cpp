import std;
import serial_xml;

struct Record {
  int value;
};

int main() {
  const std::string xml = "<Record><value>not-a-number</value></Record>";
  try {
    const auto record = serial_xml::from_xml<Record>(xml);
    std::println("Value: {}", record.value);
  } catch (const serial_xml::deserialization_error& error) {
    std::println("Read failed");
    std::println("{}", error.what());
  }
}
