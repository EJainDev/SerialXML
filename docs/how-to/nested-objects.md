(example-nested-structs)=
# Serialize a nested object

Write a record containing another record without flattening the nested fields.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Define the inner and outer records

```{literalinclude} ../_snippets/how-to/nested-objects.cpp
:language: cpp
:lines: 1-13
```

Declare `Address` before `Person`, then use it as the member type. These records have accessible data members and no custom textual formatter, so reflection expands their fields.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/nested-objects.cpp
:language: cpp
:lines: 15-
```

The nested wrapper is named `address`, after the member, rather than `Address`, after the type. Its children come from the inner record. Reading reconstructs both levels from the same structure.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<Person>
  <name>Alice</name>
  <address>
    <street>123 Main St</street>
    <city>Boston</city>
    <zip>2128</zip>
  </address>
</Person>
```

```text
Alice lives in Boston (2128)
```

## Apply this to your schema

An attribute annotation on an `Address` member places that value on the `address` wrapper. Add annotations at the level whose XML shape you want to change.
