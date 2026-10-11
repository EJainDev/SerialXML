import std;
import serial_xml;

struct OptionalChild {
  std::optional<int> value;
};

int main() {
  const auto present = serial_xml::to_xml(OptionalChild{{42}}, false);
  const auto absent = serial_xml::to_xml(OptionalChild{std::nullopt}, false);
  std::println("{}", present);
  std::println("{}", absent);
  auto restored = serial_xml::from_xml<OptionalChild>(present);
  std::println("Present value: {}", restored.value.value());
  serial_xml::from_xml(restored, absent);
  std::println("After absent input: {}", restored.value.has_value());
}
