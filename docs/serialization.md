# Write XML and adapt schemas

Choose whether to write a complete document, omit the declaration, or override the
root name for a particular destination. This application demonstrates all three
with the same record, then reads the named-root document back.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](installation.md) if needed. Place these portions in `main.cpp`.

## Define the record

```{literalinclude} _snippets/how-to/write-documents.cpp
:language: cpp
:lines: 1-6
```

The type annotation gives the default root the name `record`. The member `value`
becomes a child with that name.

## Write a document, a fragment, or a named root

Add this complete application body below the declaration:

```{literalinclude} _snippets/how-to/write-documents.cpp
:language: cpp
:lines: 8-
```

Calling `to_xml(record)` includes the UTF-8 XML declaration. Passing `false`
omits it. The third argument selects the root name at runtime and takes
precedence over the type’s `name` annotation. Pass that same root override to
`from_xml` when reading the document.

## Format XML for display

The `prettify(named)` call prints an indented version without changing `named`.
Keep the compact string for storage or parsing, especially if your schema has
raw text beside child elements. Text-bearing subtrees, CDATA, and subtrees with
`xml:space` retain their original bytes during pretty printing.

## Build and run

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The output is:

```text
Document:
```

```xml
<?xml version="1.0" encoding="UTF-8"?><record><value>42</value></record>
```

```text
Fragment:
```

```xml
<record><value>42</value></record>
```

```text
Named root:
```

```xml
<entry><value>42</value></entry>
```

```text
For display:
```

```xml
<entry>
  <value>42</value>
</entry>
```

```text
Restored value: 42
```

The root override must be a valid XML 1.0 name. An invalid override throws
`std::invalid_argument`; malformed input to `prettify` throws
`serial_xml::deserialization_error`.

## Annotate a class you cannot modify

For an existing type whose declarations you cannot edit, select an external schema
as the template argument to `to_xml`. The [external-schema guide](how-to/external-schema.md)
provides the complete declarations, application body, and output for that task.
