import std;
import serial_xml;

// std::optional members are omitted entirely when they
// contain std::nullopt. When they hold a value, they
// behave like their inner type.

struct OptionalChild {
  std::optional<int> value;
};

struct OptionalAttribute {
  [[= serial_xml::attribute]] std::optional<int> value;
};

struct MixedOptional {
  std::string always_present;
  std::optional<std::string> maybe_present;
};

int main() {
  std::print("Optional with value:\n  {}\n", serial_xml::to_xml(OptionalChild{{42}}));
  std::print("Optional without value (nullopt):\n  {}\n",
             serial_xml::to_xml(OptionalChild{std::nullopt}));
  std::print("Optional attribute with value:\n  {}\n", serial_xml::to_xml(OptionalAttribute{{42}}));
  std::print("Optional attribute without value:\n  {}\n",
             serial_xml::to_xml(OptionalAttribute{std::nullopt}));
  std::print("Mixed optional:\n  {}\n", serial_xml::to_xml(MixedOptional{"hello", std::nullopt}));
  std::print("Mixed optional (both present):\n  {}\n",
             serial_xml::to_xml(MixedOptional{"hello", "world"}));
}
