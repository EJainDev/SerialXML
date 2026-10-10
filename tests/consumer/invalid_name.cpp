import std;
import serial_xml;

#if INVALID_NAME_CASE == 1
struct Record {
  [[= serial_xml::name{"1bad"}]] int value;
};
#elif INVALID_NAME_CASE == 2
struct Record {
  [[ = serial_xml::attribute, = serial_xml::name{"bad name"} ]] int value;
};
#elif INVALID_NAME_CASE == 3
struct[[= serial_xml::name{"bad name"}]] Record {
  int value;
};
#elif INVALID_NAME_CASE == 4
struct Record {
  [[= serial_xml::iter{"1bad", "items"}]] std::vector<int> value;
};
#elif INVALID_NAME_CASE == 5
struct Record {
  [[= serial_xml::iter{"item", "bad name"}]] std::vector<int> value;
};
#else
struct Record {
  [[= serial_xml::name{""}]] int value;
};
#endif

int main() { auto xml = serial_xml::to_xml(Record{}); }
