# Load and update application settings

Build a settings reader that accepts a missing theme, retains an existing theme
during an update, and reports invalid numeric input. You will finish by saving
the current settings back to XML.

Complete [Build a book catalogue](catalog.md) first, or use an application prepared
in the first tutorial. Keep its `CMakeLists.txt` and replace `main.cpp` as shown.

## 1. Load a complete document

Start with this program:

```{literalinclude} ../_snippets/tutorials/settings-1.cpp
:language: cpp
```

Build and run from your application directory:

```bash
cmake --build build
./build/my_app
```

Expected output:

```text
Volume: 20; theme: dark
```

The root name is `settings`. Both children are present, so they replace the
initial values. The `theme` initializer alone does not make that input field
optional.

## 2. Keep a default when the theme is missing

Add `optional` to the theme member:

```cpp
[[= serial_xml::optional]] std::string theme = "light";
```

Replace `main` with:

```{literalinclude} ../_snippets/tutorials/settings-2.cpp
:language: cpp
:lines: 9-
```

Build and run again:

```text
Initial: volume 20; theme light
Updated: volume 35; theme dark
```

The first document omits `theme`, so the new object keeps `light`. You then set
the theme to `dark`. The in-place update changes the volume but retains that
current theme because the second document also omits it.

## 3. Reject invalid input and save your settings

Replace `main` with the following version:

```{literalinclude} ../_snippets/tutorials/settings.cpp
:language: cpp
:lines: 9-
```

This reads the invalid document into a separate new object. Its failed conversion
does not update `preferences`. The final call writes the current settings.

Build and run. The output, in order, is:

```text
Initial: volume 20; theme light
Updated: volume 35; theme dark
Rejected input: Invalid or out-of-range numeric value: loud
```

```xml
<settings><volume>35</volume><theme>dark</theme></settings>
```

The XML contains both values. The `optional` annotation permits omission on
input; it does not omit an initialized string when writing.

## Completed program

```{literalinclude} ../_snippets/tutorials/settings.cpp
:language: cpp
```

Try changing `loud` to `60`. The valid read will print that volume, while the
saved `preferences` will still have volume 35 because the new object was separate.
You have learned to distinguish loading a new value from updating existing state.

Continue with [Create a custom product code](product-code.md) to define your own
text conversion instead of using a built-in numeric reader.
