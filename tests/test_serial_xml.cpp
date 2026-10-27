import std;

import serial_xml;
import annotest;

using namespace annotest;

std::string clean_to_xml(const auto& obj) { return serial_xml::to_xml(obj, false); }

template <typename T>
struct CustomList {
  T data[16];
  std::size_t sz = 0;

  constexpr CustomList() = default;
  constexpr CustomList(std::initializer_list<T> init) {
    for (auto v : init) {
      if (sz < 16) data[sz++] = v;
    }
  }

  using iterator = const T*;
  using const_iterator = const T*;
  using value_type = T;
  using size_type = std::size_t;

  constexpr iterator begin() const { return data; }
  constexpr iterator end() const { return data + sz; }
  constexpr const_iterator cbegin() const { return data; }
  constexpr const_iterator cend() const { return data + sz; }

  constexpr size_type size() const { return sz; }
  constexpr bool empty() const { return sz == 0; }

  constexpr const T& operator[](std::size_t i) const { return data[i]; }
};

struct NoUnpackInner {
  int x;
};

template <>
struct std::formatter<NoUnpackInner> : std::formatter<std::string> {
  template <typename FormatContext>
  auto format(const NoUnpackInner& value, FormatContext& ctx) const {
    return std::formatter<std::string>::format(std::to_string(value.x), ctx);
  }
};

std::string format_with_prefix(const auto& value) { return "custom-" + std::to_string(value); }

std::string surround_string(const std::string& value) { return "[" + value + "]"; }

struct CustomFormatInner {
  int ignored;
};

std::string format_inner(const CustomFormatInner&) { return "formatted"; }

struct IgnoreTemplatedFunction {
  template <typename T>
  int x() const {
    return 3;
  }
};

class mock_vector {
 public:
  [[= serial_xml::attribute]] std::size_t size() const { return 0; }
};

struct DeserializationLeaf {
  int value;
  bool operator==(const DeserializationLeaf&) const = default;
};

template <>
DeserializationLeaf serial_xml::from_string<DeserializationLeaf>(std::string_view text) {
  return {serial_xml::from_string<int>(text)};
}

std::string format_deserialization_leaf(const DeserializationLeaf& leaf) {
  return std::to_string(leaf.value);
}

class SetterRecord {
 public:
  int value() const { return value_; }
  void value(int input) { value_ = input; }
  std::string get_title() const { return title_; }
  void set_title(std::string input) { title_ = std::move(input); }
  int getCount() const { return count_; }
  void setCount(int input) { count_ = input; }
  int read_only() const { return 77; }

 private:
  int value_ = 0;
  std::string title_;
  int count_ = 0;
};

class IndependentSetters {
 public:
  [[ = serial_xml::setter, = serial_xml::attribute,
     = serial_xml::name{"id"} ]] void load_id(int value) {
    id_ = value;
  }
  [[= serial_xml::setter]] void set_title(const std::string& value) { title_ = value; }
  [[ = serial_xml::setter, = serial_xml::optional ]] void setCount(int value) { count_ = value; }
  [[= serial_xml::skip]] int id() const { return id_; }
  [[= serial_xml::skip]] std::string title() const { return title_; }
  [[= serial_xml::skip]] int count() const { return count_; }

 private:
  int id_ = 0;
  std::string title_;
  int count_ = 5;
};

std::string invalid_xml_formatter(const std::string&) { return std::string(1, '\x01'); }

std::vector<named_case<std::string>> invalid_integer_cases() {
  return {case_of("empty", std::string("")),
          case_of("trailing_text", std::string("12tail")),
          case_of("overflow", std::string("9999999999999999999999")),
          case_of("double_sign", std::string("--1")),
          case_of("leading_plus", std::string("+1")),
          case_of("decimal", std::string("1.0")),
          case_of("hex", std::string("0x10")),
          case_of("internal_space", std::string("1 2")),
          case_of("split_sign", std::string("- 1"))};
}

std::vector<named_case<std::string>> malformed_xml_cases() {
  return {
      case_of("empty", std::string("")),
      case_of("unclosed_root", std::string("<MalformedInputRecord>")),
      case_of("wrong_root_close", std::string("<MalformedInputRecord><value>1</value></Wrong>")),
      case_of("long_member_close",
              std::string("<MalformedInputRecord><value>1</valueExtra></MalformedInputRecord>")),
      case_of("short_member_close",
              std::string("<MalformedInputRecord><value>1</val></MalformedInputRecord>")),
      case_of("truncated_member_close", std::string("<MalformedInputRecord><value>1</value")),
      case_of("trailing_text",
              std::string("<MalformedInputRecord><value>1</value></MalformedInputRecord>junk")),
      case_of("multiple_roots", std::string("<MalformedInputRecord/><MalformedInputRecord/>")),
      case_of(
          "duplicate_attribute",
          std::string("<MalformedInputRecord a='1' a='2'><value>1</value></MalformedInputRecord>")),
      case_of("unescaped_attribute",
              std::string("<MalformedInputRecord a='<'><value>1</value></MalformedInputRecord>")),
      case_of("unknown_entity",
              std::string("<MalformedInputRecord><value>&unknown;</value></MalformedInputRecord>")),
      case_of("surrogate_entity",
              std::string("<MalformedInputRecord><value>&#xD800;</value></MalformedInputRecord>")),
      case_of("null_entity",
              std::string("<MalformedInputRecord><value>&#0;</value></MalformedInputRecord>")),
      case_of("duplicate_member",
              std::string(
                  "<MalformedInputRecord><value>1</value><value>2</value></MalformedInputRecord>")),
      case_of("nested_scalar",
              std::string("<MalformedInputRecord><value><nested/></value></MalformedInputRecord>")),
      case_of("unclosed_cdata",
              std::string(
                  "<MalformedInputRecord><value><![CDATA[broken</value></MalformedInputRecord>")),
      case_of("doctype", std::string("<!DOCTYPE "
                                     "MalformedInputRecord><MalformedInputRecord><value>1</value></"
                                     "MalformedInputRecord>")),
      case_of(
          "invalid_comment",
          std::string(
              "<MalformedInputRecord><!--bad--comment--><value>1</value></MalformedInputRecord>")),
      case_of("cdata_terminator",
              std::string("<MalformedInputRecord><value>]]></value></MalformedInputRecord>")),
      case_of(
          "missing_attribute_space",
          std::string("<MalformedInputRecord a='1'b='2'><value>1</value></MalformedInputRecord>"))};
}

// Deliberately independent of the library's decoder; encode numeric Unicode
// scalars for exhaustive character-domain and XML Name boundary tests.
std::string utf8_scalar(unsigned value) {
  std::string result;
  if (value <= 0x7f)
    result += static_cast<char>(value);
  else if (value <= 0x7ff) {
    result += static_cast<char>(0xc0 | (value >> 6));
    result += static_cast<char>(0x80 | (value & 63));
  } else if (value <= 0xffff) {
    result += static_cast<char>(0xe0 | (value >> 12));
    result += static_cast<char>(0x80 | ((value >> 6) & 63));
    result += static_cast<char>(0x80 | (value & 63));
  } else {
    result += static_cast<char>(0xf0 | (value >> 18));
    result += static_cast<char>(0x80 | ((value >> 12) & 63));
    result += static_cast<char>(0x80 | ((value >> 6) & 63));
    result += static_cast<char>(0x80 | (value & 63));
  }
  return result;
}

std::vector<named_case<std::string, std::string>> xml_diagnostic_cases() {
  auto row = [](const char* name, const char* xml, const char* diagnostic) {
    return case_of(name, std::string(xml), std::string(diagnostic));
  };
  return {
      row("invalid_root_name", "<1r/>", "Invalid XML name"),
      row("attribute_missing_equals", "<r a/>", "Expected ="),
      row("attribute_unquoted", "<r a=1/>", "Expected quoted attribute"),
      row("attribute_unterminated", "<r a='", "Unterminated attribute"),
      row("attribute_adjacent", "<r a='1'b='2'/>", "Missing whitespace before attribute"),
      row("attribute_duplicate", "<r a='1' a='2'/>", "Duplicate attribute"),
      row("attribute_raw_markup", "<r a='<'>", "Unescaped '<' in attribute"),
      row("entity_unterminated", "<r><value>&amp</value></r>", "Unterminated entity reference"),
      row("entity_empty_decimal", "<r><value>&#;</value></r>", "Invalid character reference"),
      row("entity_empty_hex", "<r><value>&#x;</value></r>", "Invalid character reference"),
      row("entity_non_hex", "<r><value>&#xG;</value></r>", "Invalid character reference"),
      row("entity_integer_overflow", "<r><value>&#4294967296;</value></r>",
          "Invalid character reference"),
      row("entity_above_unicode", "<r><value>&#x110000;</value></r>",
          "Invalid XML character reference"),
      row("entity_noncharacter", "<r><value>&#xFFFF;</value></r>",
          "Invalid XML character reference"),
      row("entity_negative", "<r><value>&#-1;</value></r>", "Invalid character reference"),
      row("entity_unknown", "<r><value>&external;</value></r>", "Unknown entity reference"),
      row("text_unterminated", "<r><value>text", "Unterminated element text"),
      row("element_unterminated", "<r>", "Unterminated element"),
      row("closing_tag_wrong", "<r><value/></wrong>", "Mismatched closing tag"),
      row("closing_tag_extra_suffix", "<r><value/></root>", "Expected >"),
      row("nested_scalar", "<r><value><inner/></value></r>", "Unexpected nested element"),
      row("duplicate_member", "<r><value/><value/></r>", "Duplicate element"),
      row("cdata_unterminated", "<r><value><![CDATA[text</value></r>", "Unterminated CDATA"),
      row("cdata_split_unterminated", "<r><value><![CDATA[]]]]><![CDATA[>", "Unterminated CDATA"),
      row("cdata_cr_split_unterminated", "<r><value><![CDATA[a]]>&#13;<![CDATA[",
          "Unterminated CDATA"),
      row("cdata_terminator_in_text", "<r><value>]]></value></r>", "Unescaped CDATA terminator"),
      row("comment_unterminated", "<r><!--x", "Malformed XML comment"),
      row("comment_double_hyphen", "<r><!--x--y--></r>", "Malformed XML comment"),
      row("nested_dtd", "<r><!DOCTYPE r></r>", "DTD and markup declarations are unsupported"),
      row("nested_entity_declaration", "<r><!ENTITY x 'value'></r>",
          "DTD and markup declarations are unsupported"),
      row("pi_reserved_target", "<r><?XmL?></r>", "Reserved processing instruction target"),
      row("pi_unterminated", "<r><?task ", "Unterminated processing instruction"),
      row("pi_missing_separator", "<r><?task!?></r>", "Invalid processing instruction"),
      row("pi_invalid_utf8", "<r><?task \xC0\x80?></r>", "Invalid UTF-8"),
      row("comment_invalid_utf8", "<r><!--\xC0\x80--></r>", "Invalid UTF-8"),
      row("declaration_unquoted", "<?xml version=1.0?><r/>",
          "Expected quoted XML declaration value"),
      row("declaration_unterminated", "<?xml version='1.0", "Unterminated XML declaration"),
      row("declaration_missing_space", "<?xml version='1.0'encoding='UTF-8'?><r/>",
          "Missing whitespace in XML declaration"),
      row("declaration_wrong_order", "<?xml encoding='UTF-8' version='1.0'?><r/>",
          "XML declaration requires version 1.0"),
      row("declaration_extra_attribute", "<?xml version='1.0' extra='x'?><r/>",
          "Invalid XML declaration attributes"),
      row("declaration_invalid_standalone", "<?xml version='1.0' standalone='YES'?><r/>",
          "Invalid standalone declaration"),
      row("declaration_late", "<r><?xml version='1.0'?></r>",
          "Reserved processing instruction target"),
      row("trailing_text", "<r><value/></r>tail", "Trailing content after root element"),
      row("trailing_second_root", "<r><value/></r><r/>", "Trailing content after root element"),
  };
}

struct NamingTests {
  [[= test{}]] void empty_with_name() {
    struct[[= serial_xml::name{"MyStruct"}]] EmptyName {};
    EmptyName obj;

    assert_equal("<MyStruct/>", clean_to_xml(obj));
  }

  [[= test{}]] void named_attribute() {
    struct NamedAttribute {
      [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute ]] int x;
    };
    NamedAttribute obj{4};

    assert_equal("<NamedAttribute MyAttribute=\"4\"/>", clean_to_xml(obj));
  }

  [[= test{}]] void named_child() {
    struct NamedChild {
      [[= serial_xml::name{"MyChild"}]] int x;
    };
    NamedChild obj{4};

    assert_equal("<NamedChild><MyChild>4</MyChild></NamedChild>", clean_to_xml(obj));
  }

  [[= test{}]] void named_attribute_and_child() {
    struct NamedAttributeAndChild {
      [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute ]] int x;
      [[= serial_xml::name{"MyChild"}]] int y;
    };
    NamedAttributeAndChild obj{4, 5};

    assert_equal(
        "<NamedAttributeAndChild MyAttribute=\"4\"><MyChild>5</MyChild></"
        "NamedAttributeAndChild>",
        clean_to_xml(obj));
  }

  [[= test{}]] void all_named() {
    struct[[= serial_xml::name{"MyStruct"}]] AllNamed {
      [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute ]] int x;
      [[= serial_xml::name{"MyChild"}]] int y;
    };
    AllNamed obj{4, 5};

    assert_equal("<MyStruct MyAttribute=\"4\"><MyChild>5</MyChild></MyStruct>", clean_to_xml(obj));
  }

  [[= test{}]] void xml_name_range_endpoints() {
    struct NameInput {
      int value;
    };
    const std::array<std::pair<unsigned, unsigned>, 16> start_ranges{{{':', ':'},
                                                                      {'A', 'Z'},
                                                                      {'_', '_'},
                                                                      {'a', 'z'},
                                                                      {0xc0, 0xd6},
                                                                      {0xd8, 0xf6},
                                                                      {0xf8, 0x2ff},
                                                                      {0x370, 0x37d},
                                                                      {0x37f, 0x1fff},
                                                                      {0x200c, 0x200d},
                                                                      {0x2070, 0x218f},
                                                                      {0x2c00, 0x2fef},
                                                                      {0x3001, 0xd7ff},
                                                                      {0xf900, 0xfdcf},
                                                                      {0xfdf0, 0xfffd},
                                                                      {0x10000, 0xeffff}}};
    auto valid_start = [&](unsigned cp) {
      for (auto [low, high] : start_ranges)
        if (cp >= low && cp <= high) {
          return true;
        }
      return false;
    };
    for (auto [low, high] : start_ranges) {
      for (const unsigned cp : {low - 1, low, high, high + 1}) {
        const auto name = utf8_scalar(cp) + "node";
        scoped_trace trace(std::format("XML Name start U+{:X}", cp));
        if (valid_start(cp)) {
          const auto xml = serial_xml::to_xml(NameInput{7}, false, name);
          expect_equal(7, serial_xml::from_xml<NameInput>(xml, name).value);
        } else {
          expect_throws<std::invalid_argument>(
              [&] { (void)serial_xml::to_xml(NameInput{7}, false, name); });
          expect_throws<serial_xml::deserialization_error>(
              [&] { (void)serial_xml::from_xml<NameInput>("<" + name + "/>", "valid"); });
        }
      }
    }
    for (const unsigned cp : {0xb7u, 0x300u, 0x36fu, 0x203fu, 0x2040u, unsigned('-'), unsigned('.'),
                              unsigned('0'), unsigned('9')}) {
      const auto name = "n" + utf8_scalar(cp);
      const auto xml = serial_xml::to_xml(NameInput{7}, false, name);
      expect_equal(7, serial_xml::from_xml<NameInput>(xml, name).value);
    }
  }

  [[= test{}]] void fixed_name() {
    struct FixedName {};

    assert_equal("<FixedName/>", serial_xml::to_xml(FixedName{}, false, "FixedName"));
  }

  [[= test{}]] void skip_named_attribute() {
    struct SkipNamedAttribute {
      [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute, = serial_xml::skip ]] int x;
    };
    SkipNamedAttribute obj{4};

    assert_equal("<SkipNamedAttribute/>", clean_to_xml(obj));
  }

  [[= test{}]] void skip_named_child() {
    struct SkipNamedChild {
      [[ = serial_xml::name{"MyChild"}, = serial_xml::skip ]] int x;
    };
    SkipNamedChild obj{4};

    assert_equal("<SkipNamedChild/>", clean_to_xml(obj));
  }

  [[= test{}]] void named_struct_child() {
    struct NamedStructChildInner {
      [[= serial_xml::name{"MyChild"}]] int x;
    };

    struct NamedStructChildOuter {
      NamedStructChildInner inner;
    };
    NamedStructChildOuter obj{{4}};

    assert_equal(
        "<NamedStructChildOuter><inner><MyChild>4</MyChild></inner></"
        "NamedStructChildOuter>",
        clean_to_xml(obj));
  }

  [[= test{}]] void validates_runtime_root_overrides() {
    struct RuntimeRootRecord {
      int value;
    };
    for (const auto& invalid : {"1bad", "bad name", "-bad", ".bad", "bad<", "\xC0\xAF"}) {
      expect_throws<std::invalid_argument>(
          [&] { (void)(serial_xml::to_xml(RuntimeRootRecord{}, false, invalid)); });
      expect_throws<std::invalid_argument>([&] {
        (void)(serial_xml::from_xml<RuntimeRootRecord>(
            "<RuntimeRootRecord><value>0</value></RuntimeRootRecord>", invalid));
      });
    }
    for (const auto& valid : {"_record", "ns:record", "é", "xml_record", "record-1.0"}) {
      const auto xml = serial_xml::to_xml(RuntimeRootRecord{7}, false, valid);
      expect_equal(7, serial_xml::from_xml<RuntimeRootRecord>(xml, valid).value);
    }
  }

  [[= test{}]] void supports_unicode_and_prefixed_annotations() {
    struct[[= serial_xml::name{"ns:记录"}]] UnicodeNameRecord {
      [[ = serial_xml::attribute, = serial_xml::name{"_é"} ]] int value;
      [[= serial_xml::iter{"子", "ns:items"}]] std::vector<int> items;
    };
    const auto xml = clean_to_xml(UnicodeNameRecord{3, {4}});
    expect_equal("<ns:记录 _é=\"3\"><ns:items><子>4</子></ns:items></ns:记录>", xml);
    expect_equal(std::vector<int>{4}, serial_xml::from_xml<UnicodeNameRecord>(xml).items);
  }
};

struct AttributesTests {
  [[= test{}]] void single() {
    struct SingleAttribute {
      [[= serial_xml::attribute]] int x;
    };
    SingleAttribute obj{4};

    assert_equal("<SingleAttribute x=\"4\"/>", clean_to_xml(obj));
  }

  [[= test{}]] void multiple() {
    struct MultipleAttributes {
      [[= serial_xml::attribute]] int x;
      [[= serial_xml::attribute]] int y;
    };
    MultipleAttributes obj{4, 5};

    assert_equal("<MultipleAttributes x=\"4\" y=\"5\"/>", clean_to_xml(obj));
  }

  [[= test{}]] void skip() {
    struct SkipAttribute {
      [[ = serial_xml::attribute, = serial_xml::skip ]] int x;
    };
    SkipAttribute obj{4};

    assert_equal("<SkipAttribute/>", clean_to_xml(obj));
  }

  [[= test{}]] void attribute_and_child() {
    struct AttributeAndChild {
      [[= serial_xml::attribute]] int x;
      int y;
    };
    AttributeAndChild obj{4, 5};

    assert_equal("<AttributeAndChild x=\"4\"><y>5</y></AttributeAndChild>", clean_to_xml(obj));
  }

  [[= test{}]] void attribute_and_skip_child() {
    struct AttributeAndSkipChild {
      [[= serial_xml::attribute]] int x;
      [[= serial_xml::skip]] int y;
    };
    AttributeAndSkipChild obj{4, 5};

    assert_equal("<AttributeAndSkipChild x=\"4\"/>", clean_to_xml(obj));
  }

  [[= test{}]] void nested_attribute() {
    struct NestedAttributeInner {
      [[= serial_xml::attribute]] int x;
    };

    struct NestedAttributeOuter {
      NestedAttributeInner inner;
    };
    NestedAttributeOuter obj{{4}};

    assert_equal("<NestedAttributeOuter><inner x=\"4\"/></NestedAttributeOuter>",
                 clean_to_xml(obj));
  }
};

struct STLTests {
  [[= test{}]] void vector() {
    struct Vector {
      std::vector<int> values;
    };
    Vector obj{{1, 2, 3}};
    assert_equal(
        "<Vector><values><element>1</element><element>2</"
        "element><element>3</element>"
        "</values></Vector>",
        clean_to_xml(obj));
  }

  [[= test{}]] void empty_vector() {
    struct EmptyVector {
      std::vector<int> values;
    };
    EmptyVector obj{{}};
    assert_equal("<EmptyVector><values/></EmptyVector>", clean_to_xml(obj));
  }

  [[= test{}]] void exclude_on_empty() {
    struct ExcludeOnEmpty {
      [[= serial_xml::exclude_on_empty]] std::vector<int> values;
    };
    ExcludeOnEmpty obj{{}};
    assert_equal("<ExcludeOnEmpty/>", clean_to_xml(obj));
  }

  [[= test{}]] void optional() {
    struct Optional {
      std::optional<int> value;
    };
    Optional obj{{42}};
    assert_equal("<Optional><value>42</value></Optional>", clean_to_xml(obj));

    Optional obj2{std::nullopt};
    assert_equal("<Optional/>", clean_to_xml(obj2));
  }

  [[= test{}]] void no_iter() {
    struct NoIter {
      [[= serial_xml::no_iter]] std::vector<int> values;
    };
    NoIter obj{{1, 2, 3}};
    assert_equal("<NoIter><values>[1, 2, 3]</values></NoIter>", clean_to_xml(obj));
  }

  [[= test{}]] void nested_vector() {
    struct NestedVector {
      std::vector<std::vector<int>> values;
    };
    NestedVector obj{{{1, 2}, {3, 4}}};
    assert_equal(
        "<NestedVector><values><element>[1, 2]</element><element>[3, "
        "4]</element></values></NestedVector>",
        clean_to_xml(obj));
  }

  [[= test{}]] void raw_iter() {
    struct STLRawIter {
      [[= serial_xml::raw]] std::vector<int> values;
    };

    STLRawIter obj{{1, 2, 3}};
    assert_equal(
        "<STLRawIter><element>1</element><element>2</element><element>3</element></STLRawIter>",
        clean_to_xml(obj));
  }

  [[= test{}]] void optional_attribute() {
    struct OptionalAttribute {
      [[= serial_xml::attribute]] std::optional<int> value;
    };
    OptionalAttribute obj{{42}};
    assert_equal("<OptionalAttribute value=\"42\"/>", clean_to_xml(obj));

    OptionalAttribute obj2{std::nullopt};
    assert_equal("<OptionalAttribute/>", clean_to_xml(obj2));
  }
};

struct ChildrenTests {
  [[= test{}]] void single() {
    struct SingleChild {
      int x;
    };
    SingleChild obj{42};
    assert_equal("<SingleChild><x>42</x></SingleChild>", clean_to_xml(obj));
  }

  [[= test{}]] void multiple() {
    struct MultipleChildren {
      int x;
      int y;
    };
    MultipleChildren obj{42, 100};
    assert_equal("<MultipleChildren><x>42</x><y>100</y></MultipleChildren>", clean_to_xml(obj));
  }

  [[= test{}]] void skip() {
    struct SkipChild {
      [[= serial_xml::skip]] int x;
    };
    SkipChild obj{42};
    assert_equal("<SkipChild/>", clean_to_xml(obj));
  }

  [[= test{}]] void child_and_skip() {
    struct ChildAndSkip {
      int x;
      [[= serial_xml::skip]] int y;
    };
    ChildAndSkip obj{42, 100};
    assert_equal("<ChildAndSkip><x>42</x></ChildAndSkip>", clean_to_xml(obj));
  }

  [[= test{}]] void nested() {
    struct NestedChildInner {
      int x;
    };
    struct NestedChildOuter {
      NestedChildInner inner;
    };
    NestedChildOuter obj{{42}};
    assert_equal("<NestedChildOuter><inner><x>42</x></inner></NestedChildOuter>",
                 clean_to_xml(obj));
  }
};

struct NestingTests {
  [[= test{}]] void children() {
    struct NestedChildrenInner {
      int x;
      int y;
    };
    struct NestedChildrenOuter {
      NestedChildrenInner inner;
    };
    NestedChildrenOuter obj{{42, 100}};
    assert_equal("<NestedChildrenOuter><inner><x>42</x><y>100</y></inner></NestedChildrenOuter>",
                 clean_to_xml(obj));
  }

  [[= test{}]] void attributes() {
    struct NestedAttributesInner {
      [[= serial_xml::attribute]] int x;
      [[= serial_xml::attribute]] int y;
    };
    struct NestedAttributesOuter {
      NestedAttributesInner inner;
    };
    NestedAttributesOuter obj{{42, 100}};
    assert_equal("<NestedAttributesOuter><inner x=\"42\" y=\"100\"/></NestedAttributesOuter>",
                 clean_to_xml(obj));
  }

  [[= test{}]] void attributes_and_children() {
    struct NestedAttributesAndChildrenInner {
      [[= serial_xml::attribute]] int x;
      int y;
    };
    struct NestedAttributesAndChildrenOuter {
      NestedAttributesAndChildrenInner inner;
    };
    NestedAttributesAndChildrenOuter obj{{42, 100}};
    assert_equal(
        "<NestedAttributesAndChildrenOuter><inner "
        "x=\"42\"><y>100</y></inner></NestedAttributesAndChildrenOuter>",
        clean_to_xml(obj));
  }

  [[= test{}]] void skip_inner() {
    struct SkipInner {
      [[= serial_xml::skip]] int x;
    };
    struct SkipOuter {
      SkipInner inner;
    };
    SkipOuter obj{{42}};
    assert_equal("<SkipOuter><inner/></SkipOuter>", clean_to_xml(obj));
  }

  [[= test{}]] void skip_nesting() {
    struct SkipNestingInner {
      int x;
    };
    struct SkipNestingOuter {
      [[= serial_xml::skip]] SkipNestingInner inner;
    };
    SkipNestingOuter obj{{42}};
    assert_equal("<SkipNestingOuter/>", clean_to_xml(obj));
  }
};

struct IterationTests {
  [[= test{}]] void custom_list() {
    struct Iter {
      [[= serial_xml::iter{"value", "values"}]] CustomList<int> values;
    };

    Iter obj{{1, 2, 3}};
    assert_equal(
        "<Iter><values><value>1</value><value>2</value><value>3</"
        "value></values></Iter>",
        clean_to_xml(obj));
  }

  [[= test{}]] void iter_stl() {
    struct IterSTL {
      [[= serial_xml::iter{"c_val", "container"}]] std::vector<int> values;
    };

    IterSTL obj{{1, 2, 3}};
    assert_equal(
        "<IterSTL><container><c_val>1</c_val><c_val>2</c_val><c_val>3</"
        "c_val></container></IterSTL>",
        clean_to_xml(obj));
  }

  [[= test{}]] void single_char_iter() {
    struct SingleCharIter {
      [[= serial_xml::iter{"v", "vals"}]] std::vector<int> values;
    };

    SingleCharIter obj{{1, 2, 3}};
    assert_equal("<SingleCharIter><vals><v>1</v><v>2</v><v>3</v></vals></SingleCharIter>",
                 clean_to_xml(obj));
  }

  [[= test{}]] void struct_in_range() {
    struct StructInRangeInner {
      int x;
    };
    struct StructInRangeOuter {
      [[= serial_xml::iter{"inner", "inners"}]] std::vector<StructInRangeInner> inners;
    };

    StructInRangeOuter obj{{{1}, {2}, {3}}};
    assert_equal(
        "<StructInRangeOuter><inners><inner><x>1</x></inner><inner><x>2</x></"
        "inner><inner><x>3</x></inner></"
        "inners></StructInRangeOuter>",
        clean_to_xml(obj));
  }

  [[= test{}]] void raw_iter() {
    struct RawIter {
      [[ = serial_xml::raw, = serial_xml::iter{"value", "values"} ]] std::vector<int> values;
    };

    RawIter obj{{1, 2, 3}};
    assert_equal("<RawIter><value>1</value><value>2</value><value>3</value></RawIter>",
                 clean_to_xml(obj));
  }
};

struct BasicTests {
  [[= test{}]] void empty_struct() {
    struct EmptyStruct {};
    EmptyStruct obj;

    std::string xml = serial_xml::to_xml(obj);
    assert_equal("<?xml version=\"1.0\" encoding=\"UTF-8\"?><EmptyStruct/>", xml);
  }

  [[= test{}]] void attributes_and_children() {
    struct AttributesAndChildren {
      [[= serial_xml::attribute]] int x;
      int y;
    };
    AttributesAndChildren obj{42, 100};
    assert_equal("<AttributesAndChildren x=\"42\"><y>100</y></AttributesAndChildren>",
                 clean_to_xml(obj));
  }

  [[= test{}]] void raw() {
    struct Raw {
      [[= serial_xml::raw]] std::string text;
    };

    Raw obj{"text"};
    assert_equal("<Raw>text</Raw>", clean_to_xml(obj));
  }

  [[= test{}]] void cdata() {
    struct CData {
      [[= serial_xml::cdata]] std::string text;
    };

    CData obj{"text<empty> & stuff"};
    assert_equal("<CData><![CDATA[text<empty> & stuff]]></CData>", clean_to_xml(obj));
  }
};

struct UnpackingTests {
  [[= test{}]] void no_unpack() {
    struct NoUnpackOuter {
      [[= serial_xml::no_unpack]] NoUnpackInner inner;
    };
    NoUnpackOuter obj{{42}};
    assert_equal("<NoUnpackOuter><inner>42</inner></NoUnpackOuter>", clean_to_xml(obj));
  }
};

struct EscapingTests {
  [[= test{}]] void child() {
    struct EscapeChild {
      std::string text;
    };

    EscapeChild obj{"<>&'\""};
    assert_equal("<EscapeChild><text>&lt;&gt;&amp;&apos;&quot;</text></EscapeChild>",
                 clean_to_xml(obj));
  }

  [[= test{}]] void complex_child() {
    struct EscapeComplexChild {
      std::string text;
      int number;
    };

    EscapeComplexChild obj{"Hi! <> My name is & Bob. ' And \" This", 42};
    assert_equal(
        "<EscapeComplexChild><text>Hi! &lt;&gt; My name "
        "is &amp; Bob. &apos; And &quot; This</"
        "text><number>42</number></EscapeComplexChild>",
        clean_to_xml(obj));
  }

  [[= test{}]] void attribute() {
    struct EscapeAttribute {
      [[= serial_xml::attribute]] std::string text;
    };

    EscapeAttribute obj{"<>&'\""};
    assert_equal("<EscapeAttribute text=\"&lt;&gt;&amp;&apos;&quot;\"/>", clean_to_xml(obj));
  }

  [[= test{}]] void raw() {
    struct EscapeRaw {
      [[= serial_xml::raw]] std::string text;
    };

    EscapeRaw obj{"<>&'\""};
    assert_equal("<EscapeRaw>&lt;&gt;&amp;&apos;&quot;</EscapeRaw>", clean_to_xml(obj));
  }

  [[= test{}]] void escapes_adjacent_quotes_in_report_properties() {
    struct QuotedProperty {
      [[= serial_xml::attribute]] std::string value;
    };
    for (std::size_t count = 0; count <= 80; ++count) {
      scoped_trace trace(std::format("adjacent quote count: {}", count));
      const std::string value = "(" + std::string(count, '"') + ")";
      std::string escaped = "(";
      for (std::size_t i = 0; i < count; ++i) escaped += "&quot;";
      escaped += ")";
      const auto xml = clean_to_xml(QuotedProperty{value});
      expect_equal("<QuotedProperty value=\"" + escaped + "\"/>", xml);
    }
  }

  [[= test{}]] void preserves_xml_whitespace() {
    struct WhitespaceRecord {
      [[= serial_xml::attribute]] std::string attribute;
      std::string text;
      [[= serial_xml::raw]] std::string raw;
    };
    const WhitespaceRecord value{"a\t\n\r\r\nb", "a\r\nb\rc", "x\ry"};
    const auto xml = clean_to_xml(value);
    expect_equal(
        "<WhitespaceRecord attribute=\"a&#9;&#10;&#13;&#13;&#10;b\">"
        "<text>a&#13;\nb&#13;c</text>x&#13;y</WhitespaceRecord>",
        xml);
    auto parsed = serial_xml::from_xml<WhitespaceRecord>(xml);
    expect_equal(value.attribute, parsed.attribute);
    expect_equal(value.text, parsed.text);
    expect_equal(value.raw, parsed.raw);
  }

  [[= test{}]] void cdata_splits_terminators_and_carriage_returns() {
    struct CdataRecord {
      [[= serial_xml::cdata]] std::string first;
      [[= serial_xml::cdata]] std::string second;
    };
    const CdataRecord value{"a]]>b\r\nc]]>\r", "second"};
    const auto xml = clean_to_xml(value);
    expect_equal(
        "<CdataRecord><![CDATA[a]]]]><![CDATA[>b]]>&#13;<![CDATA[\nc]]]]>"
        "<![CDATA[>]]>&#13;<![CDATA[]]><![CDATA[second]]></CdataRecord>",
        xml);
    auto parsed = serial_xml::from_xml<CdataRecord>(xml);
    expect_equal(value.first, parsed.first);
    expect_equal(value.second, parsed.second);
  }

  [[= test{}]] void rejects_invalid_characters_and_utf8() {
    struct Text {
      std::string value;
    };
    struct Attribute {
      [[= serial_xml::attribute]] std::string value;
    };
    struct Raw {
      [[= serial_xml::raw]] std::string value;
    };
    struct Cdata {
      [[= serial_xml::cdata]] std::string value;
    };
    for (const auto& invalid :
         {std::string("a\0b", 3), std::string("\x01"), std::string("\xC0\xAF"),
          std::string("\xED\xA0\x80"), std::string("\xF4\x90\x80\x80"), std::string("\xEF\xBF\xBE"),
          std::string("\xE2\x82")}) {
      expect_throws<std::invalid_argument>([&] { (void)(clean_to_xml(Text{invalid})); });
      expect_throws<std::invalid_argument>([&] { (void)(clean_to_xml(Attribute{invalid})); });
      expect_throws<std::invalid_argument>([&] { (void)(clean_to_xml(Raw{invalid})); });
      expect_throws<std::invalid_argument>([&] { (void)(clean_to_xml(Cdata{invalid})); });
    }
    const std::string unicode = "é😀\t\n";
    expect_equal(unicode, serial_xml::from_xml<Text>(clean_to_xml(Text{unicode})).value);
  }

  [[= test{}]] void whitespace_across_simd_and_mask_boundaries() {
    struct BoundaryWhitespace {
      [[= serial_xml::attribute]] std::string value;
    };
    for (std::size_t length : {15, 16, 31, 32, 63, 64, 65, 127, 128}) {
      std::string value(length, 'a');
      value += "\t\n\r";
      const auto xml = clean_to_xml(BoundaryWhitespace{value});
      expect_equal("<BoundaryWhitespace value=\"" + std::string(length, 'a') + "&#9;&#10;&#13;\"/>",
                   xml);
      expect_equal(value, serial_xml::from_xml<BoundaryWhitespace>(xml).value);
    }
  }

  [[= test{}]] void validates_custom_formatter_output() {
    struct InvalidFormattedText {
      [[= serial_xml::format{invalid_xml_formatter}]] std::string value;
    };
    expect_throws<std::invalid_argument>(
        [&] { (void)(clean_to_xml(InvalidFormattedText{"valid"})); });
  }
};

struct FormattingTests {
  [[= test{}]] void attribute() {
    struct FormattedAttribute {
      [[ = serial_xml::attribute, = serial_xml::format{"03d"} ]] int x;
    };
    FormattedAttribute obj{42};

    assert_equal("<FormattedAttribute x=\"042\"/>", clean_to_xml(obj));
  }

  [[= test{}]] void child() {
    struct FormattedChild {
      [[= serial_xml::format{"03d"}]] int x;
    };
    FormattedChild obj{42};

    assert_equal("<FormattedChild><x>042</x></FormattedChild>", clean_to_xml(obj));
  }

  [[= test{}]] void raw() {
    struct FormattedRaw {
      [[ = serial_xml::raw, = serial_xml::format{"03d"} ]] int x;
    };
    FormattedRaw obj{42};

    assert_equal("<FormattedRaw>042</FormattedRaw>", clean_to_xml(obj));
  }

  [[= test{}]] void string_child() {
    struct FormattedStringChild {
      [[= serial_xml::format{"*^12"}]] std::string text;
    };
    FormattedStringChild obj{"text"};

    assert_equal("<FormattedStringChild><text>****text****</text></FormattedStringChild>",
                 clean_to_xml(obj));
  }

  [[= test{}]] void custom_function() {
    struct CustomFormattedValues {
      [[ = serial_xml::attribute, = serial_xml::format{format_with_prefix<int>} ]] int attribute;
      [[= serial_xml::format{format_with_prefix<int>}]] int child;
      [[ = serial_xml::raw, = serial_xml::format{format_with_prefix<int>} ]] int raw;
    };
    CustomFormattedValues obj{1, 2, 3};

    assert_equal(
        "<CustomFormattedValues attribute=\"custom-1\"><child>custom-2</child>custom-3"
        "</CustomFormattedValues>",
        clean_to_xml(obj));
  }

  [[= test{}]] void custom_function_bypasses_string_fast_path() {
    struct CustomFormattedString {
      [[= serial_xml::format{surround_string}]] std::string text;
    };
    CustomFormattedString obj{"text"};

    assert_equal("<CustomFormattedString><text>[text]</text></CustomFormattedString>",
                 clean_to_xml(obj));
  }

  [[= test{}]] void custom_function_prevents_unpacking() {
    struct CustomFormattedOuter {
      [[= serial_xml::format{format_inner}]] CustomFormatInner inner;
    };
    CustomFormattedOuter obj{{42}};

    assert_equal("<CustomFormattedOuter><inner>formatted</inner></CustomFormattedOuter>",
                 clean_to_xml(obj));
  }
};

struct FunctionsTests {
  [[= test{}]] void basic() {
    struct BasicFunction {
      int x() const { return 3; }
    };
    BasicFunction obj;

    assert_equal("<BasicFunction><x>3</x></BasicFunction>", clean_to_xml(obj));
  }

  [[= test{}]] void requires_const() {
    struct ConstFunction {
      int x() { return 3; }
    };
    ConstFunction obj;

    assert_equal("<ConstFunction/>", clean_to_xml(obj));
  }

  [[= test{}]] void no_static_func() {
    struct NoStaticFunction {
      static int x() { return 3; }
    };
    NoStaticFunction obj;

    assert_equal("<NoStaticFunction/>", clean_to_xml(obj));
  }

  [[= test{}]] void ignore_parameterized() {
    struct IgnoreParameterizedFunction {
      int x(int a) const { return a; }
    };
    IgnoreParameterizedFunction obj;

    assert_equal("<IgnoreParameterizedFunction/>", clean_to_xml(obj));
  }

  [[= test{}]] void ignore_templated() {
    IgnoreTemplatedFunction obj;

    assert_equal("<IgnoreTemplatedFunction/>", clean_to_xml(obj));
  }
};

struct MockingTests {
  [[= test{}]] void vector_getter_uses_actual_object() {
    const std::vector<int> values{10, 20, 30};
    assert_equal("<vector size=\"3\"/>", serial_xml::to_xml<mock_vector>(values, false));
    assert_equal("<vector size=\"0\"/>",
                 serial_xml::to_xml<mock_vector>(std::vector<std::string>{}, false));
    assert_equal("<?xml version=\"1.0\" encoding=\"UTF-8\"?><vector size=\"3\"/>",
                 serial_xml::to_xml<mock_vector>(values));
  }

  [[= test{}]] void fields_use_mock_annotations_and_order() {
    struct MockFieldsTarget {
      int a;
      int i;
      int ignored;
    };
    struct MockFields {
      [[ = serial_xml::attribute, = serial_xml::name{"second"} ]] int i;
      [[= serial_xml::name{"first"}]] int a;
      [[= serial_xml::skip]] int nonexistent;
    };
    // a and i collide in the eight-slot identifier table.
    assert_equal(
        "<MockFieldsTarget "
        "second=\"7\"><first>4</first></MockFieldsTarget>",
        serial_xml::to_xml<MockFields>(MockFieldsTarget{4, 7, 99}, false));
  }

  [[= test{}]] void getter_selects_const_callable_overload() {
    struct MockOverloadsTarget {
      int amount() { return 100; }
      int amount() const { return 8; }
      int amount(int) const { return 200; }
    };
    struct MockOverloads {
      [[= serial_xml::attribute]] int amount() const;
    };
    assert_equal("<MockOverloadsTarget amount=\"8\"/>",
                 serial_xml::to_xml<MockOverloads>(MockOverloadsTarget{}, false));
  }

  [[= test{}]] void target_return_type_controls_serialization() {
    struct MockRangeTarget {
      std::vector<int> values() const { return {2, 3}; }
    };
    struct MockRange {
      // The placeholder body and type never provide the serialized value.
      int values() const;
    };
    assert_equal(
        "<MockRangeTarget><values><element>2</element><element>3</"
        "element></values></MockRangeTarget>",
        serial_xml::to_xml<MockRange>(MockRangeTarget{}, false));
  }

  [[= test{}]] void formats_and_names_come_from_mock() {
    struct MockFormatTarget {
      [[= serial_xml::skip]] int value;
    };
    struct[[= serial_xml::name{"record"}]] MockFormat {
      MockFormat() = delete;
      [[ = serial_xml::format{"04"}, = serial_xml::name{"number"} ]] int value;
    };
    assert_equal("<record><number>0012</number></record>",
                 serial_xml::to_xml<MockFormat>(MockFormatTarget{12}, false));
    assert_equal("<override><number>0012</number></override>",
                 serial_xml::to_xml<MockFormat>(MockFormatTarget{12}, false, "override"));
  }

  [[= test{}]] void nested_values_and_explicit_iteration() {
    struct Item {
      int value;
    };
    struct MockNestedTarget {
      Item child;
      std::vector<Item> items;
    };
    struct MockNested {
      int child;
      [[= serial_xml::iter{"item", "list"}]] int items;
    };
    assert_equal(
        "<MockNestedTarget><child><value>5</value></"
        "child><list><item><value>6</value></item>"
        "<item><value>7</value></item></list></MockNestedTarget>",
        serial_xml::to_xml<MockNested>(MockNestedTarget{{5}, {{6}, {7}}}, false));
  }

  [[= test{}]] void empty_schema_does_not_serialize_target_members() {
    struct MockEmptyTarget {
      int value;
    };
    struct Empty {};
    assert_equal("<MockEmptyTarget/>", serial_xml::to_xml<Empty>(MockEmptyTarget{5}, false));
    assert_equal(
        "<MockEmptyTarget><value>5</value></"
        "MockEmptyTarget>",
        serial_xml::to_xml<MockEmptyTarget>(MockEmptyTarget{5}, false));
  }

  [[= test{}]] void getter_with_default_argument() {
    struct MockDefaultTarget {
      int value(int offset = 2) const { return 5 + offset; }
    };
    struct MockDefault {
      int value() const;
    };
    assert_equal("<MockDefaultTarget><value>7</value></MockDefaultTarget>",
                 serial_xml::to_xml<MockDefault>(MockDefaultTarget{}, false));
  }
};

template <typename T>
struct[[= serial_xml::name{"WidthInput"}]] IntegerWidthInput {
  [[= serial_xml::attribute]] T attribute;
  T text;
};

struct FromStringTests {
  [[= test{}]] void primitives_and_strict_validation() {
    expect_equal(-42, serial_xml::from_string<int>(" -42 "));
    expect_equal(42u, serial_xml::from_string<unsigned>("42"));
    expect_equal(125.0, serial_xml::from_string<double>("1.25e2"));
    expect_true(serial_xml::from_string<bool>("true"));
    expect_true(serial_xml::from_string<bool>("1"));
    expect_false(serial_xml::from_string<bool>("false"));
    expect_false(serial_xml::from_string<bool>("0"));
    expect_equal('A', serial_xml::from_string<char>("65"));
    expect_equal("  hello  ", serial_xml::from_string<std::string>("  hello  "));
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_string<unsigned>("-1")); });
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_string<bool>("yes")); });
  }

  [[
    = test{}, = parameter_source<&invalid_integer_cases>{}
  ]] void rejects_invalid_integers(const std::string& input) {
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)serial_xml::from_string<int>(input); });
  }

  [[
    = test{},
    = parameterize{parameters(std::numeric_limits<int>::min()), parameters(-1), parameters(0),
                   parameters(1), parameters(std::numeric_limits<int>::max())}
  ]] void signed_integer_boundaries(int expected) {
    const auto text = std::to_string(expected);
    expect_equal(expected, serial_xml::from_string<int>(text));
    expect_equal(expected, serial_xml::from_string<int>(" \t" + text + "\r\n"));
  }

  [[= test{}]] void rejects_integer_overflow_and_embedded_nulls() {
    const auto above_max =
        std::to_string(static_cast<long long>(std::numeric_limits<int>::max()) + 1);
    const auto below_min =
        std::to_string(static_cast<long long>(std::numeric_limits<int>::min()) - 1);
    for (const auto& input : {above_max, below_min, std::string("42\0tail", 7)}) {
      scoped_trace trace(std::format("input: {:?}", input));
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<int>(input); });
    }
    expect_equal(
        std::numeric_limits<unsigned>::max(),
        serial_xml::from_string<unsigned>(std::to_string(std::numeric_limits<unsigned>::max())));
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)serial_xml::from_string<unsigned>(std::to_string(
          static_cast<unsigned long long>(std::numeric_limits<unsigned>::max()) + 1));
    });
  }

  [[= test{}]] void every_standard_integer_width_and_xml_fast_path() {
    auto increment_decimal = [](std::string text) {
      for (auto i = text.size(); i > 0; --i) {
        if (text[i - 1] != '9') {
          ++text[i - 1];
          return text;
        }
        text[i - 1] = '0';
      }
      return "1" + text;
    };
    auto check = [&]<typename T>() {
      using WidthInput = IntegerWidthInput<T>;
      for (const T expected :
           {std::numeric_limits<T>::lowest(), T{0}, std::numeric_limits<T>::max()}) {
        char buffer[32];
        const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), expected);
        assert_equal(std::errc{}, error);
        const std::string text(buffer, end);
        scoped_trace trace(std::format("integer width {}, signed {}, value {}", sizeof(T),
                                       std::is_signed_v<T>, text));
        expect_equal(expected, serial_xml::from_string<T>(text));
        const auto xml = clean_to_xml(WidthInput{expected, expected});
        expect_equal(
            "<WidthInput attribute=\"" + text + "\"><text>" + text + "</text></WidthInput>", xml);
        const auto parsed = serial_xml::from_xml<WidthInput>(xml);
        expect_equal(expected, parsed.attribute);
        expect_equal(expected, parsed.text);
      }
      const auto above_max = increment_decimal(std::to_string(std::numeric_limits<T>::max()));
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<T>(above_max); });
      if constexpr (std::is_signed_v<T>) {
        auto minimum = std::to_string(std::numeric_limits<T>::min());
        const auto below_min = "-" + increment_decimal(minimum.substr(1));
        expect_throws<serial_xml::deserialization_error>(
            [&] { (void)serial_xml::from_string<T>(below_min); });
      } else {
        expect_throws<serial_xml::deserialization_error>(
            [] { (void)serial_xml::from_string<T>("-0"); });
      }
    };
    check.template operator()<signed char>();
    check.template operator()<unsigned char>();
    check.template operator()<short>();
    check.template operator()<unsigned short>();
    check.template operator()<int>();
    check.template operator()<unsigned>();
    check.template operator()<long>();
    check.template operator()<unsigned long>();
    check.template operator()<long long>();
    check.template operator()<unsigned long long>();
  }

  [[= test{}]] void invalid_boolean_spellings_and_whitespace() {
    for (const auto invalid : {"", " ", "TRUE", "False", "2", "-1", "truefalse", "1x"}) {
      scoped_trace trace(invalid);
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<bool>(invalid); });
    }
    expect_true(serial_xml::from_string<bool>(" \ttrue\r\n"));
    expect_false(serial_xml::from_string<bool>(" \t0\r\n"));
  }

  [[= test{}]] void floating_point_extremes_and_negative_zero() {
    auto check = []<typename T>() {
      for (const T expected :
           {std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max(),
            std::numeric_limits<T>::min(), std::numeric_limits<T>::denorm_min()}) {
        char buffer[128];
        const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), expected);
        assert_equal(std::errc{}, error);
        const std::string text(buffer, end);
        scoped_trace trace(text);
        expect_equal(expected, serial_xml::from_string<T>(text));
      }
      const auto negative_zero = serial_xml::from_string<T>("-0.0e0");
      expect_equal(T{0}, negative_zero);
      expect_true(std::signbit(negative_zero));
      expect_false(std::signbit(serial_xml::from_string<T>("0.0")));
    };
    check.template operator()<float>();
    check.template operator()<double>();
    check.template operator()<long double>();
  }

  [[= test{}]] void floating_point_rejects_range_errors_and_malformed_exponents() {
    for (const std::string input :
         {"1e99999", "-1e99999", "1e-99999", "-1e-99999", "1e", "1e+", "1e-", "1e2tail", "1.2.3",
          ".", "+1.0", "--1.0", "nan trailing", "infinity trailing"}) {
      scoped_trace trace(input);
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<float>(input); });
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<double>(input); });
    }
    // Values outside float's range remain representable as doubles.
    expect_throws<serial_xml::deserialization_error>(
        [] { (void)serial_xml::from_string<float>("1e39"); });
    expect_true(std::isfinite(serial_xml::from_string<double>("1e39")));
    expect_throws<serial_xml::deserialization_error>(
        [] { (void)serial_xml::from_string<float>("1e-50"); });
    expect_true(serial_xml::from_string<double>("1e-50") > 0);
  }

  [[= test{}]] void floating_point_nonfinite_values_follow_from_chars() {
    for (const auto input : {"nan", "NaN", "nan(payload)", "-nan"}) {
      scoped_trace trace(input);
      expect_true(std::isnan(serial_xml::from_string<float>(input)));
      expect_true(std::isnan(serial_xml::from_string<double>(input)));
    }
    for (const auto input : {"inf", "infinity", "INF"}) {
      scoped_trace trace(input);
      const auto positive = serial_xml::from_string<double>(input);
      expect_true(std::isinf(positive));
      expect_false(std::signbit(positive));
      const auto negative = serial_xml::from_string<double>(std::string("-") + input);
      expect_true(std::isinf(negative));
      expect_true(std::signbit(negative));
    }
  }

  [[= test{}]] void containers_and_custom_types() {
    using Nested = std::vector<std::vector<int>>;
    expect_equal((Nested{{1, 2}, {}, {3}}), serial_xml::from_string<Nested>("[[1, 2], [], [3]]"));
    using Strings = std::vector<std::string>;
    expect_equal((Strings{"a,b", "[x]", "a\"b", "\n"}),
                 serial_xml::from_string<Strings>("[\"a,b\", \"[x]\", \"a\\\"b\", \"\\n\"]"));
    using Array = std::array<int, 2>;
    expect_equal((Array{1, 2}), serial_xml::from_string<Array>("[1, 2]"));
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_string<Array>("[1]")); });
    expect_equal((std::deque<int>{1, 2}), serial_xml::from_string<std::deque<int>>("[1, 2]"));
    expect_equal((std::list<int>{1, 2}), serial_xml::from_string<std::list<int>>("[1, 2]"));
    expect_equal((std::forward_list<int>{1, 2}),
                 serial_xml::from_string<std::forward_list<int>>("[1, 2]"));
    expect_equal((std::set<int>{1, 2}), serial_xml::from_string<std::set<int>>("{2, 1}"));
    using Map = std::map<std::string, std::vector<int>>;
    expect_equal((Map{{"a:b", {1, 2}}, {"z", {}}}),
                 serial_xml::from_string<Map>("{\"a:b\": [1, 2], \"z\": []}"));
    using Tuple = std::tuple<int, std::string, std::vector<int>>;
    expect_equal((Tuple{3, "hi", {4, 5}}), serial_xml::from_string<Tuple>("(3, \"hi\", [4, 5])"));
    expect_equal(7, serial_xml::from_string<std::optional<int>>("7"));
    auto values = serial_xml::from_string<std::valarray<int>>("[1, 2]");
    assert_equal(2u, values.size());
    expect_equal(1, values[0]);
    expect_equal(2, values[1]);
    expect_equal(17, serial_xml::from_string<DeserializationLeaf>("17").value);
    for (auto bad : {"[1,]", "[1,,2]", "[1", "[(1]]", "[\"oops]"}) {
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)(serial_xml::from_string<std::vector<int>>(bad)); });
    }
  }

  [[= test{}]] void every_debug_escape_and_unicode_width() {
    using Strings = std::vector<std::string>;
    expect_equal(
        Strings{"\\", "\"", "'", "\n", "\r", "\t", std::string(1, '\0'), std::string(1, '\x7f'),
                "é", "€", "😀"},
        serial_xml::from_string<Strings>(
            R"(["\\", "\"", "\'", "\n", "\r", "\t", "\u{0}", "\u{7f}", "\u{e9}", "\u{20ac}", "\u{1f600}"])"));
    for (const std::string input :
         {R"(["\q"])", R"(["\u0041"])", R"(["\u{xyz}"])", R"(["\u{100000000}"])",
          R"(["\u{110000}"])", R"(["\u{DFFF}"])", R"(["a"b"])", "[\"trailing\\\"]", "[\"unclosed]",
          "[[1,2)]", "[1}]"}) {
      scoped_trace trace(input);
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<Strings>(input); });
    }
    for (const auto input : {"['']", "['ab']", "['é']"}) {
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<std::vector<char>>(input); });
    }
  }

  [[= test{}]] void empty_nested_and_malformed_associative_representations() {
    using Map = std::map<std::string, std::vector<int>>;
    expect_true(serial_xml::from_string<Map>("{}").empty());
    expect_equal(Map{{"", {}}}, serial_xml::from_string<Map>(R"({"": []})"));
    expect_equal(Map{{"a,{b}:c", {1, 2}}, {"empty", {}}},
                 serial_xml::from_string<Map>(R"({"a,{b}:c": [1, 2], "empty": []})"));
    for (const auto input :
         {"{missing}", "{\"key\":}", "{\"x\": [1,]}", "{\"x\": [1}", "{\"x\": [1],}"}) {
      scoped_trace trace(input);
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<Map>(input); });
    }
    expect_equal(std::tuple<>{}, serial_xml::from_string<std::tuple<>>("()"));
    for (const auto input : {"()", "(1)", "(1, 2, 3)"}) {
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)serial_xml::from_string<std::tuple<int, int>>(input); });
    }
    expect_equal(std::vector<char>{'A', 'B'},
                 serial_xml::from_string<std::vector<char>>("[65,66]"));
    expect_equal(Map{{"a\"b\\c", {1}}}, serial_xml::from_string<Map>(R"({"a\"b\\c": [1]})"));
    using NestedKeys = std::map<std::tuple<int, int>, int>;
    expect_equal(NestedKeys{{{1, 2}, 3}}, serial_xml::from_string<NestedKeys>("{(1,2): 3}"));
    expect_true(serial_xml::from_string<std::unordered_map<std::string, int>>("{}").empty());
    expect_equal(std::unordered_set<int>{1, 2},
                 serial_xml::from_string<std::unordered_set<int>>("{2,1,2}"));
    expect_equal(std::multiset<int>{1, 1, 2},
                 serial_xml::from_string<std::multiset<int>>("{1,2,1}"));
  }

  [[= test{}]] void character_ranges_and_malformed_quotes() {
    expect_equal((std::vector<char>{'a', '\n', '\''}),
                 serial_xml::from_string<std::vector<char>>("['a', '\\n', '\\\'']"));
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_string<std::vector<std::string>>("[\"a\" \"b\"]")); });
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_string<int>(std::string_view{})); });
  }

  [[= test{}]] void debug_unicode_escapes() {
    using Strings = std::vector<std::string>;
    expect_equal((Strings{std::string("\x01", 1), "😀"}),
                 serial_xml::from_string<Strings>("[\"\\u{1}\", \"\\u{1f600}\"]"));
    auto values = Strings{std::string("\x01", 1), "quote\" newline\n"};
    expect_equal(values, serial_xml::from_string<Strings>(std::format("{}", values)));
    for (auto input : {"[\"\\u{}\"]", "[\"\\u{D800}\"]", "[\"\\u{110000}\"]", "[\"\\u{1\"]"}) {
      expect_throws<serial_xml::deserialization_error>(
          [&] { (void)(serial_xml::from_string<Strings>(input)); });
    }
  }
};

struct FromXmlTests {
  [[= test{}]] void named_attributes_children_nested_and_skipped() {
    struct NamedInputChild {
      int number;
    };
    struct[[= serial_xml::name{"record"}]] NamedInputRecord {
      [[ = serial_xml::attribute, = serial_xml::name{"id"} ]] int identifier;
      [[= serial_xml::name{"title"}]] std::string text;
      NamedInputChild child;
      [[= serial_xml::skip]] int skipped = 99;
      bool enabled;
      double fraction;
    };
    NamedInputRecord original{12, "<hello> & \"world\"", {7}, 88, true, 1.5};
    auto xml = serial_xml::to_xml(original);
    expect_equal(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><record id=\"12\">"
        "<title>&lt;hello&gt; &amp; &quot;world&quot;</title>"
        "<child><number>7</number></child><enabled>1</enabled>"
        "<fraction>1.5</fraction></record>",
        xml);
    auto parsed = serial_xml::from_xml<NamedInputRecord>(xml);
    expect_equal(original.identifier, parsed.identifier);
    expect_equal(original.text, parsed.text);
    expect_equal(7, parsed.child.number);
    expect_equal(99, parsed.skipped);
    expect_true(parsed.enabled);
    expect_equal(1.5, parsed.fraction);
    expect_equal(xml, serial_xml::to_xml(parsed));
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_xml<NamedInputRecord>("<wrong/>")); });
    expect_equal(12, serial_xml::from_xml<NamedInputRecord>(
                         serial_xml::to_xml(original, false, "fixed"), "fixed")
                         .identifier);
  }

  [[= test{}]] void required_and_optional_members() {
    struct RequiredInputRecord {
      [[= serial_xml::attribute]] int id = 3;
      int count = 5;
      [[= serial_xml::optional]] std::string label = "default";
      std::optional<int> maybe = 9;
      [[= serial_xml::exclude_on_empty]] std::vector<int> values{10};
      [[= serial_xml::optional]] std::optional<int> retained = 55;
      [[ = serial_xml::optional, = serial_xml::exclude_on_empty ]] std::vector<int> kept{77};
    };
    auto value = serial_xml::from_xml<RequiredInputRecord>(
        "<RequiredInputRecord id='2'><count>0</count></RequiredInputRecord>");
    expect_equal(2, value.id);
    expect_equal(0, value.count);
    expect_equal("default", value.label);
    expect_false(value.maybe.has_value());
    expect_true(value.values.empty());
    expect_equal(55, value.retained);
    expect_equal((std::vector<int>{77}), value.kept);
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<RequiredInputRecord>(
          "<RequiredInputRecord><count>1</count></RequiredInputRecord>"));
    });
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<RequiredInputRecord>("<RequiredInputRecord id='1'/>"));
    });
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<RequiredInputRecord>(
          "<RequiredInputRecord id='1'><count/></RequiredInputRecord>"));
    });
    struct OptionalAttributeInput {
      [[ = serial_xml::attribute, = serial_xml::optional ]] int value = 42;
    };
    expect_equal(42,
                 serial_xml::from_xml<OptionalAttributeInput>("<OptionalAttributeInput/>").value);
  }

  [[= test{}]] void owning_ranges_and_iteration() {
    struct RangeInputItem {
      int value;
    };
    struct RangeInputRecord {
      std::vector<int> vector;
      std::array<int, 2> array;
      std::deque<std::string> deque;
      std::inplace_vector<int, 3> inplace;
      std::valarray<int> valarray;
      std::vector<RangeInputItem> objects;
      [[= serial_xml::iter{"item", "list"}]] std::list<int> list;
      [[= serial_xml::no_iter]] std::vector<int> formatted;
    };
    RangeInputRecord original{{1, 2}, {3, 4},      {"a", "b"}, {5, 6},
                              {7, 8}, {{9}, {10}}, {11, 12},   {13, 14}};
    auto xml = serial_xml::to_xml(original, false);
    auto result = serial_xml::from_xml<RangeInputRecord>(xml);
    expect_equal(xml, serial_xml::to_xml(result, false));
    expect_equal(original.vector, result.vector);
    expect_equal(original.array, result.array);
    expect_equal(original.deque, result.deque);
    expect_equal(original.inplace, result.inplace);
    assert_equal(original.valarray.size(), result.valarray.size());
    for (std::size_t i = 0; i < result.valarray.size(); ++i) {
      expect_equal(original.valarray[i], result.valarray[i], std::format("valarray index {}", i));
    }
    assert_equal(std::size_t{2}, result.objects.size());
    expect_equal(9, result.objects[0].value);
    expect_equal(10, result.objects[1].value);
    expect_equal(original.list, result.list);
    expect_equal(original.formatted, result.formatted);
    struct ExplicitForwardInput {
      [[= serial_xml::iter{"item"}]] std::forward_list<int> values;
    };
    ExplicitForwardInput forward{{1, 2}};
    expect_equal(
        serial_xml::to_xml(forward, false),
        serial_xml::to_xml(serial_xml::from_xml<ExplicitForwardInput>(serial_xml::to_xml(forward)),
                           false));
    struct EmptyRangeInput {
      std::vector<int> values;
    };
    expect_true(
        serial_xml::from_xml<EmptyRangeInput>("<EmptyRangeInput><values/></EmptyRangeInput>")
            .values.empty());
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_xml<EmptyRangeInput>("<EmptyRangeInput/>")); });
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<EmptyRangeInput>(
          "<EmptyRangeInput><values><bad>1</bad></values></EmptyRangeInput>"));
    });
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<EmptyRangeInput>(
          "<EmptyRangeInput><values>1</values></EmptyRangeInput>"));
    });
  }

  [[= test{}]] void iterated_maps_with_const_keys() {
    struct MapInputRecord {
      [[= serial_xml::iter{"entry"}]] std::map<int, int> numbers;
      [[= serial_xml::iter{"entry"}]] std::map<std::string, int> strings;
    };
    MapInputRecord original{{{1, 2}, {3, 4}}, {{"a:b", 5}, {"hello world", 6}}};
    auto xml = serial_xml::to_xml(original, false);
    auto result = serial_xml::from_xml<MapInputRecord>(xml);
    expect_equal(original.numbers, result.numbers);
    expect_equal(original.strings, result.strings);
    expect_equal(xml, serial_xml::to_xml(result, false));
  }

  [[= test{}]] void raw_and_cdata() {
    struct RawTextInput {
      int number;
      [[= serial_xml::raw]] std::string text;
    };
    RawTextInput raw{3, "<&hello>"};
    expect_equal(raw.text, serial_xml::from_xml<RawTextInput>(serial_xml::to_xml(raw)).text);
    struct CdataInput {
      [[= serial_xml::cdata]] std::string first;
      [[= serial_xml::cdata]] std::string second;
    };
    CdataInput data{"hello<&>", ""};
    auto parsed = serial_xml::from_xml<CdataInput>(serial_xml::to_xml(data));
    expect_equal(data.first, parsed.first);
    expect_equal(data.second, parsed.second);
    struct RawRangeInput {
      [[ = serial_xml::raw, = serial_xml::iter{"item"} ]] std::vector<int> values;
    };
    RawRangeInput range{{1, 2}};
    expect_equal(range.values,
                 serial_xml::from_xml<RawRangeInput>(serial_xml::to_xml(range)).values);
    expect_true(serial_xml::from_xml<RawRangeInput>("<RawRangeInput/>").values.empty());
  }

  [[= test{}]] void leaf_conversion_and_formats() {
    struct LeafInputRecord {
      [[= serial_xml::format{format_deserialization_leaf}]] DeserializationLeaf leaf;
      [[= serial_xml::format{"04d"}]] int padded;
      [[= serial_xml::no_unpack]] DeserializationLeaf explicit_leaf;
    };
    auto result = serial_xml::from_xml<LeafInputRecord>(
        "<LeafInputRecord><leaf>7</leaf><padded>0012</padded><explicit_leaf>13</explicit_leaf></"
        "LeafInputRecord>");
    expect_equal(7, result.leaf.value);
    expect_equal(12, result.padded);
    expect_equal(13, result.explicit_leaf.value);
  }

  [[= test{}]] void encapsulation_with_automatic_setter_matching() {
    SetterRecord original;
    original.value(7);
    original.set_title("test");
    original.setCount(9);
    auto result = serial_xml::from_xml<SetterRecord>(serial_xml::to_xml(original));
    expect_equal(7, result.value());
    expect_equal("test", result.get_title());
    expect_equal(9, result.getCount());
    expect_equal(77, result.read_only());
  }

  [[= test{}]] void automatic_setters_preserve_parameter_categories() {
    class ReferenceSetterInput {
     public:
      int number() const { return number_; }
      void set_number(int& value) { number_ = value; }
      std::string title() const { return title_; }
      void set_title(std::string&& value) { title_ = std::move(value); }
      std::string label() const { return label_; }
      void set_label(const std::string& value) { label_ = value; }

     private:
      int number_ = 0;
      std::string title_;
      std::string label_;
    };
    auto result = serial_xml::from_xml<ReferenceSetterInput>(
        "<ReferenceSetterInput><number>42</number><title>moved</title><label>copied</label>"
        "</ReferenceSetterInput>");
    expect_equal(42, result.number());
    expect_equal("moved", result.title());
    expect_equal("copied", result.label());
  }

  [[= test{}]] void annotated_and_mock_setters_preserve_parameter_categories() {
    class AnnotatedReferenceInput {
     public:
      [[= serial_xml::setter]] void set_number(int& value) { number_ = value; }
      [[= serial_xml::setter]] void set_title(std::string&& value) { title_ = std::move(value); }
      [[= serial_xml::skip]] int number() const { return number_; }
      [[= serial_xml::skip]] std::string title() const { return title_; }

     private:
      int number_ = 0;
      std::string title_;
    };
    auto result = serial_xml::from_xml<AnnotatedReferenceInput>(
        "<AnnotatedReferenceInput><number>42</number><title>moved</title>"
        "</AnnotatedReferenceInput>");
    expect_equal(42, result.number());
    expect_equal("moved", result.title());
    struct[[= serial_xml::name{"record"}]] ReferenceSetterSchema {
      [[= serial_xml::setter]] void set_number(int);
      [[= serial_xml::setter]] void set_title(std::string);
    };
    auto mock_result = serial_xml::from_xml<AnnotatedReferenceInput, ReferenceSetterSchema>(
        "<record><number>7</number><title>mock</title></record>");
    expect_equal(7, mock_result.number());
    expect_equal("mock", mock_result.title());
  }

  [[= test{}]] void independent_annotated_setters() {
    auto value = serial_xml::from_xml<IndependentSetters>(
        "<IndependentSetters id='3'><title>hello</title></IndependentSetters>");
    expect_equal(3, value.id());
    expect_equal("hello", value.title());
    expect_equal(5, value.count());
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_xml<IndependentSetters>("<IndependentSetters id='3'/>")); });
  }

  [[= test{}]] void annotated_setter_overrides_getter_and_supports_mock() {
    class SetterMockTarget {
     public:
      int value() const { return value_; }
      void set_value(int value) {
        value_ = value;
        ++calls;
      }
      [[= serial_xml::skip]] int calls = 0;

     private:
      int value_ = 0;
    };
    struct[[= serial_xml::name{"record"}]] SetterMockSchema {
      int value() const;
      [[ = serial_xml::setter, = serial_xml::name{"number"} ]] void set_value(int);
    };
    auto result = serial_xml::from_xml<SetterMockTarget, SetterMockSchema>(
        "<record><number>42</number></record>");
    expect_equal(42, result.value());
    expect_equal(1, result.calls);
  }

  [[= test{}]] void mock_fields_and_in_place_non_default_construction() {
    struct FieldMockTarget {
      explicit FieldMockTarget(int input) : value(input) {}
      int value;
      int other = 99;
    };
    struct[[= serial_xml::name{"record"}]] FieldMockSchema {
      [[ = serial_xml::attribute, = serial_xml::name{"number"} ]] int value;
      [[= serial_xml::skip]] int missing;
    };
    FieldMockTarget result{7};
    serial_xml::from_xml<FieldMockSchema>(result, "<record number='42'/>");
    expect_equal(42, result.value);
    expect_equal(99, result.other);
  }

  [[= test{}]] void entities_unicode_comments_and_cdata_in_scalar() {
    struct EntityTextInput {
      std::string value;
    };
    auto result = serial_xml::from_xml<EntityTextInput>(
        "\xEF\xBB\xBF<?xml version='1.0'?><!--before--><EntityTextInput><value>"
        "&lt;&gt;&amp;&quot;&apos;&#65;&#x1F600;<!--ignored--><![CDATA[<&>]]>"
        "</value></EntityTextInput><!--after-->");
    expect_equal("<>&\"'A😀<&>", result.value);
    expect_equal("line\nbreak",
                 serial_xml::from_xml<EntityTextInput>(
                     "<EntityTextInput><value>line\r\nbreak</value></EntityTextInput>")
                     .value);
    struct EntityNumberInput {
      int value;
    };
    expect_equal(42, serial_xml::from_xml<EntityNumberInput>(
                         "<EntityNumberInput>\n <value>\n 42\n </value>\n</EntityNumberInput>")
                         .value);
  }

  [[
    = test{}, = parameter_source<&malformed_xml_cases>{}
  ]] void rejects_malformed_xml_and_duplicate_members(const std::string& xml) {
    struct MalformedInputRecord {
      int value;
    };
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)serial_xml::from_xml<MalformedInputRecord>(xml); });
  }

  [[= test{}]] void rejects_malformed_utf8_text() {
    struct MalformedTextInput {
      std::string value;
    };
    for (std::string bad :
         {std::string("\xC0\x80"), std::string("\xED\xA0\x80"), std::string("\x01", 1)}) {
      expect_throws<serial_xml::deserialization_error>([&] {
        (void)(serial_xml::from_xml<MalformedTextInput>("<MalformedTextInput><value>" + bad +
                                                        "</value></MalformedTextInput>"));
      });
    }
  }

  [[= test{}]] void optional_values_and_formatted_range_items() {
    struct OptionalInputChild {
      int value;
    };
    struct OptionalInputRecord {
      [[= serial_xml::attribute]] std::optional<int> id;
      std::optional<std::string> title;
      [[= serial_xml::unpack]] std::optional<OptionalInputChild> child;
      std::optional<int> number;
      std::optional<std::string> text;
      [[= serial_xml::format{format_deserialization_leaf}]] std::vector<DeserializationLeaf> leaves;
    };
    OptionalInputRecord original{7, "text", OptionalInputChild{9}, 11, "<&>", {{13}, {14}}};
    auto xml = serial_xml::to_xml(original);
    auto restored = serial_xml::from_xml<OptionalInputRecord>(xml);
    assert_true(restored.child.has_value());
    expect_equal(9, restored.child->value);
    expect_equal(original.leaves, restored.leaves);
    expect_equal(11, restored.number);
    expect_equal("<&>", restored.text);
    expect_equal(xml, serial_xml::to_xml(restored));
    auto empty = serial_xml::from_xml<OptionalInputRecord>(
        "<OptionalInputRecord><leaves/></OptionalInputRecord>");
    expect_false(empty.id);
    expect_false(empty.title);
    expect_false(empty.child);
    expect_false(empty.number);
    expect_false(empty.text);
    struct FormattedCdataNumberInput {
      [[ = serial_xml::cdata, = serial_xml::format{"04d"} ]] int value;
    };
    auto number_xml = serial_xml::to_xml(FormattedCdataNumberInput{16});
    expect_equal(16, serial_xml::from_xml<FormattedCdataNumberInput>(number_xml).value);
  }

  [[= test{}]] void nested_and_range_setters() {
    struct SetterInputChild {
      int value;
    };
    class SetterInputRecord {
     public:
      [[ = serial_xml::setter, = serial_xml::name{"child"} ]] void load(SetterInputChild value) {
        child = value;
      }
      [[ = serial_xml::setter,
         = serial_xml::iter{"entry", "list"} ]] void set_values(const std::vector<int>& value) {
        values = value;
      }
      [[= serial_xml::skip]] SetterInputChild child{};
      [[= serial_xml::skip]] std::vector<int> values;
    };
    auto value = serial_xml::from_xml<SetterInputRecord>(
        "<SetterInputRecord><child><value>7</value></child>"
        "<list><entry>8</entry><entry>9</entry></list></SetterInputRecord>");
    expect_equal(7, value.child.value);
    expect_equal((std::vector<int>{8, 9}), value.values);
    expect_equal("<SetterInputRecord/>", serial_xml::to_xml(value, false));
  }

  [[= test{}]] void const_annotated_setter_and_readonly_getter() {
    class ConstSetterInput {
     public:
      [[= serial_xml::setter]] int set_value(int input = 0) const {
        value = input;
        return input;
      }
      std::vector<DeserializationLeaf> calculated() const { return {}; }
      [[= serial_xml::skip]] mutable int value = 0;
    };
    auto input = serial_xml::from_xml<ConstSetterInput>(
        "<ConstSetterInput><value>7</value></ConstSetterInput>");
    expect_equal(7, input.value);
    // Setter methods never participate in serialization, even when const and callable with no
    // arguments.
    expect_equal("<ConstSetterInput><calculated/></ConstSetterInput>",
                 serial_xml::to_xml(input, false));
  }

  [[= test{}]] void automatic_forward_list_and_container_limits() {
    struct ForwardListInput {
      std::forward_list<int> values;
    };
    ForwardListInput original{{1, 2, 3}};
    auto xml = serial_xml::to_xml(original);
    expect_equal(original.values, serial_xml::from_xml<ForwardListInput>(xml).values);
    struct LimitedInput {
      std::inplace_vector<int, 1> values;
    };
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<LimitedInput>(
          "<LimitedInput><values><element>1</element><element>2</element></values></"
          "LimitedInput>"));
    });
    struct ArrayInput {
      std::array<int, 2> values;
    };
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<ArrayInput>(
          "<ArrayInput><values><element>1</element></values></ArrayInput>"));
    });
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_xml<ArrayInput>("<ArrayInput><values/></ArrayInput>")); });
  }

  [[= test{}]] void declaration_validation_and_depth_limit() {
    struct DeclarationInput {};
    expect_no_throw([&] {
      (void)(serial_xml::from_xml<DeclarationInput>(
          "<?xml version='1.0' encoding='utf-8' standalone='yes'?><DeclarationInput/>"));
    });
    for (auto declaration :
         {"<?xml?>", "<?XML version='1.0'?>", "<?xml version='1.1'?>", "<?xml garbage='1'?>",
          "<?xml version='1.0' version='1.0'?>", "<?xml version='1.0' encoding='UTF-16'?>",
          "<?xml version='1.0' standalone='maybe'?>"}) {
      expect_throws<serial_xml::deserialization_error>([&] {
        (void)(serial_xml::from_xml<DeclarationInput>(std::string(declaration) +
                                                      "<DeclarationInput/>"));
      });
    }
    std::string xml = "<DeclarationInput>";
    for (int i = 0; i < 256; ++i) xml += "<x>";
    for (int i = 0; i < 256; ++i) xml += "</x>";
    xml += "</DeclarationInput>";
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_xml<DeclarationInput>(xml)); });
  }

  [[
    = test{}, = parameter_source<&xml_diagnostic_cases>{}
  ]] void malformed_xml_diagnostics(const std::string& xml, const std::string& diagnostic) {
    struct CorpusInput {
      std::string value;
    };
    try {
      (void)serial_xml::from_xml<CorpusInput>(xml, "r");
      fail("Expected XML rejection");
    } catch (const serial_xml::deserialization_error& error) {
      expect_true(std::string_view(error.what()).contains(diagnostic), error.what());
      expect_true(std::string_view(error.what()).contains(" at byte ") ||
                  diagnostic == "Unexpected nested element" || diagnostic == "Duplicate element");
    }
  }

  [[= test{}]] void literal_namespace_names_and_unknown_members() {
    struct[[= serial_xml::name{"p:r"}]] NamespaceInput {
      [[= serial_xml::name{"p:value"}]] std::string value;
    };
    const auto parsed = serial_xml::from_xml<NamespaceInput>(
        "<p:r xmlns:p='urn:one' "
        "ignored='ok'><unused/><p:value>yes</p:value><unknown>text</unknown></p:r>");
    expect_equal("yes", parsed.value);
    // Prefixes are matched literally, not resolved through namespace declarations.
    expect_throws<serial_xml::deserialization_error>([] {
      (void)serial_xml::from_xml<NamespaceInput>(
          "<q:r xmlns:q='urn:one'><q:value>yes</q:value></q:r>");
    });
  }

  [[= test{}]] void xml_references_at_each_utf8_width_and_legal_endpoint() {
    struct ReferenceInput {
      std::string value;
    };
    for (const unsigned cp : {9u, 10u, 13u, 32u, 127u, 128u, 0x7ffu, 0x800u, 0xd7ffu, 0xe000u,
                              0xfffdu, 0x10000u, 0x10ffffu}) {
      scoped_trace trace(std::format("reference U+{:X}", cp));
      const auto expected = utf8_scalar(cp);
      for (const auto reference : {std::format("&#{};", cp), std::format("&#x{:x};", cp)}) {
        expect_equal(expected,
                     serial_xml::from_xml<ReferenceInput>("<ReferenceInput><value>" + reference +
                                                          "</value></ReferenceInput>")
                         .value);
      }
    }
  }

  [[= test{}]] void every_xml10_character_round_trips() {
    struct UnicodeDomain {
      [[= serial_xml::attribute]] std::string attribute;
      std::string text;
      [[= serial_xml::raw]] std::string raw;
      [[= serial_xml::cdata]] std::string cdata;
    };
    std::string domain = "\t\n\r";
    domain.reserve(4400000);
    for (unsigned cp = 0x20; cp <= 0x10ffff; ++cp) {
      if ((cp >= 0xd800 && cp <= 0xdfff) || cp == 0xfffe || cp == 0xffff) continue;
      domain += utf8_scalar(cp);
    }
    const auto xml = clean_to_xml(UnicodeDomain{domain, domain, domain, domain});
    const auto parsed = serial_xml::from_xml<UnicodeDomain>(xml);
    expect_equal(domain, parsed.attribute);
    expect_equal(domain, parsed.text);
    expect_equal(domain, parsed.raw);
    expect_equal(domain, parsed.cdata);
    for (unsigned cp = 0; cp <= 0xffff; ++cp) {
      if (!(cp < 0x20 && cp != 9 && cp != 10 && cp != 13) && !(cp >= 0xd800 && cp <= 0xdfff) &&
          cp != 0xfffe && cp != 0xffff)
        continue;
      const auto invalid = utf8_scalar(cp);
      scoped_trace trace(std::format("invalid XML character U+{:X}", cp));
      expect_throws<std::invalid_argument>(
          [&] { (void)clean_to_xml(UnicodeDomain{invalid, "", "", ""}); });
      expect_throws<serial_xml::deserialization_error>([&] {
        (void)serial_xml::from_xml<UnicodeDomain>("<UnicodeDomain attribute=''><text>" + invalid +
                                                  "</text></UnicodeDomain>");
      });
    }
  }

  [[= test{}]] void optional_raw_range_presence_overrides_defaults() {
    struct OptionalRawRange {
      [[ = serial_xml::raw, = serial_xml::optional, = serial_xml::iter{"item"} ]] std::vector<int>
          values{9};
    };
    const auto absent =
        serial_xml::from_xml<OptionalRawRange>("<OptionalRawRange><other/></OptionalRawRange>");
    expect_equal(std::vector<int>{9}, absent.values);
    const auto present = serial_xml::from_xml<OptionalRawRange>(
        "<OptionalRawRange><other/><item>1</item><other/><item>2</item></OptionalRawRange>");
    expect_equal(std::vector<int>{1, 2}, present.values);
  }

  [[= test{}]] void raw_ranges_skip_unrelated_siblings_and_preserve_order() {
    struct RawRanges {
      int head;
      [[ = serial_xml::raw, = serial_xml::iter{"item"} ]] std::vector<int> values;
      int tail;
    };
    const auto parsed = serial_xml::from_xml<RawRanges>(
        "<RawRanges><head>1</head><unknown/><item>2</item><tail>4</tail><item>3</item></"
        "RawRanges>");
    expect_equal(1, parsed.head);
    expect_equal(std::vector<int>{2, 3}, parsed.values);
    expect_equal(4, parsed.tail);
    expect_equal(
        std::string(
            "<RawRanges><head>1</head><item>2</item><item>3</item><tail>4</tail></RawRanges>"),
        clean_to_xml(parsed));
  }

  [[= test{}]] void accepts_exact_depth_limit_and_rejects_the_next_level() {
    struct DepthInput {};
    // Root is depth zero: 255 nested elements are allowed, the 256th is not.
    for (const int nested : {254, 255, 256, 257}) {
      scoped_trace trace(std::format("nested elements: {}", nested));
      std::string xml = "<DepthInput>";
      for (int i = 0; i < nested; ++i) xml += "<x>";
      for (int i = 0; i < nested; ++i) xml += "</x>";
      xml += "</DepthInput>";
      if (nested <= 255) {
        expect_no_throw([&] { (void)serial_xml::from_xml<DepthInput>(xml); });
      } else {
        try {
          (void)serial_xml::from_xml<DepthInput>(xml);
          fail("Expected nesting limit rejection");
        } catch (const serial_xml::deserialization_error& error) {
          expect_true(std::string_view(error.what()).contains("XML nesting limit exceeded"));
        }
      }
    }
  }

  [[= test{}]] void fixed_capacity_and_array_boundaries() {
    struct CapacityInput {
      std::inplace_vector<int, 3> values;
    };
    struct ArrayInput {
      std::array<int, 3> values;
    };
    for (std::size_t count = 0; count <= 4; ++count) {
      scoped_trace trace(std::format("element count: {}", count));
      std::string elements;
      std::string formatted = "[";
      for (std::size_t i = 0; i < count; ++i) {
        elements += "<element>" + std::to_string(i) + "</element>";
        if (i) formatted += ", ";
        formatted += std::to_string(i);
      }
      formatted += "]";
      const auto capacity_xml = "<CapacityInput><values>" + elements + "</values></CapacityInput>";
      const auto array_xml = "<ArrayInput><values>" + elements + "</values></ArrayInput>";
      if (count <= 3) {
        CapacityInput parsed;
        assert_no_throw([&] { parsed = serial_xml::from_xml<CapacityInput>(capacity_xml); });
        assert_equal(count, parsed.values.size());
        std::inplace_vector<int, 3> from_text;
        assert_no_throw(
            [&] { from_text = serial_xml::from_string<std::inplace_vector<int, 3>>(formatted); });
        expect_equal(parsed.values, from_text);
        for (std::size_t i = 0; i < count; ++i) expect_equal(static_cast<int>(i), parsed.values[i]);
      } else {
        expect_throws<serial_xml::deserialization_error>(
            [&] { (void)serial_xml::from_xml<CapacityInput>(capacity_xml); });
        expect_throws<serial_xml::deserialization_error>(
            [&] { (void)serial_xml::from_string<std::inplace_vector<int, 3>>(formatted); });
      }
      if (count == 3) {
        expect_equal(std::array<int, 3>{0, 1, 2},
                     serial_xml::from_xml<ArrayInput>(array_xml).values);
        expect_equal(std::array<int, 3>{0, 1, 2},
                     serial_xml::from_string<std::array<int, 3>>(formatted));
      } else {
        expect_throws<serial_xml::deserialization_error>(
            [&] { (void)serial_xml::from_xml<ArrayInput>(array_xml); });
        expect_throws<serial_xml::deserialization_error>(
            [&] { (void)serial_xml::from_string<std::array<int, 3>>(formatted); });
      }
    }
    struct ZeroCapacity {
      std::inplace_vector<int, 0> values;
    };
    struct ZeroArray {
      std::array<int, 0> values;
    };
    expect_true(serial_xml::from_xml<ZeroCapacity>("<ZeroCapacity><values/></ZeroCapacity>")
                    .values.empty());
    expect_true(serial_xml::from_xml<ZeroArray>("<ZeroArray><values/></ZeroArray>").values.empty());
    expect_throws<serial_xml::deserialization_error>(
        [] { (void)serial_xml::from_string<std::inplace_vector<int, 0>>("[1]"); });
    expect_throws<serial_xml::deserialization_error>(
        [] { (void)serial_xml::from_string<std::array<int, 0>>("[1]"); });
  }

  [[= test{}]] void numeric_xml_preserves_nonfinite_values_and_zero_sign() {
    struct FloatingInput {
      [[= serial_xml::attribute]] double attribute;
      double child;
    };
    for (const double value :
         {-0.0, std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(),
          std::numeric_limits<double>::quiet_NaN()}) {
      const auto xml = clean_to_xml(FloatingInput{value, value});
      scoped_trace trace(xml);
      const auto parsed = serial_xml::from_xml<FloatingInput>(xml);
      for (const auto actual : {parsed.attribute, parsed.child}) {
        if (std::isnan(value))
          expect_true(std::isnan(actual));
        else {
          expect_equal(value, actual);
          expect_equal(std::signbit(value), std::signbit(actual));
        }
      }
    }
    expect_throws<serial_xml::deserialization_error>([] {
      (void)serial_xml::from_xml<FloatingInput>(
          "<FloatingInput attribute='1e99999'><child>0</child></FloatingInput>");
    });
  }

  [[= test{}]] void utf8_crosses_word_simd_and_buffer_boundaries() {
    struct UnicodeBoundary {
      [[= serial_xml::attribute]] std::string attribute;
      std::string text;
      [[= serial_xml::cdata]] std::string cdata;
    };
    for (const std::string codepoint : {"é", "€", "😀"}) {
      for (const std::size_t boundary : {8u, 16u, 32u, 64u, 128u, 256u, 4096u}) {
        for (std::size_t split = 1; split < codepoint.size(); ++split) {
          scoped_trace trace(
              std::format("codepoint {:?}, boundary {}, split {}", codepoint, boundary, split));
          const std::string prefix(boundary - split, 'a');
          const auto value = prefix + codepoint + "&<";
          const auto xml = clean_to_xml(UnicodeBoundary{value, value, value});
          expect_equal("<UnicodeBoundary attribute=\"" + prefix + codepoint + "&amp;&lt;\"><text>" +
                           prefix + codepoint + "&amp;&lt;</text><![CDATA[" + value +
                           "]]></UnicodeBoundary>",
                       xml);
          const auto parsed = serial_xml::from_xml<UnicodeBoundary>(xml);
          expect_equal(value, parsed.attribute);
          expect_equal(value, parsed.text);
          expect_equal(value, parsed.cdata);
          // Remove each possible suffix of the codepoint, leaving an incomplete sequence.
          const auto truncated = prefix + codepoint.substr(0, split);
          expect_throws<std::invalid_argument>(
              [&] { (void)clean_to_xml(UnicodeBoundary{truncated, "", ""}); });
          expect_throws<std::invalid_argument>(
              [&] { (void)clean_to_xml(UnicodeBoundary{"", truncated, ""}); });
          expect_throws<std::invalid_argument>(
              [&] { (void)clean_to_xml(UnicodeBoundary{"", "", truncated}); });
          for (const auto broken_xml : {"<UnicodeBoundary attribute='" + truncated +
                                            "'><text/><![CDATA[]]></UnicodeBoundary>",
                                        "<UnicodeBoundary attribute=''><text>" + truncated +
                                            "</text><![CDATA[]]></UnicodeBoundary>",
                                        "<UnicodeBoundary attribute=''><text/><![CDATA[" +
                                            truncated + "]]></UnicodeBoundary>"}) {
            expect_throws<serial_xml::deserialization_error>(
                [&] { (void)serial_xml::from_xml<UnicodeBoundary>(broken_xml); });
          }
        }
      }
    }
  }

  [[= test{}]] void conversion_failure_preserves_only_unvisited_members() {
    struct PartialInput {
      int first = 1;
      int second = 2;
      int last = 3;
    };
    PartialInput value;
    expect_throws<serial_xml::deserialization_error>([&] {
      serial_xml::from_xml(value,
                           "<PartialInput><first>10</first><second>invalid</second><last>30</"
                           "last></PartialInput>");
    });
    expect_equal(10, value.first);
    expect_equal(2, value.second);
    expect_equal(3, value.last);
    // The object remains usable after the failed update.
    serial_xml::from_xml(
        value, "<PartialInput><first>11</first><second>22</second><last>33</last></PartialInput>");
    expect_equal(11, value.first);
    expect_equal(22, value.second);
    expect_equal(33, value.last);
  }

  [[= test{}]] void throwing_setter_propagates_and_stops_later_assignments() {
    struct ThrowingInput {
      int first = 1;
      [[= serial_xml::skip]] int stored = 2;
      [[= serial_xml::skip]] int calls = 0;
      [[ = serial_xml::setter, = serial_xml::name{"value"} ]] void set_value(int input) {
        ++calls;
        if (input < 0) throw std::domain_error("negative value");
        stored = input;
      }
      int last = 3;
    };
    ThrowingInput value;
    try {
      serial_xml::from_xml(
          value,
          "<ThrowingInput><first>10</first><value>-1</value><last>30</last></ThrowingInput>");
      fail("Expected setter exception");
    } catch (const std::domain_error& error) {
      expect_equal(std::string("negative value"), std::string(error.what()));
    }
    expect_equal(10, value.first);
    expect_equal(2, value.stored);
    expect_equal(1, value.calls);
    expect_equal(3, value.last);
    serial_xml::from_xml(
        value, "<ThrowingInput><first>11</first><value>22</value><last>33</last></ThrowingInput>");
    expect_equal(22, value.stored);
    expect_equal(2, value.calls);
    expect_equal(33, value.last);
    // A failed conversion must not invoke the setter at all.
    expect_throws<serial_xml::deserialization_error>([&] {
      serial_xml::from_xml(
          value,
          "<ThrowingInput><first>12</first><value>bad</value><last>44</last></ThrowingInput>");
    });
    expect_equal(12, value.first);
    expect_equal(22, value.stored);
    expect_equal(2, value.calls);
    expect_equal(33, value.last);
  }

  [[= test{}]] void setter_side_effects_are_not_rolled_back_on_exception() {
    struct SideEffectInput {
      [[= serial_xml::skip]] int stored = 1;
      [[= serial_xml::setter]] void load(int input) {
        stored = input;
        throw std::runtime_error("failure after mutation");
      }
      int last = 2;
    };
    SideEffectInput value;
    expect_throws_exact<std::runtime_error>([&] {
      serial_xml::from_xml(value,
                           "<SideEffectInput><load>7</load><last>8</last></SideEffectInput>");
    });
    expect_equal(7, value.stored);
    expect_equal(2, value.last);
  }

  [[= test{}]] void raw_scalar_ambiguity_and_attribute_normalization() {
    struct AmbiguousRawInput {
      [[= serial_xml::raw]] std::string first;
      [[= serial_xml::raw]] std::string second;
    };
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml<AmbiguousRawInput>("<AmbiguousRawInput>ab</AmbiguousRawInput>"));
    });
    struct NormalizedAttributeInput {
      [[= serial_xml::attribute]] std::string text;
    };
    auto result = serial_xml::from_xml<NormalizedAttributeInput>(
        "<NormalizedAttributeInput text='a\r\nb\tc&#10;d'/>");
    expect_equal("a b c\nd", result.text);
  }

  [[= test{}]] void attributes_with_container_representations() {
    struct ContainerAttributeInput {
      [[= serial_xml::attribute]] std::vector<int> values;
      [[= serial_xml::attribute]] std::optional<std::vector<int>> maybe;
    };
    ContainerAttributeInput original{{1, 2}, std::vector<int>{3, 4}};
    auto xml = serial_xml::to_xml(original, false);
    expect_equal("<ContainerAttributeInput values=\"[1, 2]\" maybe=\"[3, 4]\"/>", xml);
    auto restored = serial_xml::from_xml<ContainerAttributeInput>(xml);
    expect_equal(original.values, restored.values);
    expect_equal(original.maybe, restored.maybe);
  }

  [[= test{}]] void unicode_names_and_processing_instructions() {
    struct[[= serial_xml::name{"résumé"}]] UnicodeNameInput {
      [[= serial_xml::name{"état"}]] int value;
    };
    expect_equal(
        7, serial_xml::from_xml<UnicodeNameInput>(
               "<?xml-stylesheet href='example'?><résumé><?work test?><état>7</état></résumé>")
               .value);
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::from_xml<UnicodeNameInput>("<\xC2\x80/>")); });
    std::string override_name = "override";
    expect_equal(8, serial_xml::from_xml<UnicodeNameInput>("<override><état>8</état></override>",
                                                           override_name)
                        .value);
  }

  [[= test{}]] void escaping_across_simd_and_mask_boundaries() {
    struct EscapeBoundaryInput {
      [[= serial_xml::attribute]] std::string attribute;
      std::string text;
      [[= serial_xml::raw]] std::string raw;
    };
    auto reference_escape = [](std::string_view input) {
      std::string output;
      for (char c : input) {
        switch (c) {
          case '<':
            output += "&lt;";
            break;
          case '>':
            output += "&gt;";
            break;
          case '&':
            output += "&amp;";
            break;
          case '"':
            output += "&quot;";
            break;
          case '\'':
            output += "&apos;";
            break;
          default:
            output += c;
        }
      }
      return output;
    };
    for (std::size_t size = 0; size <= 150; ++size) {
      std::string value;
      for (std::size_t i = 0; i < size; ++i) value += "<&a>\"b'c"[i % 8];
      auto escaped = reference_escape(value);
      EscapeBoundaryInput original{value, value, value};
      auto xml = serial_xml::to_xml(original, false);
      expect_equal("<EscapeBoundaryInput attribute=\"" + escaped + "\"><text>" + escaped +
                       "</text>" + escaped + "</EscapeBoundaryInput>",
                   xml, std::format("{}", size));
      auto restored = serial_xml::from_xml<EscapeBoundaryInput>(xml);
      expect_equal(value, restored.attribute, std::format("{}", size));
      expect_equal(value, restored.text, std::format("{}", size));
      expect_equal(value, restored.raw, std::format("{}", size));
    }
    for (std::size_t position = 0; position < 140; ++position) {
      std::string value(140, 'a');
      value[position] = '&';
      auto restored = serial_xml::from_xml<EscapeBoundaryInput>(
          serial_xml::to_xml(EscapeBoundaryInput{value, value, value}));
      expect_equal(value, restored.text, std::format("{}", position));
    }
  }

  [[= test{}]] void raw_precedence_and_optional_raw_defaults() {
    struct RawPrecedenceChild {
      int value;
    };
    struct RawPrecedenceInput {
      [[= serial_xml::raw]] RawPrecedenceChild child;
      [[= serial_xml::raw]] std::optional<int> value;
    };
    RawPrecedenceInput original{{7}, 8};
    auto restored = serial_xml::from_xml<RawPrecedenceInput>(serial_xml::to_xml(original));
    expect_equal(7, restored.child.value);
    expect_equal(8, restored.value);
    struct OptionalRawInput {
      [[ = serial_xml::raw, = serial_xml::optional ]] std::string text = "default";
      [[ = serial_xml::raw, = serial_xml::optional, = serial_xml::iter{"item"} ]] std::vector<int>
          values{9};
    };
    auto absent = serial_xml::from_xml<OptionalRawInput>("<OptionalRawInput/>");
    expect_equal("default", absent.text);
    expect_equal((std::vector<int>{9}), absent.values);
  }

  [[= test{}]] void string_input_with_literal_root_override() {
    struct StringRootOverrideInput {
      int value;
    };
    std::string xml = "<record><value>42</value></record>";
    const std::string const_xml = xml;
    expect_equal(42, serial_xml::from_xml<StringRootOverrideInput>(xml, "record").value);
    expect_equal(42, serial_xml::from_xml<StringRootOverrideInput>(const_xml, "record").value);
    std::string_view view = xml;
    expect_equal(42, serial_xml::from_xml<StringRootOverrideInput>(view, "record").value);
    expect_equal(const_xml, xml);
  }

  [[= test{}]] void borrowed_parser_text_produces_owning_results() {
    struct BorrowedTextInput {
      std::string text;
      std::vector<std::string> values;
    };
    std::string xml =
        "<BorrowedTextInput><text>long text &amp; decoded text owned by result</text>"
        "<values><element>first &lt; value</element><element>second value</element>"
        "</values></BorrowedTextInput>";
    auto result = serial_xml::from_xml<BorrowedTextInput>(xml);
    xml.assign(xml.size(), 'x');
    expect_equal("long text & decoded text owned by result", result.text);
    expect_equal((std::vector<std::string>{"first < value", "second value"}), result.values);
  }

  [[= test{}]] void parsed_strings_outlive_destroyed_source_storage() {
    struct OwnedInput {
      [[= serial_xml::attribute]] std::string label;
      std::vector<std::string> values;
    };
    auto result = [] {
      const auto xml =
          std::string("<OwnedInput label='a&amp;b'><values>") +
          "<element>first &lt; value</element><element><![CDATA[second & value]]></element>"
          "</values></OwnedInput>";
      return serial_xml::from_xml<OwnedInput>(xml);
    }();
    const std::vector<std::string> reused_storage(32, std::string(256, 'x'));
    expect_equal("a&b", result.label);
    expect_equal(std::vector<std::string>{"first < value", "second & value"}, result.values);
  }

  [[= test{}]] void arena_overflow_preserves_decoded_values() {
    struct ArenaOverflowInput {
      std::vector<std::string> values;
    };
    ArenaOverflowInput original;
    for (int i = 0; i < 600; ++i)
      original.values.push_back(std::to_string(i) + " long & escaped <text>");
    auto restored = serial_xml::from_xml<ArenaOverflowInput>(serial_xml::to_xml(original));
    expect_equal(original.values, restored.values);
  }

  [[= test{}]] void in_place_input_may_alias_an_updated_field() {
    struct AliasedInputRecord {
      std::string source;
      int value;
    };
    AliasedInputRecord record{
        "<AliasedInputRecord><source>replacement</source><value>42</value></AliasedInputRecord>",
        0};
    serial_xml::from_xml(record, record.source);
    expect_equal("replacement", record.source);
    expect_equal(42, record.value);
  }

  [[= test{}]] void mixed_text_normalization_and_arena_lifetimes() {
    struct MixedArenaInput {
      [[= serial_xml::attribute]] std::string attribute;
      std::string text;
    };
    auto result = serial_xml::from_xml<MixedArenaInput>(
        "<MixedArenaInput attribute='é&#x1F600;\t&amp;&#13;'>"
        "<text>é&#x1F600;\r\n<![CDATA[<&>]]><!--gap-->tail</text></MixedArenaInput>");
    expect_equal("é😀 &\r", result.attribute);
    expect_equal("é😀\n<&>tail", result.text);
  }

  [[= test{}]] void ascii_fast_path_validates_every_byte_and_word_boundary() {
    struct AsciiBoundaryInput {
      std::string value;
    };
    for (std::size_t position : {0u, 7u, 8u, 15u, 16u, 31u, 32u, 63u, 64u}) {
      for (unsigned byte = 0; byte <= 255; ++byte) {
        std::string text(96, 'a');
        text[position] = static_cast<char>(byte);
        auto xml = "<AsciiBoundaryInput><value>" + text + "</value></AsciiBoundaryInput>";
        bool valid = byte == 9 || byte == 10 || byte == 13 || (byte >= 32 && byte < 128);
        valid = valid && byte != '<' && byte != '&';
        if (valid) {
          if (byte == 13) text[position] = '\n';
          expect_equal(text, serial_xml::from_xml<AsciiBoundaryInput>(xml).value,
                       std::format("{}{}{}", byte, ":", position));
        } else {
          expect_throws<serial_xml::deserialization_error>(
              [&] { (void)(serial_xml::from_xml<AsciiBoundaryInput>(xml)); },
              std::format("{}{}{}", byte, ":", position));
        }
      }
    }
  }

  [[= test{}]] void independent_threads_keep_escape_state_separate() {
    struct ThreadInput {
      [[= serial_xml::attribute]] std::string attribute;
      std::string text;
    };
    std::array<std::exception_ptr, 8> exceptions{};
    std::array<bool, 8> matches{};
    {
      std::vector<std::jthread> workers;
      for (std::size_t worker = 0; worker < matches.size(); ++worker) {
        workers.emplace_back([&, worker] {
          try {
            matches[worker] = true;
            for (unsigned iteration = 0; iteration < 128; ++iteration) {
              const auto value = std::string(worker * 17 + iteration, 'a') + "<&\"'é😀\r\n";
              const auto xml = clean_to_xml(ThreadInput{value, value});
              const auto parsed = serial_xml::from_xml<ThreadInput>(xml);
              if (parsed.attribute != value || parsed.text != value ||
                  clean_to_xml(parsed) != xml) {
                matches[worker] = false;
              }
            }
          } catch (...) {
            exceptions[worker] = std::current_exception();
          }
        });
      }
    }
    for (std::size_t worker = 0; worker < matches.size(); ++worker) {
      scoped_trace trace(std::format("worker {}", worker));
      assert_no_throw([&] {
        if (exceptions[worker]) std::rethrow_exception(exceptions[worker]);
      });
      expect_true(matches[worker]);
    }
  }

  [[= test{}]] void seeded_unicode_round_trip_fuzzing() {
    struct FuzzRecord {
      [[= serial_xml::attribute]] std::string attribute;
      std::string text;
      [[= serial_xml::cdata]] std::string cdata;
    };
    std::mt19937 random(0x5EED1234);
    const std::array<std::string_view, 14> fragments{"a", " ",  "\t", "\n",  "\r", "&", "<",
                                                     ">", "\"", "'",  "]]>", "é",  "€", "😀"};
    for (unsigned iteration = 0; iteration < 1000; ++iteration) {
      scoped_trace trace(std::format("seed 0x5EED1234, iteration {}", iteration));
      auto generate = [&] {
        std::string value;
        const auto count = random() % 80;
        for (unsigned i = 0; i < count; ++i) value += fragments[random() % fragments.size()];
        return value;
      };
      const FuzzRecord original{generate(), generate(), generate()};
      const auto xml = clean_to_xml(original);
      const auto parsed = serial_xml::from_xml<FuzzRecord>(xml);
      expect_equal(original.attribute, parsed.attribute);
      expect_equal(original.text, parsed.text);
      expect_equal(original.cdata, parsed.cdata);
      expect_equal(xml, clean_to_xml(parsed));
    }
  }

  [[= test{}]] void seeded_malformed_input_fuzzing() {
    struct FuzzInput {
      std::string value;
    };
    std::mt19937 random(0xBAD5EED);
    const std::string valid = "<FuzzInput><value>é &amp; 😀</value></FuzzInput>";
    unsigned accepted = 0;
    unsigned rejected = 0;
    for (unsigned iteration = 0; iteration < 4000; ++iteration) {
      std::string input;
      if (iteration % 2 == 0) {
        input = valid;
        for (unsigned edits = 1 + random() % 4; edits > 0; --edits) {
          const auto index = random() % (input.size() + 1);
          switch (random() % 3) {
            case 0:
              input.insert(index, 1, static_cast<char>(random() & 255));
              break;
            case 1:
              if (index < input.size()) input.erase(index, 1);
              break;
            default:
              if (index < input.size()) input[index] = static_cast<char>(random() & 255);
              break;
          }
        }
      } else {
        input.resize(random() % 256);
        for (auto& byte : input) byte = static_cast<char>(random() & 255);
      }
      scoped_trace trace(std::format("seed 0xBAD5EED, iteration {}, input {:?}", iteration, input));
      std::optional<FuzzInput> parsed;
      try {
        parsed = serial_xml::from_xml<FuzzInput>(input);
      } catch (const serial_xml::deserialization_error&) {
        ++rejected;
        continue;
      }
      ++accepted;
      // Once accepted, failures in canonicalization or reparsing must fail this
      // test rather than being counted as ordinary malformed-input rejection.
      const auto canonical = clean_to_xml(*parsed);
      const auto reparsed = serial_xml::from_xml<FuzzInput>(canonical);
      expect_equal(parsed->value, reparsed.value);
      expect_equal(canonical, clean_to_xml(reparsed));
    }
    expect_true(accepted > 0);
    expect_true(rejected > 0);
    expect_equal(4000u, accepted + rejected);
  }

  [[= test{}]] void malformed_late_text_does_not_update_in_place_target() {
    struct ArenaAtomicParseInput {
      std::string first;
      std::string second;
    };
    ArenaAtomicParseInput original{"original first", "original second"};
    expect_throws<serial_xml::deserialization_error>([&] {
      (void)(serial_xml::from_xml(original,
                                  "<ArenaAtomicParseInput><first>new first</first><second>invalid "
                                  "&bad;</second></ArenaAtomicParseInput>"));
    });
    expect_equal("original first", original.first);
    expect_equal("original second", original.second);
  }
};

struct PrettifyTests {
  [[= test{}]] void preserves_text_and_mixed_content() {
    for (const std::string xml :
         {"<root>hello</root>", "<root>  hello\n </root>", "<root>a<child/>b</root>",
          "<root><![CDATA[a<b>]]></root>", "<root xml:space='preserve'><child/></root>"})
      expect_equal(xml, serial_xml::prettify(xml));
    expect_equal("<root>\n  <text>hello</text>\n  <empty/>\n</root>",
                 serial_xml::prettify("<root><text>hello</text><empty/></root>"));
    struct PrettyTextRecord {
      std::string text;
    };
    const auto xml = serial_xml::prettify(clean_to_xml(PrettyTextRecord{"hello"}));
    expect_equal("hello", serial_xml::from_xml<PrettyTextRecord>(xml).text);
    expect_equal(xml, serial_xml::prettify(xml));
  }

  [[= test{}]] void handles_quoted_markup_comments_and_instructions() {
    expect_equal(
        "<?xml version='1.0'?>\n<root>\n  <!-- a > b -->\n  <?task a > b?>\n"
        "  <child attr='>'>hello</child>\n  <other><![CDATA[x>y]]></other>\n</root>",
        serial_xml::prettify("<?xml version='1.0'?><root><!-- a > b --><?task a > b?>"
                             "<child attr='>'>hello</child><other><![CDATA[x>y]]></other></root>"));
    expect_throws<serial_xml::deserialization_error>(
        [&] { (void)(serial_xml::prettify("<root><broken></root>")); });
  }

  [[= test{}]] void preserves_nested_space_and_is_idempotent() {
    const std::string xml =
        "<root><keep xml:space='preserve'><a/>  <b/></keep><other><c/></other></root>";
    const std::string expected =
        "<root>\n  <keep xml:space='preserve'><a/>  <b/></keep>\n  <other>\n    <c/>\n  "
        "</other>\n</root>";
    expect_equal(expected, serial_xml::prettify(xml));
    expect_equal(expected, serial_xml::prettify(expected));
    expect_equal(std::string("<?task?>\n<root/>\n<!--end-->"),
                 serial_xml::prettify("<?task?><root/><!--end-->"));
  }

  [[= test{}]] void preserves_utf8_bom_before_declaration() {
    const std::string xml = "\xEF\xBB\xBF<?xml version='1.0'?><root><text>hello</text></root>";
    expect_equal("\xEF\xBB\xBF<?xml version='1.0'?>\n<root>\n  <text>hello</text>\n</root>",
                 serial_xml::prettify(xml));
    expect_equal("\xEF\xBB\xBF<root>hello</root>",
                 serial_xml::prettify("\xEF\xBB\xBF<root>hello</root>"));
  }
};

int main(int argc, char** argv) {
  return run_tests<NamingTests, AttributesTests, STLTests, ChildrenTests, NestingTests,
                   IterationTests, BasicTests, UnpackingTests, EscapingTests, FormattingTests,
                   FunctionsTests, MockingTests, FromStringTests, FromXmlTests, PrettifyTests>(
      argc, argv);
}
