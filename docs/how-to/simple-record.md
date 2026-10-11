(example-hello-world)=
# Serialize a simple record

Use this guide when your C++ member names already match the XML you want to write.
The program below saves a `Person`, prints its XML, and reads the values back.

Start with a CMake application linked to `serial_xml::serial_xml`. If you have
not set that up yet, follow [installation](../installation.md). Save the following
code in `main.cpp`.

## Define the record

Import the standard library and SerialXML, then declare the data you want to save:

```cpp
import std;
import serial_xml;

struct Person {
  int age;
  std::string favorite_food;
};
```

No annotations are needed. `Person` becomes the root element, and `age` and
`favorite_food` become child elements in their declaration order.

## Write the XML

Add `main` below the record definition:

```cpp
int main() {
  const Person original{3, "pizza"};
  const std::string xml = serial_xml::to_xml(original, false);
  std::println("{}", serial_xml::prettify(xml));
}
```

`to_xml` returns the document as a string. The `false` argument omits the XML
declaration, which is useful when you want a fragment. Omit that argument when
you want the declaration included.

Configure, build, and run the application from the directory containing your
`CMakeLists.txt`:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<Person>
  <age>3</age>
  <favorite_food>pizza</favorite_food>
</Person>
```

`prettify` adds indentation for display. The original `xml` string remains compact:

```xml
<Person><age>3</age><favorite_food>pizza</favorite_food></Person>
```

## Read the values back

Replace `main` with this version to reconstruct the record from the same XML:

```cpp
int main() {
  const Person original{3, "pizza"};
  const std::string xml = serial_xml::to_xml(original, false);
  std::println("{}", serial_xml::prettify(xml));

  const Person restored = serial_xml::from_xml<Person>(xml);
  std::println("Age: {}", restored.age);
  std::println("Favorite food: {}", restored.favorite_food);
}
```

The `Person` template argument tells the reader which type to construct. It
matches the `Person` root and populates the members from their named child
elements. Pass the compact `xml` string directly; pretty printing is only needed
for the displayed document.

After building and running again, the complete output is:

```xml
<Person>
  <age>3</age>
  <favorite_food>pizza</favorite_food>
</Person>
```

```text
Age: 3
Favorite food: pizza
```

Your `main.cpp` now consists of the imports and record definition from the first
section followed by this final `main` function. Change the initializer to
`Person{7, "pasta"}` to write and read different values using the same XML shape.
