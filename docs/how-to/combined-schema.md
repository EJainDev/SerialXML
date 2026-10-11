(example-advanced-mixed)=
# Combine nesting and annotations

Build a department document with employee attributes, nested addresses, and named skill items.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Define the schema at each level

```{literalinclude} ../_snippets/how-to/combined-schema.cpp
:language: cpp
:lines: 1-20
```

`Address` renames a child. `Employee` chooses an attribute name and names its skill items. `Department` names its attribute and the employees wrapper. Declare inner records first so each type is available to the next.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/combined-schema.cpp
:language: cpp
:lines: 22-
```

The `team` wrapper contains an `element` for each employee because that vector has no custom item name. Inside each employee, the address and skill annotations apply at their own nesting levels.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<Department dept_name="Engineering">
  <team>
    <element emp_id="1">
      <name>Alice</name>
      <address>
        <street>123 Main St</street>
        <city_name>Boston</city_name>
        <zip_code>2128</zip_code>
      </address>
      <skills>
        <skill>C++</skill>
        <skill>Python</skill>
      </skills>
    </element>
  </team>
</Department>
```

```text
Department: Engineering; employee: Alice; city: Boston; skills: 2
```

## Apply this to your schema

To name each employee item `employee`, replace the employees member’s `name` annotation with `iter{"employee", "team"}`. Keep annotations on the member or type whose part of the XML you want to control.
