import std;
import serial_xml;

class mock_vector {
 public:
  // Only the declaration and annotations matter; this method is never called.
  [[= serial_xml::attribute]] std::size_t size() const;
};

int main() {
  const std::vector<int> values{1, 2, 3};
  std::println("{}", serial_xml::to_xml<mock_vector>(values));
}
