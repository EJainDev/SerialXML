#include <gtest/gtest.h>

import std;

import serial_xml;

std::string clean_to_xml(auto obj) { return serial_xml::to_xml(obj, false); }

TEST(Naming, EmptyWithName) {
  struct[[= serial_xml::name{"MyStruct"}]] EmptyName {};
  EmptyName obj;

  ASSERT_EQ(clean_to_xml(obj), "<MyStruct/>");
}

TEST(Naming, NamedAttribute) {
  struct NamedAttribute {
    [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute ]] int x;
  };
  NamedAttribute obj{4};

  ASSERT_EQ(clean_to_xml(obj), "<NamedAttribute MyAttribute=\"4\"/>");
}

TEST(Naming, NamedChild) {
  struct NamedChild {
    [[= serial_xml::name{"MyChild"}]] int x;
  };
  NamedChild obj{4};

  ASSERT_EQ(clean_to_xml(obj), "<NamedChild><MyChild>4</MyChild></NamedChild>");
}

TEST(Naming, NamedAttributeAndChild) {
  struct NamedAttributeAndChild {
    [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute ]] int x;
    [[= serial_xml::name{"MyChild"}]] int y;
  };
  NamedAttributeAndChild obj{4, 5};

  ASSERT_EQ(clean_to_xml(obj),
            "<NamedAttributeAndChild MyAttribute=\"4\"><MyChild>5</MyChild></"
            "NamedAttributeAndChild>");
}

TEST(Naming, AllNamed) {
  struct[[= serial_xml::name{"MyStruct"}]] AllNamed {
    [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute ]] int x;
    [[= serial_xml::name{"MyChild"}]] int y;
  };
  AllNamed obj{4, 5};

  ASSERT_EQ(clean_to_xml(obj), "<MyStruct MyAttribute=\"4\"><MyChild>5</MyChild></MyStruct>");
}

TEST(Naming, FixedName) {
  struct FixedName {};

  ASSERT_EQ(serial_xml::to_xml(FixedName{}, false, "FixedName"), "<FixedName/>");
}

TEST(Naming, SkipNamedAttribute) {
  struct SkipNamedAttribute {
    [[ = serial_xml::name{"MyAttribute"}, = serial_xml::attribute, = serial_xml::skip ]] int x;
  };
  SkipNamedAttribute obj{4};

  ASSERT_EQ(clean_to_xml(obj), "<SkipNamedAttribute/>");
}

TEST(Naming, SkipNamedChild) {
  struct SkipNamedChild {
    [[ = serial_xml::name{"MyChild"}, = serial_xml::skip ]] int x;
  };
  SkipNamedChild obj{4};

  ASSERT_EQ(clean_to_xml(obj), "<SkipNamedChild/>");
}

TEST(Naming, NamedStructChild) {
  struct NamedStructChildInner {
    [[= serial_xml::name{"MyChild"}]] int x;
  };

  struct NamedStructChildOuter {
    NamedStructChildInner inner;
  };
  NamedStructChildOuter obj{{4}};

  ASSERT_EQ(clean_to_xml(obj),
            "<NamedStructChildOuter><inner><MyChild>4</MyChild></inner></"
            "NamedStructChildOuter>");
}

TEST(Attributes, Single) {
  struct SingleAttribute {
    [[= serial_xml::attribute]] int x;
  };
  SingleAttribute obj{4};

  ASSERT_EQ(clean_to_xml(obj), "<SingleAttribute x=\"4\"/>");
}

TEST(Attributes, Multiple) {
  struct MultipleAttributes {
    [[= serial_xml::attribute]] int x;
    [[= serial_xml::attribute]] int y;
  };
  MultipleAttributes obj{4, 5};

  ASSERT_EQ(clean_to_xml(obj), "<MultipleAttributes x=\"4\" y=\"5\"/>");
}

TEST(Attributes, Skip) {
  struct SkipAttribute {
    [[ = serial_xml::attribute, = serial_xml::skip ]] int x;
  };
  SkipAttribute obj{4};

  ASSERT_EQ(clean_to_xml(obj), "<SkipAttribute/>");
}

TEST(Attributes, AttributeAndChild) {
  struct AttributeAndChild {
    [[= serial_xml::attribute]] int x;
    int y;
  };
  AttributeAndChild obj{4, 5};

  ASSERT_EQ(clean_to_xml(obj), "<AttributeAndChild x=\"4\"><y>5</y></AttributeAndChild>");
}

TEST(Attributes, AttributeAndSkipChild) {
  struct AttributeAndSkipChild {
    [[= serial_xml::attribute]] int x;
    [[= serial_xml::skip]] int y;
  };
  AttributeAndSkipChild obj{4, 5};

  ASSERT_EQ(clean_to_xml(obj), "<AttributeAndSkipChild x=\"4\"/>");
}

TEST(Attributes, NestedAttribute) {
  struct NestedAttributeInner {
    [[= serial_xml::attribute]] int x;
  };

  struct NestedAttributeOuter {
    NestedAttributeInner inner;
  };
  NestedAttributeOuter obj{{4}};

  ASSERT_EQ(clean_to_xml(obj), "<NestedAttributeOuter><inner x=\"4\"/></NestedAttributeOuter>");
}

TEST(STL, Vector) {
  struct Vector {
    std::vector<int> values;
  };
  Vector obj{{1, 2, 3}};
  ASSERT_EQ(clean_to_xml(obj),
            "<Vector><values><element>1</element><element>2</"
            "element><element>3</element>"
            "</values></Vector>");
}

TEST(STL, Optional) {
  struct Optional {
    std::optional<int> value;
  };
  Optional obj{{42}};
  ASSERT_EQ(clean_to_xml(obj), "<Optional><value>42</value></Optional>");

  Optional obj2{std::nullopt};
  ASSERT_EQ(clean_to_xml(obj2), "<Optional></Optional>");
}

TEST(STL, NoIter) {
  struct NoIter {
    [[= serial_xml::no_iter]] std::vector<int> values;
  };
  NoIter obj{{1, 2, 3}};
  ASSERT_EQ(clean_to_xml(obj), "<NoIter><values>[1, 2, 3]</values></NoIter>");
}

TEST(STL, NestedVector) {
  struct NestedVector {
    std::vector<std::vector<int>> values;
  };
  NestedVector obj{{{1, 2}, {3, 4}}};
  ASSERT_EQ(clean_to_xml(obj),
            "<NestedVector><values><element>[1, 2]</element><element>[3, "
            "4]</element></values></NestedVector>");
}

TEST(STL, RawIter) {
  struct STLRawIter {
    [[= serial_xml::raw]] std::vector<int> values;
  };

  STLRawIter obj{{1, 2, 3}};
  ASSERT_EQ(
      clean_to_xml(obj),
      "<STLRawIter><element>1</element><element>2</element><element>3</element></STLRawIter>");
}

TEST(STL, OptionalAttribute) {
  struct OptionalAttribute {
    [[= serial_xml::attribute]] std::optional<int> value;
  };
  OptionalAttribute obj{{42}};
  ASSERT_EQ(clean_to_xml(obj), "<OptionalAttribute value=\"42\"/>");

  OptionalAttribute obj2{std::nullopt};
  ASSERT_EQ(clean_to_xml(obj2), "<OptionalAttribute/>");
}

TEST(Children, Single) {
  struct SingleChild {
    int x;
  };
  SingleChild obj{42};
  ASSERT_EQ(clean_to_xml(obj), "<SingleChild><x>42</x></SingleChild>");
}

TEST(Children, Multiple) {
  struct MultipleChildren {
    int x;
    int y;
  };
  MultipleChildren obj{42, 100};
  ASSERT_EQ(clean_to_xml(obj), "<MultipleChildren><x>42</x><y>100</y></MultipleChildren>");
}

TEST(Children, Skip) {
  struct SkipChild {
    [[= serial_xml::skip]] int x;
  };
  SkipChild obj{42};
  ASSERT_EQ(clean_to_xml(obj), "<SkipChild/>");
}

TEST(Children, ChildAndSkip) {
  struct ChildAndSkip {
    int x;
    [[= serial_xml::skip]] int y;
  };
  ChildAndSkip obj{42, 100};
  ASSERT_EQ(clean_to_xml(obj), "<ChildAndSkip><x>42</x></ChildAndSkip>");
}

TEST(Children, Nested) {
  struct NestedChildInner {
    int x;
  };
  struct NestedChildOuter {
    NestedChildInner inner;
  };
  NestedChildOuter obj{{42}};
  ASSERT_EQ(clean_to_xml(obj), "<NestedChildOuter><inner><x>42</x></inner></NestedChildOuter>");
}

TEST(Nesting, Children) {
  struct NestedChildrenInner {
    int x;
    int y;
  };
  struct NestedChildrenOuter {
    NestedChildrenInner inner;
  };
  NestedChildrenOuter obj{{42, 100}};
  ASSERT_EQ(clean_to_xml(obj),
            "<NestedChildrenOuter><inner><x>42</x><y>100</y></inner></NestedChildrenOuter>");
}

TEST(Nesting, Attributes) {
  struct NestedAttributesInner {
    [[= serial_xml::attribute]] int x;
    [[= serial_xml::attribute]] int y;
  };
  struct NestedAttributesOuter {
    NestedAttributesInner inner;
  };
  NestedAttributesOuter obj{{42, 100}};
  ASSERT_EQ(clean_to_xml(obj),
            "<NestedAttributesOuter><inner x=\"42\" y=\"100\"/></NestedAttributesOuter>");
}

TEST(Nesting, AttributesAndChildren) {
  struct NestedAttributesAndChildrenInner {
    [[= serial_xml::attribute]] int x;
    int y;
  };
  struct NestedAttributesAndChildrenOuter {
    NestedAttributesAndChildrenInner inner;
  };
  NestedAttributesAndChildrenOuter obj{{42, 100}};
  ASSERT_EQ(clean_to_xml(obj),
            "<NestedAttributesAndChildrenOuter><inner "
            "x=\"42\"><y>100</y></inner></NestedAttributesAndChildrenOuter>");
}

TEST(Nesting, SkipInner) {
  struct SkipInner {
    [[= serial_xml::skip]] int x;
  };
  struct SkipOuter {
    SkipInner inner;
  };
  SkipOuter obj{{42}};
  ASSERT_EQ(clean_to_xml(obj), "<SkipOuter><inner/></SkipOuter>");
}

TEST(Nesting, SkipNesting) {
  struct SkipNestingInner {
    int x;
  };
  struct SkipNestingOuter {
    [[= serial_xml::skip]] SkipNestingInner inner;
  };
  SkipNestingOuter obj{{42}};
  ASSERT_EQ(clean_to_xml(obj), "<SkipNestingOuter/>");
}

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

TEST(Iteration, CustomList) {
  struct Iter {
    [[= serial_xml::iter{"value", "values"}]] CustomList<int> values;
  };

  Iter obj{{1, 2, 3}};
  ASSERT_EQ(clean_to_xml(obj),
            "<Iter><values><value>1</value><value>2</value><value>3</"
            "value></values></Iter>");
}

TEST(Iteration, IterSTL) {
  struct IterSTL {
    [[= serial_xml::iter{"c_val", "container"}]] std::vector<int> values;
  };

  IterSTL obj{{1, 2, 3}};
  ASSERT_EQ(clean_to_xml(obj),
            "<IterSTL><container><c_val>1</c_val><c_val>2</c_val><c_val>3</"
            "c_val></container></IterSTL>");
}

TEST(Iteration, SingleCharIter) {
  struct SingleCharIter {
    [[= serial_xml::iter{"v", "vals"}]] std::vector<int> values;
  };

  SingleCharIter obj{{1, 2, 3}};
  ASSERT_EQ(clean_to_xml(obj),
            "<SingleCharIter><vals><v>1</v><v>2</v><v>3</v></vals></SingleCharIter>");
}

TEST(Iteration, StructInRange) {
  struct StructInRangeInner {
    int x;
  };
  struct StructInRangeOuter {
    [[= serial_xml::iter{"inner", "inners"}]] std::vector<StructInRangeInner> inners;
  };

  StructInRangeOuter obj{{{1}, {2}, {3}}};
  ASSERT_EQ(clean_to_xml(obj),
            "<StructInRangeOuter><inners><inner><x>1</x></inner><inner><x>2</x></"
            "inner><inner><x>3</x></inner></"
            "inners></StructInRangeOuter>");
}

TEST(Iteration, RawIter) {
  struct RawIter {
    [[ = serial_xml::raw, = serial_xml::iter{"value", "values"} ]] std::vector<int> values;
  };

  RawIter obj{{1, 2, 3}};
  ASSERT_EQ(clean_to_xml(obj),
            "<RawIter><value>1</value><value>2</value><value>3</value></RawIter>");
}

TEST(Basic, EmptyStruct) {
  struct EmptyStruct {};
  EmptyStruct obj;

  std::string xml = serial_xml::to_xml(obj);
  ASSERT_EQ(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><EmptyStruct/>");
}

TEST(Basic, AttributesAndChildren) {
  struct AttributesAndChildren {
    [[= serial_xml::attribute]] int x;
    int y;
  };
  AttributesAndChildren obj{42, 100};
  ASSERT_EQ(clean_to_xml(obj),
            "<AttributesAndChildren x=\"42\"><y>100</y></AttributesAndChildren>");
}

TEST(Basic, Raw) {
  struct Raw {
    [[= serial_xml::raw]] std::string text;
  };

  Raw obj{"text"};
  ASSERT_EQ(clean_to_xml(obj), "<Raw>text</Raw>");
}

TEST(Basic, CData) {
  struct CData {
    [[= serial_xml::cdata]] std::string text;
  };

  CData obj{"text<empty> & stuff"};
  ASSERT_EQ(clean_to_xml(obj), "<CData><![CDATA[text<empty> & stuff]]></CData>");
}

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

TEST(Unpacking, NoUnpack) {
  struct NoUnpackOuter {
    [[= serial_xml::no_unpack]] NoUnpackInner inner;
  };
  NoUnpackOuter obj{{42}};
  ASSERT_EQ(clean_to_xml(obj), "<NoUnpackOuter><inner>42</inner></NoUnpackOuter>");
}
TEST(Escaping, Child) {
  struct EscapeChild {
    std::string text;
  };

  EscapeChild obj{"<>&'\""};
  ASSERT_EQ(clean_to_xml(obj), "<EscapeChild><text>&lt;&gt;&amp;&apos;&quot;</text></EscapeChild>");
}

TEST(Escaping, ComplexChild) {
  struct EscapeComplexChild {
    std::string text;
    int number;
  };

  EscapeComplexChild obj{"Hi! <> My name is & Bob. ' And \" This", 42};
  ASSERT_EQ(clean_to_xml(obj),
            "<EscapeComplexChild><text>Hi! &lt;&gt; My name "
            "is &amp; Bob. &apos; And &quot; This</"
            "text><number>42</number></EscapeComplexChild>");
}

TEST(Escaping, Attribute) {
  struct EscapeAttribute {
    [[= serial_xml::attribute]] std::string text;
  };

  EscapeAttribute obj{"<>&'\""};
  ASSERT_EQ(clean_to_xml(obj), "<EscapeAttribute text=\"&lt;&gt;&amp;&apos;&quot;\"/>");
}

TEST(Escaping, Raw) {
  struct EscapeRaw {
    [[= serial_xml::raw]] std::string text;
  };

  EscapeRaw obj{"<>&'\""};
  ASSERT_EQ(clean_to_xml(obj), "<EscapeRaw>&lt;&gt;&amp;&apos;&quot;</EscapeRaw>");
}

TEST(Formatting, Attribute) {
  struct FormattedAttribute {
    [[ = serial_xml::attribute, = serial_xml::format{"03d"} ]] int x;
  };
  FormattedAttribute obj{42};

  ASSERT_EQ(clean_to_xml(obj), "<FormattedAttribute x=\"042\"/>");
}

TEST(Formatting, Child) {
  struct FormattedChild {
    [[= serial_xml::format{"03d"}]] int x;
  };
  FormattedChild obj{42};

  ASSERT_EQ(clean_to_xml(obj), "<FormattedChild><x>042</x></FormattedChild>");
}

TEST(Formatting, Raw) {
  struct FormattedRaw {
    [[ = serial_xml::raw, = serial_xml::format{"03d"} ]] int x;
  };
  FormattedRaw obj{42};

  ASSERT_EQ(clean_to_xml(obj), "<FormattedRaw>042</FormattedRaw>");
}

TEST(Formatting, StringChild) {
  struct FormattedStringChild {
    [[= serial_xml::format{"*^12"}]] std::string text;
  };
  FormattedStringChild obj{"text"};

  ASSERT_EQ(clean_to_xml(obj),
            "<FormattedStringChild><text>****text****</text></FormattedStringChild>");
}