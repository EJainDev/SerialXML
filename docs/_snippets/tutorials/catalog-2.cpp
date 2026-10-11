import std;
import serial_xml;

struct Book {
  [[= serial_xml::attribute]] int id;
  std::string title;
};

struct[[= serial_xml::name{"catalog"}]] Catalog {
  [[= serial_xml::iter{"book"}]] std::vector<Book> books;
};

int main() {
  const Catalog original{{{1, "The XML Handbook"}, {2, "Reflection in C++"}}};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", serial_xml::prettify(xml));
}
