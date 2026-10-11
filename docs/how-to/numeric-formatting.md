(example-format-specifiers)=
# Format a numeric value

Use a format annotation to give a numeric field a fixed decimal width.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Attach the decimal format

```{literalinclude} ../_snippets/how-to/numeric-formatting.cpp
:language: cpp
:lines: 1-6
```

`03d` requests three decimal digits with leading zeroes. Pass the specification without a leading colon or surrounding braces. Combine it with `attribute` to format the value on the opening tag.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/numeric-formatting.cpp
:language: cpp
:lines: 8-
```

The writer emits `042`. The default numeric reader accepts the zero-padded decimal representation and restores the integer 42.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<FormattedAttribute x="042"/>
```

```text
Restored value: 42
```

## Apply this to your schema

The same format works on a child member without `attribute`. Hexadecimal text, custom prefixes, and alignment fill need a matching input conversion. A format that rounds away precision cannot restore the original value.
