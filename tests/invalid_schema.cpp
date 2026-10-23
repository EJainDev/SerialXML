import std;
import serial_xml;

struct Nested {
  int value;
};

#if INVALID_SCHEMA_CASE == 1
struct Record {
  [[ = serial_xml::attribute, = serial_xml::unpack ]] Nested value;
};
#elif INVALID_SCHEMA_CASE == 2
struct Record {
  [[ = serial_xml::attribute, = serial_xml::cdata ]] std::string value;
};
#elif INVALID_SCHEMA_CASE == 3
struct Record {
  [[ = serial_xml::attribute, = serial_xml::raw ]] int value;
};
#elif INVALID_SCHEMA_CASE == 4
struct Record {
  [[ = serial_xml::cdata, = serial_xml::raw ]] std::string value;
};
#elif INVALID_SCHEMA_CASE == 5
struct Record {
  [[ = serial_xml::cdata, = serial_xml::unpack ]] Nested value;
};
#elif INVALID_SCHEMA_CASE == 6
struct Record {
  [[ = serial_xml::cdata, = serial_xml::iter{"item"} ]] std::vector<int> value;
};
#elif INVALID_SCHEMA_CASE == 7
struct Record {
  [[= serial_xml::iter{"item"}]] int value;
};
#elif INVALID_SCHEMA_CASE == 8
struct Record {
  int actual;
};
struct Schema {
  int missing;
};
#elif INVALID_SCHEMA_CASE == 9
struct Record {
  int value() const { return 1; }
  int value(int = 0) const { return 2; }
};
struct Schema {
  int value() const;
};
#elif INVALID_SCHEMA_CASE == 10
struct Record {
  int value() const { return 1; }
};
struct Schema {
  int value;
};
#elif INVALID_SCHEMA_CASE == 11
struct Record {
  [[= serial_xml::setter]] void load() {}
};
#elif INVALID_SCHEMA_CASE == 12
struct Record {
  [[= serial_xml::setter]] static void load(int) {}
};
#elif INVALID_SCHEMA_CASE == 13
struct Record {
  [[= serial_xml::setter]] void load(int, int) {}
};
#elif INVALID_SCHEMA_CASE == 14
struct Record {
  const int value = 0;
};
#elif INVALID_SCHEMA_CASE == 15
struct Record {
  int value() const { return 0; }
  void value(int) {}
  void set_value(int) {}
};
#elif INVALID_SCHEMA_CASE == 16
struct Record {
  int value;
};
struct Schema {
  [[= serial_xml::setter]] void missing(int);
};
#elif INVALID_SCHEMA_CASE == 17
struct Record {
  [[ = serial_xml::setter, = serial_xml::attribute, = serial_xml::raw ]] void load(int) {}
};
#elif INVALID_SCHEMA_CASE == 18
struct Record {
  [[ = serial_xml::setter, = serial_xml::cdata, = serial_xml::raw ]] void load(std::string) {}
};
#elif INVALID_SCHEMA_CASE == 19
struct Record {
  [[= serial_xml::setter]] int value;
};
#elif INVALID_SCHEMA_CASE == 20
struct Record {
  [[= serial_xml::skip]] std::span<int> ignored;
  std::span<int> value;
};
#elif INVALID_SCHEMA_CASE == 21
struct Record {
  void load(int) {}
  void load(double) {}
};
struct Schema {
  [[= serial_xml::setter]] void load(int);
};
#elif INVALID_SCHEMA_CASE == 22
struct Record {
  [[= serial_xml::no_unpack]] std::unique_ptr<int> value;
};
#endif

int main() {
#if INVALID_SCHEMA_CASE >= 8 && INVALID_SCHEMA_CASE <= 10
  (void)serial_xml::to_xml<Schema>(Record{});
#elif INVALID_SCHEMA_CASE == 16 || INVALID_SCHEMA_CASE == 21
  (void)serial_xml::from_xml<Record, Schema>("<Record/>");
#elif INVALID_SCHEMA_CASE >= 11
  (void)serial_xml::from_xml<Record>("<Record/>");
#else
  (void)serial_xml::to_xml(Record{});
#endif
}
