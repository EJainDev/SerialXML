(example-mocking)=
# Annotate a class you cannot modify

Use an external schema to expose a const getter on an existing class. This example reads the size of a standard vector without modifying its definition.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Declare the external schema

```{literalinclude} ../_snippets/how-to/external-schema.cpp
:language: cpp
:lines: 1-7
```

The placeholder getter has the same identifier as the vector’s accessible `size()` method. It needs no definition: the library calls the method on the actual vector, and never constructs a mock.

## Write the selected representation

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/external-schema.cpp
:language: cpp
:lines: 9-
```

The template argument selects the mock’s annotations. The attribute contains the actual size, while the root uses the real type’s identifier, `vector`. The schema selects only `size`, so the vector’s elements are not written.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<vector size="3"/>
```

```text
Actual vector size: 3
```

## Apply this to your schema

This schema cannot reconstruct the vector’s contents: a size getter has no writable counterpart, and the document has no items. For an input schema, expose writable fields or setters. Match directly accessible member identifiers and field/getter kinds; invalid or ambiguous mappings are diagnosed at compile time.
