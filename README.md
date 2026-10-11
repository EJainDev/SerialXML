# SerialXML 1.0

![SerialXML banner](assets/SerialXML.png)

[![Build and Tests](https://github.com/EJainDev/SerialXML/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/EJainDev/SerialXML/actions/workflows/build-and-test.yml)
![C++26](https://img.shields.io/badge/C%2B%2B-26-blue)
![CMake 4.3.3+](https://img.shields.io/badge/CMake-4.3.3%2B-orange)
[![MIT License](https://img.shields.io/badge/license-MIT-lightgray)](LICENSE)

> Reflection based XML serialization and deserialization for C++26.

SerialXML turns regular C++ classes and structs into XML, and XML back into objects.
Simply import the module and call `to_xml` or `from_xml`. No boilerplate required!

Want more control? Add annotations to members or class declarations to name tags,
use attributes, skip fields, and configure formatting. Invalid annotation
combinations are diagnosed at compile time. Deserialization reports malformed
XML, missing required members, and invalid values through
`serial_xml::deserialization_error`.

**Requires GCC 16.1+, CMake 4.3.3+, and Ninja.** The dev container provides a matching environment.

The full documentation website lives in [`docs/`](docs/), with guides, examples,
and an indexed API reference. See [Building the documentation](docs/building-docs.md)
for local previews and Read the Docs setup.

## Table of Contents

- [Benchmarks](#benchmarks)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Annotations](#annotations)
- [Serialization](#serialization)
- [Deserialization](#deserialization)
- [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)

## Benchmarks

No boilerplate, and fast too. In this order benchmark, SerialXML deserialization
was **3.8× faster than cereal, 16.8× faster than Boost, and 9% faster than pugixml**.

| Library | Serialization (ns/order) | Deserialization (ns/order) |
| --- | ---: | ---: |
| **SerialXML** | **830.308** | **849.361** |
| Boost.Serialization | 6959.951 | 14250.021 |
| cereal | 7699.705 | 3195.171 |
| pugixml | 1523.398 | 936.498 |

Lower is better. Measured on an Intel Core 7 240H with GCC 16.2.0.

Reproduce in the dev container (choose an available CPU on your machine):

```sh
cmake --preset release-test-gcc -DBUILD_BENCHMARKS=ON
cmake --build --preset build-release-test
ctest --test-dir build/release --output-on-failure
taskset -c 4 ./build/release/benchmarks/xml_serialization_benchmarks 100000 1000
```

See [the benchmark source](benchmarks) for the workload and validation. Run five
times sequentially to compare medians.

## Quick Start

Getting started is easy. Import the module and call `to_xml` on a regular C++
struct. No modifications required to the struct!

```cpp
// main.cpp
import std;
import serial_xml;

struct Person {
  int age;
  std::string favorite_food;
};

int main() {
  const auto xml = serial_xml::to_xml(Person{3, "pizza"});
  std::println("{}", serial_xml::prettify(xml));
}
```

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Person>
  <age>3</age>
  <favorite_food>pizza</favorite_food>
</Person>
```

That's all you need: one function call and SerialXML does the rest. `to_xml`
returns compact XML; `prettify` makes it easier to read.

To read it back, use the same type:

```cpp
const auto person = serial_xml::from_xml<Person>(xml);
// person.age == 3, person.favorite_food == "pizza"
```

See [Installation](#installation) for the CMake setup, or jump to
[Examples](#examples) for more small programs.

## Installation

### Requirements

| Component | Minimum version | Notes |
| --- | --- | --- |
| Compiler | GCC 16.1 | C++26 reflection, annotations, and SIMD |
| CMake | 4.3.3 | Enable experimental `import std` before `project()` |
| Build system | Ninja | Used by the repository presets |
| C++ standard | 26 | Required by the library |

The dev container uses GCC 16.2.0 and CMake 4.4.3. Other compilers are not currently
supported. StructuralTuple is pinned to a tested commit.

### CMake FetchContent (recommended)

```cmake
cmake_minimum_required(VERSION 4.3.3)
# import std is experimental; its opt-in key depends on the CMake version.
if(CMAKE_VERSION VERSION_LESS 4.4)
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
else()
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_MODULE_STD ON)
project(my_app LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    serial_xml
    GIT_REPOSITORY https://github.com/EJainDev/SerialXML.git
    GIT_TAG main
)
FetchContent_MakeAvailable(serial_xml)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE serial_xml::serial_xml)
```

### Install from source

```bash
git clone https://github.com/EJainDev/SerialXML.git
cd SerialXML

cmake --preset release-gcc -DBUILD_BENCHMARKS=OFF
cmake --build --preset build-release
cmake --install build/release --prefix "$HOME/.local"
```

Use the same compiler and standard-module setup shown above, then replace the
FetchContent block with:

```cmake
find_package(SerialXML 1.0 CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE serial_xml::serial_xml)
```

Configure your application with `-DCMAKE_PREFIX_PATH="$HOME/.local"` for this
install prefix. A source install also installs the fetched StructuralTuple dependency;
if using an existing StructuralTuple package, make its prefix available to consumers too.
Reflection compiler options and the C++26 requirement propagate from the library target.
FetchContent builds default to disabling examples and benchmarks when embedded.

## Annotations

This library is annotation driven. Most customization points are exposed via
C++26 annotations, so the XML structure stays visually associated with the C++
definition.

By default, members become child elements named after the member. Classes that
cannot be formatted are unpacked into their own members. Many STL ranges and
`std::optional` are handled automatically; an empty `std::optional` is omitted.

For example, here's how to make `age` an attribute and rename `favorite_food`:

```cpp
struct Person {
  [[= serial_xml::attribute]] int age;
  [[= serial_xml::name{"food"}]] std::string favorite_food;
};

const auto xml = serial_xml::to_xml(Person{3, "pizza"}, false);
// <Person age="3"><food>pizza</food></Person>
```

The annotations below live in the `serial_xml` namespace. Use the qualified name,
as above, or bring the annotations into scope.

| Annotation | What it does | Details |
| --- | --- | --- |
| `[[=attribute]]` | Emit a member as an XML attribute. | Uses the member name unless overridden by `name`. |
| `[[=name{"custom_name"}]]` | Rename an attribute or element. | Also works on a class or struct to rename its root element. |
| `[[=skip]]` | Omit a member. | Ignored by serialization and deserialization. |
| `[[=raw]]` | Emit text without the member's enclosing tag. | For ranges, removes the outer range tag, not the item tags. Text is still XML-escaped. |
| `[[=cdata]]` | Emit the value in a CDATA section. | Produces `<![CDATA[your_content]]>`. |
| `[[=unpack]]` | Serialize an object's members inside an enclosing element. | Uses reflection instead of `std::format`. |
| `[[=no_unpack]]` | Format an object as text. | Disables unpacking. |
| `[[=iter{a, b}]]` | Iterate a range instead of formatting it as text. | Optional `a` names each item; optional `b` names the enclosing range tag. |
| `[[=no_iter]]` | Disable automatic range iteration. | Uses the range's formatted representation. |
| `[[=exclude_on_empty]]` | Omit tags for an empty range. | See the precedence rules below. |
| `[[=format{"format_specifier"}]]` | Pass a format specifier to `std::format`. | Leave out the leading `:`; SerialXML adds it. |
| `[[=format{format_function}]]` | Use a custom formatting function. | Accepts the member value and returns a string-like value. |
| `[[=optional]]` | Allow a field or selected setter to be absent when reading. | An absent member keeps its initializer or current value. |
| `[[=setter]]` | Select a one-parameter method for XML input. | Ignored by `to_xml`; see [Setters and encapsulation](#setters-and-encapsulation). |

### Common confusion points

Some STL containers are automatically iterated. If you want their formatted text
instead, add `[[=serial_xml::no_iter]]` to the member. The automatically iterated
containers are `std::vector`, `std::array`, `std::inplace_vector`, `std::deque`,
`std::forward_list`, `std::span`, and `std::valarray`.

When annotations overlap, precedence matters:

| Context | Precedence (highest first) |
| --- | --- |
| Automatically handled STL ranges | `exclude_on_empty` → `raw` → `cdata` |
| Child members | STL handling → `raw` → iteration → unpacking → `cdata` |

For ranges, `raw` only removes the outer layer of tags. Individual elements keep
their tags. `format` is ignored for unpacked or iterated members.

## Serialization

### Configuring `to_xml`

`to_xml` accepts the object, an XML declaration flag, and an optional root name:

```cpp
template <typename Mock = void, typename T>
  requires(std::is_class_v<T> && (std::is_void_v<Mock> || std::is_class_v<Mock>))
auto to_xml(const T& value, bool first = true, const std::string& fixed_name = "") -> std::string;
```

| Parameter | Default | Purpose |
| --- | --- | --- |
| `value` | Required | The class instance to serialize. |
| `first` | `true` | Include the XML declaration. Pass `false` for an XML fragment. |
| `fixed_name` | `""` | Override the root name, including any `name` annotation. |

### Mocking classes

Use `to_xml<Mock>(value)` to serialize a class you cannot annotate directly:

```cpp
class mock_vector {
 public:
  [[= serial_xml::attribute]] std::size_t size() const;
};

const std::vector<int> values{1, 2, 3};
std::println("{}", serial_xml::to_xml<mock_vector>(values, false));
// <vector size="3"/>
```

The mock lists the fields and const getters to serialize, in output order, and
supplies their XML annotations. Member identifiers must match accessible members
of the actual class. Field placeholders match fields; getter placeholders match
non-static const getters callable without arguments, including getters with
default arguments.

Mock methods need no implementation, and the mock is never constructed.
Serialization reads values from the actual object, using its member types for
formatting, iteration, and nested serialization. The mock's annotations replace
the target member's annotations; nested objects use their own annotations normally.

The root name defaults to the actual type's identifier. A `name` annotation on the
mock overrides it, and `fixed_name` overrides both. Members marked `skip` need not
exist on the target. Missing members, incompatible field/getter kinds, and
ambiguous callable const overloads produce compile-time errors. Mapping covers
direct accessible members; inherited members are not included.

See [the mocking example](examples/mocking.cpp).

### Pretty printing

`serial_xml::prettify(xml)` validates the input and returns XML with two-space
indentation for element-only content. Subtrees containing direct text (including
whitespace), CDATA, or an `xml:space` attribute retain their original bytes. This
preserves string leaf values and mixed content, including quoted `>` characters,
comments, and processing instructions. Malformed input throws `deserialization_error`.

Indentation adds whitespace between elements. Use compact `to_xml` output for
round trips involving raw scalar fields alongside child elements, or when every
whitespace node matters.

## Deserialization

`serial_xml::from_xml<T>(xml)` constructs a value-initialized `T` and reconstructs it
using the same member names, attributes, nesting, iteration, escaping, and mock
annotations as `to_xml`. `serial_xml::from_xml<T, Schema>(xml)` uses an external mock
schema. Both accept an optional root-name override as their second argument.

For an existing object, including one without a default constructor, use
`serial_xml::from_xml<Schema>(object, xml, fixed_name)`; omit `Schema` for the object's
own annotations. The in-place form snapshots the input so XML stored in an updated
field remains safe, and parses it before updating the object. A later conversion
error or throwing setter can leave earlier members updated.

```cpp
struct Person {
  [[= serial_xml::attribute]] int age;
  std::string name;
  [[= serial_xml::optional]] std::string nickname = "unknown";
};

auto person = serial_xml::from_xml<Person>(
  "<Person age='21'><name>Ekansh</name></Person>");
// person.nickname remains "unknown".
```

### Missing members and empty values

Members are **required** unless marked `[[=serial_xml::optional]]`, skipped, or
omitted naturally by serialization. Here's what happens when a member is absent:

| Member | Result when absent |
| --- | --- |
| Regular required member | Throws `deserialization_error`. |
| Explicitly `optional` member or setter | Keeps its initializer, or its current value with the in-place API. |
| `std::optional<T>` without an explicit `optional` annotation | Resets to `std::nullopt`. |
| Automatically iterated `exclude_on_empty` member without an explicit `optional` annotation | Resets to an empty value. |
| Skipped field or unselected mock field | Keeps its initialized value, or its current value with the in-place API. |

An empty element counts as present: it produces an empty string or range but
fails numeric conversion. Fixed arrays require the exact number of elements.
Raw ranges may have no elements because their enclosing tag is omitted.

Unknown attributes and child elements are ignored. Duplicate matched singleton
elements and duplicate XML attributes are rejected. XML child and attribute order
is otherwise independent of C++ member order.

### Leaf conversion with `from_string`

Every non-unpacked leaf is converted through
`serial_xml::from_string<T>(std::string_view)`. Built-in implementations cover
arithmetic types (including booleans), `std::string`, `std::optional`, pairs,
tuples, and owning STL sequences, sets, and maps. Numbers must consume the entire
input after trimming surrounding XML whitespace; overflow and invalid text throw
`deserialization_error`. Booleans accept `true`, `false`, `1`, and `0`; characters
use their numeric value, matching `to_xml`. Strings retain whitespace.

Container conversion reads the standard formatted representations, such as
`[1, 2]`, `(1, "text")`, and `{"key": 3}`, including nested containers and quoted
strings. XML iteration handles `vector`, `array`, `inplace_vector`, `deque`,
`forward_list`, and `valarray` automatically. Use `iter` for other owning ranges.
Borrowed views (`span`, `string_view`, pointers) have no built-in reconstruction:
provide an implementation with an appropriate storage lifetime if you need them.

A custom leaf type requires an explicit specialization, declared before the first
call that needs it. An unpacked class uses reflection instead.

```cpp
struct Code {
  int value;
};

template <>
Code serial_xml::from_string<Code>(std::string_view text) {
  return {serial_xml::from_string<int>(text)};
}
```

`format` continues to determine whether a value is represented as text.
Deserialization passes that text to `from_string`; it does not try to invert
custom formatter functions or lossy format strings. Decimal zero padding works
with the default numeric parser. Custom prefixes, hexadecimal representations,
alignment fill, or other representations need a matching `from_string`
implementation. Lost precision cannot be recovered.

### Setters and encapsulation

Accessible const getters can be reconstructed through a matching non-static setter
with exactly one parameter of the same value type (references and cv qualifiers
are ignored). Supported conventions are `value()` → `value(T)` or `set_value(T)`,
`get_value()` → `set_value(T)`, and `getValue()` → `setValue(T)`. XML naming and
annotations come from the getter. Getters without a matching setter are ignored
by deserialization. Non-function members that cannot be assigned produce a
compile-time error; mark them `skip` to omit them.

Use `[[= serial_xml::setter]]` to select a method independently, without requiring
a getter. It must be an accessible non-static method with one parameter. Its
parameter type controls reconstruction, and its own annotations control XML
naming, attributes, optionality, and iteration. By default `set_value` reads
`value` and `setValue` reads `value`; other method names are used literally.
`name` overrides this. Setters are ignored by `to_xml`.

```cpp
class Account {
 public:
  [[= serial_xml::skip]] int balance() const { return balance_; }
  [[= serial_xml::setter, = serial_xml::name{"balance"}]]
  void deposit_balance(int amount) { balance_ = amount; }

 private:
  int balance_ = 0;
};
```

An explicitly selected setter takes precedence over automatic reconstruction of
its matching getter, so it is invoked once. Setter annotations can also live on
a mock schema; placeholder methods do not need definitions. Ambiguous setter
overloads are diagnosed at compile time.

### Supported XML input

The reader supports UTF-8 XML 1.0, a UTF-8 BOM, XML declarations, quoted attributes,
self-closing elements, comments, processing instructions, CDATA, the five predefined
entities, and decimal/hexadecimal Unicode character references. It rejects
mismatched tags, invalid characters/UTF-8, unclosed structures, invalid numeric
values, missing required members, and multiple roots. DTDs and external entities
are unsupported. Namespace prefixes are matched literally; namespace declarations
are not resolved. Nesting is limited to 256 elements. Line endings and literal
attribute whitespace follow XML normalization rules.

### Raw text, CDATA, and escaping

`raw` scalar fields read the parent's direct text; more than one raw scalar field
cannot be separated and causes an error. `raw` ranges read matching item tags
directly from the parent. Unpacked objects and automatically handled optionals
keep their tagged representation even with `raw`, following `to_xml` precedence.

CDATA fields read the parent's CDATA sections in member order, matching `to_xml`'s
unwrapped CDATA output. The writer splits `]]>` across CDATA sections and emits carriage returns as
character references; the reader rejoins those canonical continuations. Adjacent
fields whose boundaries look exactly like a canonical split (a field ending in
`]]` followed by one starting in `>`) and optional or externally split CDATA fields
can be ambiguous; use tagged string fields when boundaries must be
unambiguous. Standard tagged string fields can contain mixed ordinary text and
CDATA sections, which are concatenated.

Serialization rejects forbidden XML 1.0 characters and malformed UTF-8 with
`std::invalid_argument`, including custom formatter output. Carriage returns in
text and tabs/newlines/carriage returns in attributes use character references to
preserve their values. Annotation names must be valid XML 1.0 names at compile
time; invalid runtime root overrides throw `std::invalid_argument`. Unicode,
underscores, and literal namespace prefixes are accepted.

See [the deserialization example](examples/deserialization.cpp) for round trips,
custom string conversion, optional fields, and independent setters.

## Examples

The [`examples`](examples) directory contains small, standalone programs for each
major SerialXML feature. Examples are built by default; configure the project and
run an executable from the build directory, for example:

```bash
cmake --preset release-gcc -DBUILD_BENCHMARKS=OFF
cmake --build --preset build-release
./build/release/examples/hello_world
```

Set `-DBUILD_EXAMPLES=OFF` when configuring CMake to omit them from your build.

| Example | Demonstrates |
| ------- | ------------ |
| [`hello_world.cpp`](examples/hello_world.cpp) | A minimal struct-to-XML serialization. |
| [`attributes.cpp`](examples/attributes.cpp) | Emitting members as XML attributes. |
| [`named_tags.cpp`](examples/named_tags.cpp) | Naming root elements, attributes, and child tags. |
| [`skip_members.cpp`](examples/skip_members.cpp) | Excluding members from the output. |
| [`nested_structs.cpp`](examples/nested_structs.cpp) | Recursive serialization of nested structs and nested attributes. |
| [`mocking.cpp`](examples/mocking.cpp) | External annotations for classes you cannot modify. |
| [`deserialization.cpp`](examples/deserialization.cpp) | Round trips, custom leaf conversion, optional reconstruction, and setters. |
| [`stl_containers.cpp`](examples/stl_containers.cpp) | Automatic container iteration, `exclude_on_empty`, and `no_iter`. |
| [`iteration.cpp`](examples/iteration.cpp) | Custom element and container names for ranges, including ranges of structs. |
| [`optional_types.cpp`](examples/optional_types.cpp) | Omitting empty `std::optional` values and serializing present values. |
| [`raw_cdata.cpp`](examples/raw_cdata.cpp) | Raw text and CDATA output. |
| [`format_specifiers.cpp`](examples/format_specifiers.cpp) | Passing format specifications through to `std::format`. |
| [`escaping.cpp`](examples/escaping.cpp) | Escaping XML-special characters in children, attributes, and raw text. |
| [`advanced_mixed.cpp`](examples/advanced_mixed.cpp) | A combined example using attributes, named tags, nesting, iteration, and escaping. |

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for setup instructions, testing
guidelines, and the pull request process.

## License

SerialXML is licensed under the [MIT License](LICENSE).
