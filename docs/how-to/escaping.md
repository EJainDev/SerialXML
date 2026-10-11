(example-escaping)=
# Preserve XML-special characters

Pass ordinary strings to SerialXML and let it escape the characters that have meaning in XML.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Define the three text contexts

```{literalinclude} ../_snippets/how-to/escaping.cpp
:language: cpp
:lines: 1-14
```

The plain member becomes a child, the attribute belongs on the opening tag, and the raw member contributes direct text. Each representation still requires valid XML text.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/escaping.cpp
:language: cpp
:lines: 16-
```

The original string contains all five XML-special characters. Each output escapes them, and the reader decodes the child document back into the original string.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<EscapeChild><text>&lt;&gt;&amp;&apos;&quot;</text></EscapeChild>
<EscapeAttribute text="&lt;&gt;&amp;&apos;&quot;"/>
<EscapeRaw>&lt;&gt;&amp;&apos;&quot;</EscapeRaw>
```

```text
Restored text: <>&'"
```

## Apply this to your schema

Do not pre-escape strings: an existing ampersand in `&amp;` is itself escaped. `raw` removes enclosing tags, not escaping. Malformed UTF-8 and invalid XML 1.0 characters cause serialization to throw `std::invalid_argument`.
