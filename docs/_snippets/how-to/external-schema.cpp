import std;
import serial_xml;

class mock_vector {
 public:
  [[= serial_xml::attribute]] std::size_t size() const;
};

int main() {
  const std::vector<int> values{1, 2, 3};
  const auto xml = serial_xml::to_xml<mock_vector>(values, false);
  std::println("{}", xml);
  std::println("Actual vector size: {}", values.size());
}
