(example-named-tags)=
# Match an existing XML naming scheme

Use explicit names when an XML schema uses different identifiers from your C++ code.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Name the root and child

```{literalinclude} ../_snippets/how-to/naming.cpp
:language: cpp
:lines: 1-7
```

Place the root annotation after the `struct` keyword. The member annotation changes `favorite_food` to `food` in XML without changing its C++ identifier.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/naming.cpp
:language: cpp
:lines: 9-
```

Both writing and reading use `person` as the root, `age` as an attribute, and `food` as a child. An annotation on a nested type applies within its containing member wrapper.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<person age="3"><food>pizza</food></person>
```

```text
Age: 3; food: pizza
```

## Apply this to your schema

To override only the root at runtime, pass the same name to both calls: `to_xml(original, false, "customer")` and `from_xml<Person>(xml, "customer")`. That override takes precedence over the root annotation.
