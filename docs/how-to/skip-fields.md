(example-skip-members)=
# Leave a field out of the schema

Exclude internal state from XML while continuing to store it in your C++ object.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Mark the internal member

```{literalinclude} ../_snippets/how-to/skip-fields.cpp
:language: cpp
:lines: 1-7
```

`skip` excludes `cached_value` from both writing and reading. Its initializer gives a newly constructed record a defined cache value.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/skip-fields.cpp
:language: cpp
:lines: 9-
```

Although `original.cached_value` is 100, it contributes no XML. A new object retains the initializer, while reading into an existing object retains that object’s current cache.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<Record><value>42</value></Record>
```

```text
New object: value 42; cache -1
Updated object: cache 250
```

## Apply this to your schema

Use `skip` when the member is outside the XML schema. It is different from allowing an input field to be missing: an optional field still participates when present.
