# Create a custom product code

Give a C++ value the XML representation `SKU-42`, then restore its integer and
reject a code with the wrong prefix. You will build both sides of a custom leaf
conversion rather than asking the reader to guess an inverse format.

Complete [Load and update application settings](settings.md) first. Keep the
same application configuration and replace `main.cpp` for this project.

## 1. Write a code with a prefix

Start with this complete program:

```{literalinclude} ../_snippets/tutorials/product-code-1.cpp
:language: cpp
```

Build and run:

```bash
cmake --build build
./build/my_app
```

Expected output:

```xml
<Product id="7"><code>SKU-42</code></Product>
```

`ProductCode` stores an integer. The formatter returns an owning string with the
prefix, and the annotation on `Product::code` chooses that text representation.
This first program only writes the document.

## 2. Teach the reader to reverse the format

Insert this specialization after `format_code`, before the `Product` declaration:

```cpp
template <>
ProductCode serial_xml::from_string<ProductCode>(std::string_view text) {
  if (!text.starts_with("SKU-")) {
    throw serial_xml::deserialization_error("Expected SKU- code");
  }
  return {serial_xml::from_string<int>(text.substr(4))};
}
```

The input conversion checks the prefix and passes the remaining decimal text to
the built-in integer conversion. It must be declared before the first call that
needs it. It receives decoded leaf text, so XML entity handling is already done.

## 3. Read a valid code and reject an invalid one

Replace `main` with:

```{literalinclude} ../_snippets/tutorials/product-code.cpp
:language: cpp
:lines: 23-
```

Build and run again. The output, in order, is:

```xml
<Product id="7"><code>SKU-42</code></Product>
```

```text
Restored product: 7; code: 42
Rejected input: Expected SKU- code
```

The successful read recovers the integer 42. The second document contains
`OTHER-42`, so your specialization rejects it with the diagnostic you supplied.

## Completed program

```{literalinclude} ../_snippets/tutorials/product-code.cpp
:language: cpp
```

Change the prefix in the formatter and reader to `ITEM-`, update the invalid-input
check if needed, and rebuild. Verify that the new XML representation still reads
back to the same integer. This exercise keeps the two conversion directions in
agreement.

You have completed the learning path: ordinary records, annotations, nested
collections, input defaults, and custom text conversions. Use the
[how-to guides](../how-to/index.md) when applying these techniques to your own schema.
