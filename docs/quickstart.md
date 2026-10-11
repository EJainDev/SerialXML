# Tutorial: your first XML round trip

In this tutorial, you will build a small program that writes a `Person` to XML,
prints the document, and reads the same values back. You will use an ordinary
struct with no annotations.

## 1. Prepare the application

You need GCC 16.1+ with reflection support, CMake 4.3.3+, Ninja, and network
access for the CMake dependencies. The repository's dev container supplies the
toolchain. Select that GCC as your compiler before configuring the application.

Create an empty directory for this tutorial. Save the following configuration as
`CMakeLists.txt` in that directory:

```{literalinclude} _snippets/CMakeLists.txt
:language: cmake
:caption: CMakeLists.txt
```

Create `main.cpp` beside it. Import the standard library and SerialXML, then define the
object you will save:

```cpp
import std;
import serial_xml;

struct Person {
  int age;
  std::string favorite_food;
};
```

## 2. Write the object to XML

Add a `main` function that creates a person and serializes it:

```cpp
int main() {
  const Person original{3, "pizza"};
  const auto xml = serial_xml::to_xml(original);
  std::println("{}", serial_xml::prettify(xml));
}
```

Configure and build your application from its root:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

You should see:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Person>
  <age>3</age>
  <favorite_food>pizza</favorite_food>
</Person>
```

The struct name becomes the root element; its members become child elements.
`to_xml` produces compact XML. `prettify` makes that document readable for display.

## 3. Read the document back

Before the closing brace of `main`, add:

```cpp
const auto restored = serial_xml::from_xml<Person>(xml);
std::println("Age: {}; favorite food: {}", restored.age, restored.favorite_food);
```

Build and run again. The final line should be:

```text
Age: 3; favorite food: pizza
```

You have now saved and restored the same object. The `Person` definition tells
SerialXML how to interpret the XML without a separate runtime schema.

## 4. Change a value

Change `original` to `Person{7, "pasta"}`, rebuild, and run. The XML should contain
`<age>7</age>` and `<favorite_food>pasta</favorite_food>`, and the final line should
show the new values. The shape of the document stays the same.

## Completed program

Your `main.cpp` should now look like this (using the original values):

```cpp
import std;
import serial_xml;

struct Person {
  int age;
  std::string favorite_food;
};

int main() {
  const Person original{3, "pizza"};
  const auto xml = serial_xml::to_xml(original);
  std::println("{}", serial_xml::prettify(xml));
  const auto restored = serial_xml::from_xml<Person>(xml);
  std::println("Age: {}; favorite food: {}", restored.age, restored.favorite_food);
}
```

## Continue learning

Continue with [Shape an XML document](tutorials/schema.md) to add attributes,
rename fields, and serialize a collection. If the application did not build,
check [integration troubleshooting](installation.md#troubleshoot-integration).
For a specific task, browse the [how-to guides](how-to/index.md).
