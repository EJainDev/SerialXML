(example-iteration)=
# Choose item and container names

Replace the default range wrapper and `element` item names with the names required by your schema.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Specify the item and wrapper names

```{literalinclude} ../_snippets/how-to/item-names.cpp
:language: cpp
:lines: 1-6
```

The first `iter` argument names each item; the second names their enclosing element. The C++ member remains `values`.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/item-names.cpp
:language: cpp
:lines: 8-
```

The writer puts each number inside a `number` element under `numbers`. The reader expects those same names and fills the vector in item order.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<Numbers>
  <numbers>
    <number>1</number>
    <number>2</number>
    <number>3</number>
  </numbers>
</Numbers>
```

```text
Number: 1
Number: 2
Number: 3
```

## Apply this to your schema

Use `iter{"number"}` to change only the item name and retain the member’s wrapper name. For a range of records, each named item contains that record’s reflected members.
