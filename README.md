# SerialXML

![Banner](./assets/SerialXML.png)

![Build and Tests](https://github.com/EJainDev/SerialXML/actions/workflows/build-and-test.yml/badge.svg)
![C++26](https://img.shields.io/badge/C%2B%2B-26-blue)
![CMake 4.3+](https://img.shields.io/badge/CMake-4.3%2B-orange)
![License](https://img.shields.io/badge/license-MIT-lightgray)

> Reflection based XML serialization for C++26 -- call `to_xml` on any object for a string serialization

SerialXML is a C++26 reflection based serialization library for XML. Behaviour is configurable via annotations on object members and object type declarations (`class/struct`). Errors are all compile time using `static_assert` messages so any invalid XML combination is caught at compile time.

## Table of Contents
* [Quick Start](#quick-start)
* [Installation](#installation)
  * [FetchContent](#cmake-fetchcontent-recommended)
  * [Source](#install-from-source)
  * [Requirements](#requirements)
* [Benchmarks](#benchmarks)
* [Annotations](#annotations)
  * [Common Confusion Points](#common-confusion-points)
  * [Configuring `to_xml`](#configuring-to_xml)
  * [The `prettify` function](#the-prettify-function)
* [Examples](#examples)
* [Contributing](#contributing)
* [License](#license)

## Quick Start

Getting started is easy. Simply import the module and call `to_xml` on any regular C++ struct. No modifications required to anything!

```cpp
// main.cpp

import std;
import serial_xml;

struct Person {
    int age;
    std::string favorite_food;
}

int main() {
    std::print(to_xml(Person{3, "pizza"}));
}
```

That's all you need: one function call and SerialXML does the rest.

## Installation

### CMake FetchContent (Recommended)

```cmake
FetchContent_Declare(
    serial_xml
    GIT_REPOSITORY https://github.com/EJainDev/SerialXML.git
    GIT_TAG main
)
FetchContent_MakeAvailable(serial_xml)

add_executable(my_tests test.cpp)
target_link_libraries(my_tests PRIVATE serial_xml::serial_xml)
```

### Install from source

```bash
git clone https://github.com/EJainDev/SerialXML.git
cd SerialXML

cmake --preset "release-gcc-16"
cmake --build build

cmake --install build
```

Then, in your `CMakeLists.txt`, put:
```cmake
find_package(SerialXML REQUIRED)
target_link_libraries(my_app PRIVATE serial_xml::serial_xml)
```

### Requirements

| Component | Min Version | Notes |
| --------- | ----------- | ----- |
Compiler | GCC 16.1 | C++26 SIMD, Reflection, and more |
CMake | 4.3 | Change `std` experiment key for lower versions |
C++ Standard | 26 | SIMD, Reflection, Annotations |

## Benchmarks

This library is the *fastest* XML serialization library. It is 8.3x faster than `boost_xml` and 1.9x faster than `pugixml` for struct to XML serialization.

Benchmarks on my machine (single Intel Core 7 240H with 5600 MT/s DDR5):

| Library | Iterations | Time per Iter (ns) | Total Time (ms) |
| --- | --- | --- | --- |
| SerialXML | 100000 | 822.085 | 82.208 |
| Boost.Serialization | 100000 | 7899.305 | 789.931 |
| Pugixml | 100000 | 1585.212 | 158.521 |
| Cereal | 100000 | 7945.590 | 794.559 |

The main reason this library is so much faster is because all of the above ones use runtime DOM creation adding overhead. Pugixml, for example, uses two pass heap allocations while this library uses two `std::string` objects for each call to `to_xml`. Since this DOM is entirely evaluated at compile time, it also opens up opportunities for the compiler to optimize far better.

## Annotations

This library is annotation driven, which means that most customization points are exposed via C++26 annotations. The reason behind this design choice is to create consistency and visually associate the output structure to the definition.

By default, all members are treated as children of the parent struct with closing tags the same as the name of the member. However, if a struct is not formattable, it by default is unpacked. Many STL ranges and the `std::optional` container are also handled by default. A `std::optional` member is omitted if it does not contain a value.

This is the complete list of annotations:
- `[[=attribute]]` -- Mark a struct member as a XML attribute instead of a child.
- `[[=raw]]` -- Mark a struct member to be emitted as raw text instead of being surrounded by closing tags with the same name as the member.
- `[[=skip]]` -- Don't include this struct member in the generated XML output.
- `[[=name{"custom_name"}]]` -- Specify the name of this attribute or child tag to be something other than the name of the member. Note: You can also specify this on the struct to control its closing tag (eg. generate `person` instead of `Person` for `struct Person` with `[[=name{"person"}]]`).
- `[[=unpack]]` -- Instead of calling `std::format` on the member object, generate an enclosing XML tag for it and serialize its members as well.
- `[[=no_unpack]]` -- Call `std::format` on the member object instead of breaking it down into its children. Opposite of `unpack`.
- `[[=iter{a, b}]]` -- For classes satisfying `std::ranges::range`, iterate through each member instead of directly calling `std::format`. The first (optional) parameter is the name of the tag for each element in the range. The second (optional) parameter is the name of the range tag enclosing each element.
- `[[=no_iter]]` -- The opposite of `iter` to disable automatic iteration of STL ranges. See the confusion points for more information on STL handling.
- `[[=format{"format_specifier"}]]` -- Add a format specifier in the call to `std::format` for that member. Do not prefix with a colon (`:`) as the library handles that on its own.
- `[[=cdata]]` -- Emit the value inside `cdata` (`<![CDATA[your_content]]>`) tags.
- `[[=exclude_on_empty]]` -- Do not emit any tags when the range is empty

### Common Confusion Points

1. For *some* STL containers, the library automatically iterates through them. Therefore, your generated XML will not match the expectations. To avoid this, add the `[[=no_iter]]` annotation to object member. The current list of STL containers that are automatically iterated:
    - `std::vector`
    - `std::array`
    - `std::inplace_vector`
    - `std::deque`
    - `std::forward_list`
    - `std::span`
    - `std::valarray`
1. The precedence order for STL handled ranges is as follows:
    1. `exclude_on_empty`
    1. `raw` -- note that this only applies to the outer layer of tags for the range. Not each individual element in the range
    1. `cdata`
1. The precedence order for children is as follows:
    1. STL Handling (see above)
    1. `raw`
    1. Iteration
    1. Unpacking
    1. CData
1. `format` is ignored for unpacked or iterated members

### Configuring `to_xml`

The function signature of `to_xml` is the following:

```
template <typename T>
  requires(std::is_class_v<T>)
auto to_xml(const T& value, bool first = true, const std::string& fixed_name = "") -> std::string;
```

As you can see, there are some parameters for configuration"
- `value` -- the instance of the class to serialize
- `first` -- a bool indicating whether to insert the XML header defining the file as XML. `true` means yes
- `fixed_name` -- a custom name to specify for the instance being serialized. Overrides `name` annotation.

### The `prettify` function

A simple function to prettify (add indentation and newlines) the generated XML output. The only parameter is the output and it returns a new string with the output.

## Examples

The [`examples`](examples) directory contains small, standalone programs for each
major SerialXML feature. Examples are built by default; configure the project and
run an executable from the build directory, for example:

```bash
cmake --preset "release-gcc-16"
cmake --build build
./build/examples/hello_world
```

Set `-D BUILD_EXAMPLES=OFF` when configuring CMake to omit them from your build.

| Example | Demonstrates |
| ------- | ------------ |
| [`hello_world.cpp`](examples/hello_world.cpp) | A minimal struct-to-XML serialization. |
| [`attributes.cpp`](examples/attributes.cpp) | Emitting members as XML attributes. |
| [`named_tags.cpp`](examples/named_tags.cpp) | Naming root elements, attributes, and child tags. |
| [`skip_members.cpp`](examples/skip_members.cpp) | Excluding members from the output. |
| [`nested_structs.cpp`](examples/nested_structs.cpp) | Recursive serialization of nested structs and nested attributes. |
| [`stl_containers.cpp`](examples/stl_containers.cpp) | Automatic container iteration, `exclude_on_empty`, and `no_iter`. |
| [`iteration.cpp`](examples/iteration.cpp) | Custom element and container names for ranges, including ranges of structs. |
| [`optional_types.cpp`](examples/optional_types.cpp) | Omitting empty `std::optional` values and serializing present values. |
| [`raw_cdata.cpp`](examples/raw_cdata.cpp) | Raw text and CDATA output. |
| [`format_specifiers.cpp`](examples/format_specifiers.cpp) | Passing format specifications through to `std::format`. |
| [`escaping.cpp`](examples/escaping.cpp) | Escaping XML-special characters in children, attributes, and raw text. |
| [`advanced_mixed.cpp`](examples/advanced_mixed.cpp) | A combined example using attributes, named tags, nesting, iteration, and escaping. |

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on the process for submitting pull requests to us.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
