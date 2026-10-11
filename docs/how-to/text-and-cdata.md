(example-raw-cdata)=
# Write direct text or CDATA

Place string content directly inside its parent element instead of giving the member a child wrapper.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Choose how direct text is represented

```{literalinclude} ../_snippets/how-to/text-and-cdata.cpp
:language: cpp
:lines: 1-10
```

`raw` removes the member’s wrapper but still escapes XML characters. `cdata` writes a CDATA section directly into the parent. Neither requires callers to prepare escaped input.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/text-and-cdata.cpp
:language: cpp
:lines: 12-
```

The raw document contains entity references where needed. The CDATA document keeps markup characters inside the section. Reading either representation reconstructs the original string.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<RawExample>Hello &lt;world&gt; &amp; friends</RawExample>
<CDataExample><![CDATA[text<empty> & stuff]]></CDataExample>
```

```text
Raw value: Hello <world> & friends
CDATA value: text<empty> & stuff
```

## Apply this to your schema

Use compact XML for mixed-content round trips: indentation can become part of direct text. Multiple raw scalar fields cannot be separated on input, and CDATA fields are read in member order. Prefer tagged strings when field boundaries need to be explicit.
