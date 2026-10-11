import std;
import serial_xml;

struct Book {
  [[= serial_xml::attribute]] int id;
  std::string title;
};

int main() {
  const Book book{1, "The XML Handbook"};
  std::println("{}", serial_xml::to_xml(book, false));
}
