(example-attributes)=
# Put a value in an XML attribute

Use an attribute for scalar metadata while keeping other values as child elements.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Choose the attribute member

```{literalinclude} ../_snippets/how-to/attributes.cpp
:language: cpp
:lines: 1-7
```

The annotation moves `x` to the opening tag. `y` stays a child element. Both members are still ordinary integers in your C++ object.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/attributes.cpp
:language: cpp
:lines: 9-
```

The reader looks for `x` as an attribute and `y` as a child. Their placement comes from the same declaration used by the writer. Attribute order in the input does not need to match C++ declaration order.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<AttributeAndChild x="4"><y>5</y></AttributeAndChild>
```

```text
x: 4; y: 5
```

## Apply this to your schema

Use `attribute` on scalar values. A nested object needs child structure; do not combine an attribute with unpacking, `raw`, or `cdata`.
