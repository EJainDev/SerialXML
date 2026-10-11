(example-optional-types)=
# Omit a value that is not present

Use an owning `std::optional` when a field may have no value in your XML document.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Declare an optional field

```{literalinclude} ../_snippets/how-to/optional-values.cpp
:language: cpp
:lines: 1-6
```

An engaged optional is written as its contained integer. `nullopt` contributes no element. This also works with an optional member annotated as an attribute.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/optional-values.cpp
:language: cpp
:lines: 8-
```

The example first reads 42, then updates the same object from the document without a value. The reader resets the absent `std::optional` to `nullopt`.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<OptionalChild><value>42</value></OptionalChild>
<OptionalChild/>
```

```text
Present value: 42
After absent input: false
```

## Apply this to your schema

To retain a current value when input omits the field, add `[[= serial_xml::optional]]`. That annotation changes the missing-input policy; it does not change what the writer emits. An empty `<value/>` is present and cannot be converted to an integer.
