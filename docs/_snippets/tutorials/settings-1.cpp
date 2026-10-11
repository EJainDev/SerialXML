import std;
import serial_xml;

struct[[= serial_xml::name{"settings"}]] Preferences {
  int volume;
  std::string theme = "light";
};

int main() {
  const auto preferences = serial_xml::from_xml<Preferences>(
      "<settings><volume>20</volume><theme>dark</theme></settings>");
  std::println("Volume: {}; theme: {}", preferences.volume, preferences.theme);
}
