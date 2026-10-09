import std;
import serial_xml;

struct ProductCode {
  int value;
};

template <>
ProductCode serial_xml::from_string<ProductCode>(std::string_view text) {
  if (!text.starts_with("SKU-")) throw serial_xml::deserialization_error("Expected SKU- code");
  return {serial_xml::from_string<int>(text.substr(4))};
}

std::string format_code(const ProductCode& code) { return "SKU-" + std::to_string(code.value); }

struct[[= serial_xml::name{"product"}]] Product {
  [[= serial_xml::attribute]] int id;
  std::string title;
  [[= serial_xml::format{format_code}]] ProductCode code;
  [[= serial_xml::iter{"tag"}]] std::vector<std::string> tags;
  [[= serial_xml::optional]] std::string note = "No note supplied";
};

class Inventory {
 public:
  [[= serial_xml::skip]] int quantity() const { return quantity_; }
  [[ = serial_xml::setter, = serial_xml::name{"quantity"} ]] void load_quantity(int value) {
    quantity_ = value;
  }

 private:
  int quantity_ = 0;
};

int main() {
  Product original{7, "Reflection & XML", {42}, {"C++", "XML"}, "In stock"};
  auto xml = serial_xml::to_xml(original);
  auto restored = serial_xml::from_xml<Product>(xml);
  std::println("{}", xml);
  std::println("Restored {} with code {}", restored.title, restored.code.value);

  auto inventory =
      serial_xml::from_xml<Inventory>("<Inventory><quantity>12</quantity></Inventory>");
  std::println("Quantity set through encapsulation: {}", inventory.quantity());

  // In-place reconstruction preserves absent optional members' current values.
  serial_xml::from_xml(restored,
                       "<product id='8'><title>Next product</title><code>SKU-43</code>"
                       "<tags><tag>example</tag></tags></product>");
  std::println("{}: {}", restored.title, restored.note);
}
