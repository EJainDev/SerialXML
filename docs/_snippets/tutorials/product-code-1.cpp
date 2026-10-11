import std;
import serial_xml;

struct ProductCode {
  int value;
};

std::string format_code(const ProductCode& code) { return "SKU-" + std::to_string(code.value); }

struct Product {
  [[= serial_xml::attribute]] int id;
  [[= serial_xml::format{format_code}]] ProductCode code;
};

int main() {
  const Product original{7, {42}};
  std::println("{}", serial_xml::to_xml(original, false));
}
