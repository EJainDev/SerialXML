import std;
import serial_xml;

// [[= serial_xml::raw]] emits text without wrapping tags.
// [[= serial_xml::cdata]] wraps content in <![CDATA[...]]> tags.

struct RawExample {
  [[= serial_xml::raw]] std::string text;
};

struct CDataExample {
  [[= serial_xml::cdata]] std::string content;
};

int main() {
  std::print("Raw text (no wrapper):\n  {}\n", serial_xml::to_xml(RawExample{"Hello, world!"}));
  std::print("CData:\n  {}\n", serial_xml::to_xml(CDataExample{"text<empty> & stuff"}));
}
