<span id="how-to-cookbook"></span>

# Example programs

Build and run the example applications included with SerialXML. Each example
imports the library module and writes XML from ordinary C++ records or annotated
schemas. Use this guide to run the repository programs without creating a separate
consumer application.

## Run the accompanying examples

From the repository root, configure a release build with examples enabled and
benchmarks disabled:

```bash
cmake --preset release-gcc -DBUILD_EXAMPLES=ON -DBUILD_BENCHMARKS=OFF
cmake --build --preset build-release
```

The dev container supplies the required GCC, CMake, and Ninja toolchain. Keeping
benchmarks disabled lets you build the examples without Boost Serialization or
cereal. The executables are written under `build/release/examples/`.

## Inspect the first program

The first portion of `examples/hello_world.cpp` imports the modules and defines
the record:

```{literalinclude} ../examples/hello_world.cpp
:language: cpp
:lines: 1-7
```

The remaining portion creates a record, serializes it, and prints readable XML:

```{literalinclude} ../examples/hello_world.cpp
:language: cpp
:lines: 9-
```

This is the complete source of the first example. Its default `to_xml` call
includes the XML declaration.

## Run and check the output

```bash
./build/release/examples/hello_world
```

The application prints:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Person>
  <age>3</age>
  <favorite_food>pizza</favorite_food>
</Person>
```

Run another program by using its source filename without `.cpp`. For example:

```bash
./build/release/examples/attributes
./build/release/examples/deserialization
```

The attribute program shows scalar values on opening tags. The deserialization
program demonstrates a custom product-code conversion, a selected setter, and
an in-place update. Some programs print labels before XML; those labels are not
part of the documents.

## Choose an example

The task guides below explain the code in portions and include the resulting
output. The repository programs may exercise additional variations of each task.

| Task guide |
| --- |
| <span id="example-hello-world"></span><span id="serialize-a-simple-record"></span>[Serialize a simple record](how-to/simple-record.md) |
| <span id="example-attributes"></span><span id="put-a-value-in-an-xml-attribute"></span>[Put a value in an XML attribute](how-to/attributes.md) |
| <span id="example-named-tags"></span><span id="match-an-existing-xml-naming-scheme"></span>[Match an existing XML naming scheme](how-to/naming.md) |
| <span id="example-skip-members"></span><span id="leave-a-field-out-of-the-schema"></span>[Leave a field out of the schema](how-to/skip-fields.md) |
| <span id="example-nested-structs"></span><span id="serialize-a-nested-object"></span>[Serialize a nested object](how-to/nested-objects.md) |
| <span id="example-mocking"></span><span id="annotate-a-class-you-cannot-modify"></span>[Annotate a class you cannot modify](how-to/external-schema.md) |
| <span id="example-deserialization"></span><span id="round-trip-a-custom-leaf-representation"></span>[Round-trip a custom leaf representation](how-to/custom-conversion.md) |
| <span id="example-stl-containers"></span><span id="choose-a-container-representation"></span>[Choose a container representation](how-to/containers.md) |
| <span id="example-iteration"></span><span id="choose-item-and-container-names"></span>[Choose item and container names](how-to/item-names.md) |
| <span id="example-optional-types"></span><span id="omit-a-value-that-is-not-present"></span>[Omit a value that is not present](how-to/optional-values.md) |
| <span id="example-raw-cdata"></span><span id="write-direct-text-or-cdata"></span>[Write direct text or CDATA](how-to/text-and-cdata.md) |
| <span id="example-format-specifiers"></span><span id="format-a-numeric-value"></span>[Format a numeric value](how-to/numeric-formatting.md) |
| <span id="example-escaping"></span><span id="preserve-xml-special-characters"></span>[Preserve XML-special characters](how-to/escaping.md) |
| <span id="example-advanced-mixed"></span><span id="combine-nesting-and-annotations"></span>[Combine nesting and annotations](how-to/combined-schema.md) |
