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

TEST(STL, EmptyVector) {
  struct EmptyVector {
    std::vector<int> values;
  };
  EmptyVector obj{{}};
  ASSERT_EQ(clean_to_xml(obj), "<EmptyVector><values/></EmptyVector>");
}

TEST(STL, ExcludeOnEmpty) {
  struct ExcludeOnEmpty {
    [[= serial_xml::exclude_on_empty]] std::vector<int> values;
  };
  ExcludeOnEmpty obj{{}};
  ASSERT_EQ(clean_to_xml(obj), "<ExcludeOnEmpty/>");
}

TEST(STL, Optional) {
  struct Optional {
    std::optional<int> value;
  };
  Optional obj{{42}};
  ASSERT_EQ(clean_to_xml(obj), "<Optional><value>42</value></Optional>");

  Optional obj2{std::nullopt};
  ASSERT_EQ(clean_to_xml(obj2), "<Optional/>");
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

std::string format_with_prefix(const auto& value) { return "custom-" + std::to_string(value); }

TEST(Formatting, CustomFunction) {
  struct CustomFormattedValues {
    [[ = serial_xml::attribute, = serial_xml::format{format_with_prefix<int>} ]] int attribute;
    [[= serial_xml::format{format_with_prefix<int>}]] int child;
    [[ = serial_xml::raw, = serial_xml::format{format_with_prefix<int>} ]] int raw;
  };
  CustomFormattedValues obj{1, 2, 3};

  ASSERT_EQ(clean_to_xml(obj),
            "<CustomFormattedValues attribute=\"custom-1\"><child>custom-2</child>custom-3"
            "</CustomFormattedValues>");
}

std::string surround_string(const std::string& value) { return "[" + value + "]"; }

TEST(Formatting, CustomFunctionBypassesStringFastPath) {
  struct CustomFormattedString {
    [[= serial_xml::format{surround_string}]] std::string text;
  };
  CustomFormattedString obj{"text"};

  ASSERT_EQ(clean_to_xml(obj),
            "<CustomFormattedString><text>[text]</text></CustomFormattedString>");
}

struct CustomFormatInner {
  int ignored;
};

std::string format_inner(const CustomFormatInner&) { return "formatted"; }

TEST(Formatting, CustomFunctionPreventsUnpacking) {
  struct CustomFormattedOuter {
    [[= serial_xml::format{format_inner}]] CustomFormatInner inner;
  };
  CustomFormattedOuter obj{{42}};

  ASSERT_EQ(clean_to_xml(obj),
            "<CustomFormattedOuter><inner>formatted</inner></CustomFormattedOuter>");
}

TEST(Functions, Basic) {
  struct BasicFunction {
    int x() const { return 3; }
  };
  BasicFunction obj;

  ASSERT_EQ(clean_to_xml(obj), "<BasicFunction><x>3</x></BasicFunction>");
}

TEST(Functions, RequiresConst) {
  struct ConstFunction {
    int x() { return 3; }
  };
  ConstFunction obj;

  ASSERT_EQ(clean_to_xml(obj), "<ConstFunction/>");
}

TEST(Functions, NoStaticFunc) {
  struct NoStaticFunction {
    static int x() { return 3; }
  };
  NoStaticFunction obj;

  ASSERT_EQ(clean_to_xml(obj), "<NoStaticFunction/>");
}

TEST(Functions, IgnoreParameterized) {
  struct IgnoreParameterizedFunction {
    int x(int a) const { return a; }
  };
  IgnoreParameterizedFunction obj;

  ASSERT_EQ(clean_to_xml(obj), "<IgnoreParameterizedFunction/>");
}

struct IgnoreTemplatedFunction {
  template <typename T>
  int x() const {
    return 3;
  }
};

TEST(Functions, IgnoreTemplated) {
  IgnoreTemplatedFunction obj;

  ASSERT_EQ(clean_to_xml(obj), "<IgnoreTemplatedFunction/>");
}

class mock_vector {
 public:
  [[= serial_xml::attribute]] std::size_t size() const { return 0; }
};

TEST(Mocking, VectorGetterUsesActualObject) {
  const std::vector<int> values{10, 20, 30};
  ASSERT_EQ(serial_xml::to_xml<mock_vector>(values, false), "<vector size=\"3\"/>");
  ASSERT_EQ(serial_xml::to_xml<mock_vector>(std::vector<std::string>{}, false),
            "<vector size=\"0\"/>");
  ASSERT_EQ(serial_xml::to_xml<mock_vector>(values),
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?><vector size=\"3\"/>");
}

TEST(Mocking, FieldsUseMockAnnotationsAndOrder) {
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
  ASSERT_EQ(serial_xml::to_xml<MockFields>(MockFieldsTarget{4, 7, 99}, false),
            "<MockFieldsTarget "
            "second=\"7\"><first>4</first></MockFieldsTarget>");
}

TEST(Mocking, GetterSelectsConstCallableOverload) {
  struct MockOverloadsTarget {
    int amount() { return 100; }
    int amount() const { return 8; }
    int amount(int) const { return 200; }
  };
  struct MockOverloads {
    [[= serial_xml::attribute]] int amount() const;
  };
  ASSERT_EQ(serial_xml::to_xml<MockOverloads>(MockOverloadsTarget{}, false),
            "<MockOverloadsTarget amount=\"8\"/>");
}

TEST(Mocking, TargetReturnTypeControlsSerialization) {
  struct MockRangeTarget {
    std::vector<int> values() const { return {2, 3}; }
  };
  struct MockRange {
    // The placeholder body and type never provide the serialized value.
    int values() const;
  };
  ASSERT_EQ(serial_xml::to_xml<MockRange>(MockRangeTarget{}, false),
            "<MockRangeTarget><values><element>2</element><element>3</"
            "element></values></MockRangeTarget>");
}

TEST(Mocking, FormatsAndNamesComeFromMock) {
  struct MockFormatTarget {
    [[= serial_xml::skip]] int value;
  };
  struct[[= serial_xml::name{"record"}]] MockFormat {
    MockFormat() = delete;
    [[ = serial_xml::format{"04"}, = serial_xml::name{"number"} ]] int value;
  };
  ASSERT_EQ(serial_xml::to_xml<MockFormat>(MockFormatTarget{12}, false),
            "<record><number>0012</number></record>");
  ASSERT_EQ(serial_xml::to_xml<MockFormat>(MockFormatTarget{12}, false, "override"),
            "<override><number>0012</number></override>");
}

TEST(Mocking, NestedValuesAndExplicitIteration) {
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
  ASSERT_EQ(serial_xml::to_xml<MockNested>(MockNestedTarget{{5}, {{6}, {7}}}, false),
            "<MockNestedTarget><child><value>5</value></"
            "child><list><item><value>6</value></item>"
            "<item><value>7</value></item></list></MockNestedTarget>");
}

TEST(Mocking, EmptySchemaDoesNotSerializeTargetMembers) {
  struct MockEmptyTarget {
    int value;
  };
  struct Empty {};
  ASSERT_EQ(serial_xml::to_xml<Empty>(MockEmptyTarget{5}, false), "<MockEmptyTarget/>");
  ASSERT_EQ(serial_xml::to_xml<MockEmptyTarget>(MockEmptyTarget{5}, false),
            "<MockEmptyTarget><value>5</value></"
            "MockEmptyTarget>");
}

TEST(Mocking, GetterWithDefaultArgument) {
  struct MockDefaultTarget {
    int value(int offset = 2) const { return 5 + offset; }
  };
  struct MockDefault {
    int value() const;
  };
  ASSERT_EQ(serial_xml::to_xml<MockDefault>(MockDefaultTarget{}, false),
            "<MockDefaultTarget><value>7</value></MockDefaultTarget>");
}

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

TEST(FromString, PrimitivesAndStrictValidation) {
  EXPECT_EQ(serial_xml::from_string<int>(" -42 "), -42);
  EXPECT_EQ(serial_xml::from_string<unsigned>("42"), 42u);
  EXPECT_DOUBLE_EQ(serial_xml::from_string<double>("1.25e2"), 125.0);
  EXPECT_TRUE(serial_xml::from_string<bool>("true"));
  EXPECT_TRUE(serial_xml::from_string<bool>("1"));
  EXPECT_FALSE(serial_xml::from_string<bool>("false"));
  EXPECT_FALSE(serial_xml::from_string<bool>("0"));
  EXPECT_EQ(serial_xml::from_string<char>("65"), 'A');
  EXPECT_EQ(serial_xml::from_string<std::string>("  hello  "), "  hello  ");
  for (auto bad : {"", "12tail", "9999999999999999999999", "--1"}) {
    EXPECT_THROW(serial_xml::from_string<int>(bad), serial_xml::deserialization_error);
  }
  EXPECT_THROW(serial_xml::from_string<unsigned>("-1"), serial_xml::deserialization_error);
  EXPECT_THROW(serial_xml::from_string<bool>("yes"), serial_xml::deserialization_error);
}

TEST(FromString, ContainersAndCustomTypes) {
  using Nested = std::vector<std::vector<int>>;
  EXPECT_EQ(serial_xml::from_string<Nested>("[[1, 2], [], [3]]"), (Nested{{1, 2}, {}, {3}}));
  using Strings = std::vector<std::string>;
  EXPECT_EQ(serial_xml::from_string<Strings>("[\"a,b\", \"[x]\", \"a\\\"b\", \"\\n\"]"),
            (Strings{"a,b", "[x]", "a\"b", "\n"}));
  using Array = std::array<int, 2>;
  EXPECT_EQ(serial_xml::from_string<Array>("[1, 2]"), (Array{1, 2}));
  EXPECT_THROW(serial_xml::from_string<Array>("[1]"), serial_xml::deserialization_error);
  EXPECT_EQ(serial_xml::from_string<std::deque<int>>("[1, 2]"), (std::deque<int>{1, 2}));
  EXPECT_EQ(serial_xml::from_string<std::list<int>>("[1, 2]"), (std::list<int>{1, 2}));
  EXPECT_EQ(serial_xml::from_string<std::forward_list<int>>("[1, 2]"),
            (std::forward_list<int>{1, 2}));
  EXPECT_EQ(serial_xml::from_string<std::set<int>>("{2, 1}"), (std::set<int>{1, 2}));
  using Map = std::map<std::string, std::vector<int>>;
  EXPECT_EQ(serial_xml::from_string<Map>("{\"a:b\": [1, 2], \"z\": []}"),
            (Map{{"a:b", {1, 2}}, {"z", {}}}));
  using Tuple = std::tuple<int, std::string, std::vector<int>>;
  EXPECT_EQ(serial_xml::from_string<Tuple>("(3, \"hi\", [4, 5])"), (Tuple{3, "hi", {4, 5}}));
  EXPECT_EQ(serial_xml::from_string<std::optional<int>>("7"), 7);
  auto values = serial_xml::from_string<std::valarray<int>>("[1, 2]");
  EXPECT_EQ(values.size(), 2u);
  EXPECT_EQ(values[1], 2);
  EXPECT_EQ(serial_xml::from_string<DeserializationLeaf>("17").value, 17);
  for (auto bad : {"[1,]", "[1,,2]", "[1", "[(1]]", "[\"oops]"}) {
    EXPECT_THROW(serial_xml::from_string<std::vector<int>>(bad), serial_xml::deserialization_error);
  }
}

TEST(FromXml, NamedAttributesChildrenNestedAndSkipped) {
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
  auto parsed = serial_xml::from_xml<NamedInputRecord>(xml);
  EXPECT_EQ(parsed.identifier, original.identifier);
  EXPECT_EQ(parsed.text, original.text);
  EXPECT_EQ(parsed.child.number, 7);
  EXPECT_EQ(parsed.skipped, 99);
  EXPECT_TRUE(parsed.enabled);
  EXPECT_DOUBLE_EQ(parsed.fraction, 1.5);
  EXPECT_EQ(serial_xml::to_xml(parsed), xml);
  EXPECT_THROW(serial_xml::from_xml<NamedInputRecord>("<wrong/>"),
               serial_xml::deserialization_error);
  EXPECT_EQ(
      serial_xml::from_xml<NamedInputRecord>(serial_xml::to_xml(original, false, "fixed"), "fixed")
          .identifier,
      12);
}

TEST(FromXml, RequiredAndOptionalMembers) {
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
  EXPECT_EQ(value.id, 2);
  EXPECT_EQ(value.count, 0);
  EXPECT_EQ(value.label, "default");
  EXPECT_FALSE(value.maybe.has_value());
  EXPECT_TRUE(value.values.empty());
  EXPECT_EQ(value.retained, 55);
  EXPECT_EQ(value.kept, (std::vector<int>{77}));
  EXPECT_THROW(serial_xml::from_xml<RequiredInputRecord>(
                   "<RequiredInputRecord><count>1</count></RequiredInputRecord>"),
               serial_xml::deserialization_error);
  EXPECT_THROW(serial_xml::from_xml<RequiredInputRecord>("<RequiredInputRecord id='1'/>"),
               serial_xml::deserialization_error);
  EXPECT_THROW(serial_xml::from_xml<RequiredInputRecord>(
                   "<RequiredInputRecord id='1'><count/></RequiredInputRecord>"),
               serial_xml::deserialization_error);
  struct OptionalAttributeInput {
    [[ = serial_xml::attribute, = serial_xml::optional ]] int value = 42;
  };
  EXPECT_EQ(serial_xml::from_xml<OptionalAttributeInput>("<OptionalAttributeInput/>").value, 42);
}

TEST(FromXml, OwningRangesAndIteration) {
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
  EXPECT_EQ(serial_xml::to_xml(result, false), xml);
  EXPECT_EQ(result.objects[1].value, 10);
  struct ExplicitForwardInput {
    [[= serial_xml::iter{"item"}]] std::forward_list<int> values;
  };
  ExplicitForwardInput forward{{1, 2}};
  EXPECT_EQ(serial_xml::to_xml(
                serial_xml::from_xml<ExplicitForwardInput>(serial_xml::to_xml(forward)), false),
            serial_xml::to_xml(forward, false));
  struct EmptyRangeInput {
    std::vector<int> values;
  };
  EXPECT_TRUE(serial_xml::from_xml<EmptyRangeInput>("<EmptyRangeInput><values/></EmptyRangeInput>")
                  .values.empty());
  EXPECT_THROW(serial_xml::from_xml<EmptyRangeInput>("<EmptyRangeInput/>"),
               serial_xml::deserialization_error);
  EXPECT_THROW(serial_xml::from_xml<EmptyRangeInput>(
                   "<EmptyRangeInput><values><bad>1</bad></values></EmptyRangeInput>"),
               serial_xml::deserialization_error);
  EXPECT_THROW(serial_xml::from_xml<EmptyRangeInput>(
                   "<EmptyRangeInput><values>1</values></EmptyRangeInput>"),
               serial_xml::deserialization_error);
}

TEST(FromXml, RawAndCdata) {
  struct RawTextInput {
    int number;
    [[= serial_xml::raw]] std::string text;
  };
  RawTextInput raw{3, "<&hello>"};
  EXPECT_EQ(serial_xml::from_xml<RawTextInput>(serial_xml::to_xml(raw)).text, raw.text);
  struct CdataInput {
    [[= serial_xml::cdata]] std::string first;
    [[= serial_xml::cdata]] std::string second;
  };
  CdataInput data{"hello<&>", ""};
  auto parsed = serial_xml::from_xml<CdataInput>(serial_xml::to_xml(data));
  EXPECT_EQ(parsed.first, data.first);
  EXPECT_EQ(parsed.second, data.second);
  struct RawRangeInput {
    [[ = serial_xml::raw, = serial_xml::iter{"item"} ]] std::vector<int> values;
  };
  RawRangeInput range{{1, 2}};
  EXPECT_EQ(serial_xml::from_xml<RawRangeInput>(serial_xml::to_xml(range)).values, range.values);
  EXPECT_TRUE(serial_xml::from_xml<RawRangeInput>("<RawRangeInput/>").values.empty());
}

TEST(FromXml, LeafConversionAndFormats) {
  struct LeafInputRecord {
    [[= serial_xml::format{format_deserialization_leaf}]] DeserializationLeaf leaf;
    [[= serial_xml::format{"04d"}]] int padded;
    [[= serial_xml::no_unpack]] DeserializationLeaf explicit_leaf;
  };
  auto result = serial_xml::from_xml<LeafInputRecord>(
      "<LeafInputRecord><leaf>7</leaf><padded>0012</padded><explicit_leaf>13</explicit_leaf></"
      "LeafInputRecord>");
  EXPECT_EQ(result.leaf.value, 7);
  EXPECT_EQ(result.padded, 12);
  EXPECT_EQ(result.explicit_leaf.value, 13);
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

TEST(FromXml, EncapsulationWithAutomaticSetterMatching) {
  SetterRecord original;
  original.value(7);
  original.set_title("test");
  original.setCount(9);
  auto result = serial_xml::from_xml<SetterRecord>(serial_xml::to_xml(original));
  EXPECT_EQ(result.value(), 7);
  EXPECT_EQ(result.get_title(), "test");
  EXPECT_EQ(result.getCount(), 9);
  EXPECT_EQ(result.read_only(), 77);
}

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

TEST(FromXml, IndependentAnnotatedSetters) {
  auto value = serial_xml::from_xml<IndependentSetters>(
      "<IndependentSetters id='3'><title>hello</title></IndependentSetters>");
  EXPECT_EQ(value.id(), 3);
  EXPECT_EQ(value.title(), "hello");
  EXPECT_EQ(value.count(), 5);
  EXPECT_THROW(serial_xml::from_xml<IndependentSetters>("<IndependentSetters id='3'/>"),
               serial_xml::deserialization_error);
}

TEST(FromXml, AnnotatedSetterOverridesGetterAndSupportsMock) {
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
  EXPECT_EQ(result.value(), 42);
  EXPECT_EQ(result.calls, 1);
}

TEST(FromXml, MockFieldsAndInPlaceNonDefaultConstruction) {
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
  EXPECT_EQ(result.value, 42);
  EXPECT_EQ(result.other, 99);
}

TEST(FromXml, EntitiesUnicodeCommentsAndCdataInScalar) {
  struct EntityTextInput {
    std::string value;
  };
  auto result = serial_xml::from_xml<EntityTextInput>(
      "\xEF\xBB\xBF<?xml version='1.0'?><!--before--><EntityTextInput><value>"
      "&lt;&gt;&amp;&quot;&apos;&#65;&#x1F600;<!--ignored--><![CDATA[<&>]]>"
      "</value></EntityTextInput><!--after-->");
  EXPECT_EQ(result.value, "<>&\"'A😀<&>");
  EXPECT_EQ(serial_xml::from_xml<EntityTextInput>(
                "<EntityTextInput><value>line\r\nbreak</value></EntityTextInput>")
                .value,
            "line\nbreak");
  struct EntityNumberInput {
    int value;
  };
  EXPECT_EQ(serial_xml::from_xml<EntityNumberInput>(
                "<EntityNumberInput>\n <value>\n 42\n </value>\n</EntityNumberInput>")
                .value,
            42);
}

TEST(FromXml, RejectsMalformedXmlAndDuplicateMembers) {
  struct MalformedInputRecord {
    int value;
  };
  for (auto xml :
       {"",
        "<MalformedInputRecord>",
        "<MalformedInputRecord><value>1</value></Wrong>",
        "<MalformedInputRecord><value>1</valueExtra></MalformedInputRecord>",
        "<MalformedInputRecord><value>1</val></MalformedInputRecord>",
        "<MalformedInputRecord><value>1</value",
        "<MalformedInputRecord><value>1</value></MalformedInputRecord>junk",
        "<MalformedInputRecord/><MalformedInputRecord/>",
        "<MalformedInputRecord a='1' a='2'><value>1</value></MalformedInputRecord>",
        "<MalformedInputRecord a='<'><value>1</value></MalformedInputRecord>",
        "<MalformedInputRecord><value>&unknown;</value></MalformedInputRecord>",
        "<MalformedInputRecord><value>&#xD800;</value></MalformedInputRecord>",
        "<MalformedInputRecord><value>&#0;</value></MalformedInputRecord>",
        "<MalformedInputRecord><value>1</value><value>2</value></MalformedInputRecord>",
        "<MalformedInputRecord><value><nested/></value></MalformedInputRecord>",
        "<MalformedInputRecord><value><![CDATA[broken</value></MalformedInputRecord>",
        "<!DOCTYPE "
        "MalformedInputRecord><MalformedInputRecord><value>1</value></MalformedInputRecord>",
        "<MalformedInputRecord><!--bad--comment--><value>1</value></MalformedInputRecord>",
        "<MalformedInputRecord><value>]]></value></MalformedInputRecord>",
        "<MalformedInputRecord a='1'b='2'><value>1</value></MalformedInputRecord>"}) {
    EXPECT_THROW(serial_xml::from_xml<MalformedInputRecord>(xml), serial_xml::deserialization_error)
        << xml;
  }
  struct MalformedTextInput {
    std::string value;
  };
  for (std::string bad :
       {std::string("\xC0\x80"), std::string("\xED\xA0\x80"), std::string("\x01", 1)}) {
    EXPECT_THROW(serial_xml::from_xml<MalformedTextInput>("<MalformedTextInput><value>" + bad +
                                                          "</value></MalformedTextInput>"),
                 serial_xml::deserialization_error);
  }
}

TEST(FromXml, OptionalValuesAndFormattedRangeItems) {
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
  ASSERT_TRUE(restored.child.has_value());
  EXPECT_EQ(restored.child->value, 9);
  EXPECT_EQ(restored.leaves, original.leaves);
  EXPECT_EQ(restored.number, 11);
  EXPECT_EQ(restored.text, "<&>");
  EXPECT_EQ(serial_xml::to_xml(restored), xml);
  auto empty = serial_xml::from_xml<OptionalInputRecord>(
      "<OptionalInputRecord><leaves/></OptionalInputRecord>");
  EXPECT_FALSE(empty.id);
  EXPECT_FALSE(empty.title);
  EXPECT_FALSE(empty.child);
  EXPECT_FALSE(empty.number);
  EXPECT_FALSE(empty.text);
  struct FormattedCdataNumberInput {
    [[ = serial_xml::cdata, = serial_xml::format{"04d"} ]] int value;
  };
  auto number_xml = serial_xml::to_xml(FormattedCdataNumberInput{16});
  EXPECT_EQ(serial_xml::from_xml<FormattedCdataNumberInput>(number_xml).value, 16);
}

TEST(FromXml, NestedAndRangeSetters) {
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
  EXPECT_EQ(value.child.value, 7);
  EXPECT_EQ(value.values, (std::vector<int>{8, 9}));
  EXPECT_EQ(serial_xml::to_xml(value, false), "<SetterInputRecord/>");
}

TEST(FromXml, ConstAnnotatedSetterAndReadonlyGetter) {
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
  EXPECT_EQ(input.value, 7);
  // Setter methods never participate in serialization, even when const and callable with no
  // arguments.
  EXPECT_EQ(serial_xml::to_xml(input, false), "<ConstSetterInput><calculated/></ConstSetterInput>");
}

TEST(FromXml, AutomaticForwardListAndContainerLimits) {
  struct ForwardListInput {
    std::forward_list<int> values;
  };
  ForwardListInput original{{1, 2, 3}};
  auto xml = serial_xml::to_xml(original);
  EXPECT_EQ(serial_xml::from_xml<ForwardListInput>(xml).values, original.values);
  struct LimitedInput {
    std::inplace_vector<int, 1> values;
  };
  EXPECT_THROW(
      serial_xml::from_xml<LimitedInput>(
          "<LimitedInput><values><element>1</element><element>2</element></values></LimitedInput>"),
      serial_xml::deserialization_error);
  struct ArrayInput {
    std::array<int, 2> values;
  };
  EXPECT_THROW(serial_xml::from_xml<ArrayInput>(
                   "<ArrayInput><values><element>1</element></values></ArrayInput>"),
               serial_xml::deserialization_error);
  EXPECT_THROW(serial_xml::from_xml<ArrayInput>("<ArrayInput><values/></ArrayInput>"),
               serial_xml::deserialization_error);
}

TEST(FromXml, DeclarationValidationAndDepthLimit) {
  struct DeclarationInput {};
  EXPECT_NO_THROW(serial_xml::from_xml<DeclarationInput>(
      "<?xml version='1.0' encoding='utf-8' standalone='yes'?><DeclarationInput/>"));
  for (auto declaration :
       {"<?xml?>", "<?XML version='1.0'?>", "<?xml version='1.1'?>", "<?xml garbage='1'?>",
        "<?xml version='1.0' version='1.0'?>", "<?xml version='1.0' encoding='UTF-16'?>",
        "<?xml version='1.0' standalone='maybe'?>"}) {
    EXPECT_THROW(
        serial_xml::from_xml<DeclarationInput>(std::string(declaration) + "<DeclarationInput/>"),
        serial_xml::deserialization_error);
  }
  std::string xml = "<DeclarationInput>";
  for (int i = 0; i < 256; ++i) xml += "<x>";
  for (int i = 0; i < 256; ++i) xml += "</x>";
  xml += "</DeclarationInput>";
  EXPECT_THROW(serial_xml::from_xml<DeclarationInput>(xml), serial_xml::deserialization_error);
}

TEST(FromXml, RawScalarAmbiguityAndAttributeNormalization) {
  struct AmbiguousRawInput {
    [[= serial_xml::raw]] std::string first;
    [[= serial_xml::raw]] std::string second;
  };
  EXPECT_THROW(serial_xml::from_xml<AmbiguousRawInput>("<AmbiguousRawInput>ab</AmbiguousRawInput>"),
               serial_xml::deserialization_error);
  struct NormalizedAttributeInput {
    [[= serial_xml::attribute]] std::string text;
  };
  auto result = serial_xml::from_xml<NormalizedAttributeInput>(
      "<NormalizedAttributeInput text='a\r\nb\tc&#10;d'/>");
  EXPECT_EQ(result.text, "a b c\nd");
}

TEST(FromString, CharacterRangesAndMalformedQuotes) {
  EXPECT_EQ(serial_xml::from_string<std::vector<char>>("['a', '\\n', '\\\'']"),
            (std::vector<char>{'a', '\n', '\''}));
  EXPECT_THROW(serial_xml::from_string<std::vector<std::string>>("[\"a\" \"b\"]"),
               serial_xml::deserialization_error);
  EXPECT_THROW(serial_xml::from_string<int>(std::string_view{}), serial_xml::deserialization_error);
}

TEST(FromXml, AttributesWithContainerRepresentations) {
  struct ContainerAttributeInput {
    [[= serial_xml::attribute]] std::vector<int> values;
    [[= serial_xml::attribute]] std::optional<std::vector<int>> maybe;
  };
  ContainerAttributeInput original{{1, 2}, std::vector<int>{3, 4}};
  auto xml = serial_xml::to_xml(original, false);
  EXPECT_EQ(xml, "<ContainerAttributeInput values=\"[1, 2]\" maybe=\"[3, 4]\"/>");
  auto restored = serial_xml::from_xml<ContainerAttributeInput>(xml);
  EXPECT_EQ(restored.values, original.values);
  EXPECT_EQ(restored.maybe, original.maybe);
}

TEST(FromString, DebugUnicodeEscapes) {
  using Strings = std::vector<std::string>;
  EXPECT_EQ(serial_xml::from_string<Strings>("[\"\\u{1}\", \"\\u{1f600}\"]"),
            (Strings{std::string("\x01", 1), "😀"}));
  auto values = Strings{std::string("\x01", 1), "quote\" newline\n"};
  EXPECT_EQ(serial_xml::from_string<Strings>(std::format("{}", values)), values);
  for (auto input : {"[\"\\u{}\"]", "[\"\\u{D800}\"]", "[\"\\u{110000}\"]", "[\"\\u{1\"]"}) {
    EXPECT_THROW(serial_xml::from_string<Strings>(input), serial_xml::deserialization_error);
  }
}

TEST(FromXml, UnicodeNamesAndProcessingInstructions) {
  struct[[= serial_xml::name{"résumé"}]] UnicodeNameInput {
    [[= serial_xml::name{"état"}]] int value;
  };
  EXPECT_EQ(serial_xml::from_xml<UnicodeNameInput>(
                "<?xml-stylesheet href='example'?><résumé><?work test?><état>7</état></résumé>")
                .value,
            7);
  EXPECT_THROW(serial_xml::from_xml<UnicodeNameInput>("<\xC2\x80/>"),
               serial_xml::deserialization_error);
  std::string override_name = "override";
  EXPECT_EQ(
      serial_xml::from_xml<UnicodeNameInput>("<override><état>8</état></override>", override_name)
          .value,
      8);
}

TEST(FromXml, EscapingAcrossSimdAndMaskBoundaries) {
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
    EXPECT_EQ(xml, "<EscapeBoundaryInput attribute=\"" + escaped + "\"><text>" + escaped +
                       "</text>" + escaped + "</EscapeBoundaryInput>")
        << size;
    auto restored = serial_xml::from_xml<EscapeBoundaryInput>(xml);
    EXPECT_EQ(restored.attribute, value) << size;
    EXPECT_EQ(restored.text, value) << size;
    EXPECT_EQ(restored.raw, value) << size;
  }
  for (std::size_t position = 0; position < 140; ++position) {
    std::string value(140, 'a');
    value[position] = '&';
    auto restored = serial_xml::from_xml<EscapeBoundaryInput>(
        serial_xml::to_xml(EscapeBoundaryInput{value, value, value}));
    EXPECT_EQ(restored.text, value) << position;
  }
}

TEST(FromXml, RawPrecedenceAndOptionalRawDefaults) {
  struct RawPrecedenceChild {
    int value;
  };
  struct RawPrecedenceInput {
    [[= serial_xml::raw]] RawPrecedenceChild child;
    [[= serial_xml::raw]] std::optional<int> value;
  };
  RawPrecedenceInput original{{7}, 8};
  auto restored = serial_xml::from_xml<RawPrecedenceInput>(serial_xml::to_xml(original));
  EXPECT_EQ(restored.child.value, 7);
  EXPECT_EQ(restored.value, 8);
  struct OptionalRawInput {
    [[ = serial_xml::raw, = serial_xml::optional ]] std::string text = "default";
    [[ = serial_xml::raw, = serial_xml::optional, = serial_xml::iter{"item"} ]] std::vector<int>
        values{9};
  };
  auto absent = serial_xml::from_xml<OptionalRawInput>("<OptionalRawInput/>");
  EXPECT_EQ(absent.text, "default");
  EXPECT_EQ(absent.values, (std::vector<int>{9}));
}

TEST(FromXml, StringInputWithLiteralRootOverride) {
  struct StringRootOverrideInput {
    int value;
  };
  std::string xml = "<record><value>42</value></record>";
  const std::string const_xml = xml;
  EXPECT_EQ(serial_xml::from_xml<StringRootOverrideInput>(xml, "record").value, 42);
  EXPECT_EQ(serial_xml::from_xml<StringRootOverrideInput>(const_xml, "record").value, 42);
  std::string_view view = xml;
  EXPECT_EQ(serial_xml::from_xml<StringRootOverrideInput>(view, "record").value, 42);
  EXPECT_EQ(xml, const_xml);
}

TEST(FromXml, BorrowedParserTextProducesOwningResults) {
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
  EXPECT_EQ(result.text, "long text & decoded text owned by result");
  EXPECT_EQ(result.values, (std::vector<std::string>{"first < value", "second value"}));
}

TEST(FromXml, ArenaOverflowPreservesDecodedValues) {
  struct ArenaOverflowInput {
    std::vector<std::string> values;
  };
  ArenaOverflowInput original;
  for (int i = 0; i < 600; ++i)
    original.values.push_back(std::to_string(i) + " long & escaped <text>");
  auto restored = serial_xml::from_xml<ArenaOverflowInput>(serial_xml::to_xml(original));
  EXPECT_EQ(restored.values, original.values);
}

TEST(FromXml, InPlaceInputMayAliasAnUpdatedField) {
  struct AliasedInputRecord {
    std::string source;
    int value;
  };
  AliasedInputRecord record{
      "<AliasedInputRecord><source>replacement</source><value>42</value></AliasedInputRecord>", 0};
  serial_xml::from_xml(record, record.source);
  EXPECT_EQ(record.source, "replacement");
  EXPECT_EQ(record.value, 42);
}

TEST(FromXml, MixedTextNormalizationAndArenaLifetimes) {
  struct MixedArenaInput {
    [[= serial_xml::attribute]] std::string attribute;
    std::string text;
  };
  auto result = serial_xml::from_xml<MixedArenaInput>(
      "<MixedArenaInput attribute='é&#x1F600;\t&amp;&#13;'>"
      "<text>é&#x1F600;\r\n<![CDATA[<&>]]><!--gap-->tail</text></MixedArenaInput>");
  EXPECT_EQ(result.attribute, "é😀 &\r");
  EXPECT_EQ(result.text, "é😀\n<&>tail");
}

TEST(FromXml, AsciiFastPathValidatesEveryByteAndWordBoundary) {
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
        EXPECT_EQ(serial_xml::from_xml<AsciiBoundaryInput>(xml).value, text)
            << byte << ":" << position;
      } else {
        EXPECT_THROW(serial_xml::from_xml<AsciiBoundaryInput>(xml),
                     serial_xml::deserialization_error)
            << byte << ":" << position;
      }
    }
  }
}

TEST(FromXml, MalformedLateTextDoesNotUpdateInPlaceTarget) {
  struct ArenaAtomicParseInput {
    std::string first;
    std::string second;
  };
  ArenaAtomicParseInput original{"original first", "original second"};
  EXPECT_THROW(
      serial_xml::from_xml(original,
                           "<ArenaAtomicParseInput><first>new first</first><second>invalid "
                           "&bad;</second></ArenaAtomicParseInput>"),
      serial_xml::deserialization_error);
  EXPECT_EQ(original.first, "original first");
  EXPECT_EQ(original.second, "original second");
}
