# Shape an XML document

In this tutorial, you will extend the application from
[your first XML round trip](../quickstart.md). You will create a `person` element
with an age attribute, a renamed food element, and a list of hobbies, then read
it back. Keep the same `CMakeLists.txt` and replace `main.cpp` as instructed below.

## 1. Name the root and one child

Keep the imports and replace your `Person` declaration with:

```cpp
struct [[= serial_xml::name{"person"}]] Person {
  int age;
  [[= serial_xml::name{"food"}]] std::string favorite_food;
};
```

Replace `main` with this fragment-printing version:

```cpp
int main() {
  const Person original{3, "pizza"};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", xml);
}
```

Run `cmake --build build` and `./build/my_app`. You should see:

```xml
<person><age>3</age><food>pizza</food></person>
```

Both annotations changed XML names. The C++ member still has the name
`favorite_food`, so the initializer and member access remain the same.

## 2. Make age an attribute

Change the age declaration to:

```cpp
[[= serial_xml::attribute]] int age;
```

Build and run again. Your output should be:

```xml
<person age="3"><food>pizza</food></person>
```

The age is now carried by the opening tag. The food remains a child element.

## 3. Add named repeated items

Add this member after `favorite_food`:

```cpp
[[= serial_xml::iter{"hobby"}]] std::vector<std::string> hobbies;
```

Change the initializer in `main` to:

```cpp
const Person original{3, "pizza", {"drawing", "cycling"}};
```

Build and run. You should see two `hobby` elements inside the `hobbies` wrapper:

```xml
<person age="3"><food>pizza</food><hobbies><hobby>drawing</hobby><hobby>cycling</hobby></hobbies></person>
```

## 4. Read the shaped document

Add these lines after printing `xml`:

```cpp
const auto restored = serial_xml::from_xml<Person>(xml);
std::println("Age: {}; food: {}; hobbies: {}", restored.age,
             restored.favorite_food, restored.hobbies.size());
```

Build and run once more. The last line should be:

```text
Age: 3; food: pizza; hobbies: 2
```

The reader uses the same names, attribute placement, and item names as the writer.

## Completed program

```cpp
import std;
import serial_xml;

struct [[= serial_xml::name{"person"}]] Person {
  [[= serial_xml::attribute]] int age;
  [[= serial_xml::name{"food"}]] std::string favorite_food;
  [[= serial_xml::iter{"hobby"}]] std::vector<std::string> hobbies;
};

int main() {
  const Person original{3, "pizza", {"drawing", "cycling"}};
  const auto xml = serial_xml::to_xml(original, false);
  std::println("{}", xml);
  const auto restored = serial_xml::from_xml<Person>(xml);
  std::println("Age: {}; food: {}; hobbies: {}", restored.age,
               restored.favorite_food, restored.hobbies.size());
}
```

## Continue learning

You have shaped individual fields and repeated items. Continue with
[Build a book catalogue](catalog.md) to combine these choices with nested records
and reconstruct a whole collection.
