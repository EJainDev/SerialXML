import std;
import serial_xml;

struct[[= serial_xml::name{"settings"}]] Preferences {
  int volume;
  [[= serial_xml::optional]] std::string theme = "light";
};

int main() {
  auto preferences = serial_xml::from_xml<Preferences>("<settings><volume>20</volume></settings>");
  std::println("Initial: volume {}; theme {}", preferences.volume, preferences.theme);
  preferences.theme = "dark";
  serial_xml::from_xml(preferences, "<settings><volume>35</volume></settings>");
  std::println("Updated: volume {}; theme {}", preferences.volume, preferences.theme);
  try {
    const auto invalid =
        serial_xml::from_xml<Preferences>("<settings><volume>loud</volume></settings>");
    std::println("Volume: {}", invalid.volume);
  } catch (const serial_xml::deserialization_error& error) {
    std::println("Rejected input: {}", error.what());
  }
  std::println("{}", serial_xml::to_xml(preferences, false));
}
