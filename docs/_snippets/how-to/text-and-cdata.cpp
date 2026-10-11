import std;
import serial_xml;

struct RawExample {
  [[= serial_xml::raw]] std::string text;
};

struct CDataExample {
  [[= serial_xml::cdata]] std::string content;
};

int main() {
  const auto raw_xml = serial_xml::to_xml(RawExample{"Hello <world> & friends"}, false);
  const auto cdata_xml = serial_xml::to_xml(CDataExample{"text<empty> & stuff"}, false);
  std::println("{}", raw_xml);
  std::println("{}", cdata_xml);
  const auto raw_value = serial_xml::from_xml<RawExample>(raw_xml);
  const auto cdata_value = serial_xml::from_xml<CDataExample>(cdata_xml);
  std::println("Raw value: {}", raw_value.text);
  std::println("CDATA value: {}", cdata_value.content);
}
