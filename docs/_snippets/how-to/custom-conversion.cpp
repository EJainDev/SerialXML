import std;
import serial_xml;

struct ProductCode {
  int value;
};

std::string format_code(const ProductCode& code) { return "SKU-" + std::to_string(code.value); }

template <>
ProductCode serial_xml::from_string<ProductCode>(std::string_view text) {
  if (!text.starts_with("SKU-")) {
    throw serial_xml::deserialization_error("Expected SKU- code");
  }
  return {serial_xml::from_string<int>(text.substr(4))};
}

struct Product {
  [[= serial_xml::attribute]] int id;
  [[= serial_xml::format{format_code}]] ProductCode code;
};

int main() {
  const Product original{7, {42}};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", xml);
  const auto restored = serial_xml::from_xml<Product>(xml);
  std::println("Product: {}; code: {}", restored.id, restored.code.value);
  try {
    const auto invalid = serial_xml::from_string<ProductCode>("OTHER-42");
    std::println("{}", invalid.value);
  } catch (const serial_xml::deserialization_error& error) {
    std::println("Conversion failed: {}", error.what());
  }
}
