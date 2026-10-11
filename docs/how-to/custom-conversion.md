(example-deserialization)=
# Round-trip a custom leaf representation

Use a formatter and a matching input conversion when XML must represent a value
as custom text. This example writes a product code as `SKU-42` and reads it back
as an integer-bearing type.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if needed. Put the following portions in
`main.cpp` in order.

## Define the leaf and its output format

```{literalinclude} ../_snippets/how-to/custom-conversion.cpp
:language: cpp
:lines: 1-8
```

`ProductCode` holds the value. `format_code` provides its textual representation
and returns an owning string. The prefix is part of the XML format, not the stored
integer.

## Supply the inverse conversion

```{literalinclude} ../_snippets/how-to/custom-conversion.cpp
:language: cpp
:lines: 10-16
```

Declare the specialization before the first use that needs it. It checks the
prefix, then delegates numeric conversion to SerialXML. That conversion rejects
invalid numeric text and overflow. The output formatter alone cannot tell the
reader how to remove the prefix.

## Attach the formatter to the record

```{literalinclude} ../_snippets/how-to/custom-conversion.cpp
:language: cpp
:lines: 18-21
```

The `format` annotation makes `code` a text leaf. The `id` member remains an
attribute. Reading a `Product` passes the leaf’s decoded text to the specialization.

## Write, read, and handle invalid input

```{literalinclude} ../_snippets/how-to/custom-conversion.cpp
:language: cpp
:lines: 23-
```

The successful round trip restores the underlying integer. The final call checks
that the custom parser reports a representation with the wrong prefix.

## Build and check the result

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<Product id="7"><code>SKU-42</code></Product>
```

```text
Product: 7; code: 42
Conversion failed: Expected SKU- code
```

Keep output and input formats compatible when changing the prefix or numeric
representation. A lossy formatter, such as one that rounds away precision, cannot
be inverted to recover the original value.
