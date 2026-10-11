(example-stl-containers)=
# Choose a container representation

Choose repeated elements for structured XML, formatted text for a leaf value, or omission for an empty sequence.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](../installation.md) if it is not configured yet. Save the following
portions in `main.cpp`, in the order shown.

## Define the three representations

```{literalinclude} ../_snippets/how-to/containers.cpp
:language: cpp
:lines: 1-14
```

A plain vector iterates automatically. `exclude_on_empty` suppresses its wrapper when empty. `no_iter` turns the entire vector into one formatted text value.

## Write and read the XML

Add this complete `main` function below the declarations:

```{literalinclude} ../_snippets/how-to/containers.cpp
:language: cpp
:lines: 16-
```

The first document contains three `element` children. The second retains an empty wrapper; the third omits it. The reader reconstructs repeated items from elements and the formatted representation through `from_string`.

## Build and check the result

From your application directory, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The program prints:

```xml
<VectorExample><values><element>1</element><element>2</element><element>3</element></values></VectorExample>
<VectorExample><values/></VectorExample>
<ExcludeOnEmpty/>
<NoIter><values>[1, 2, 3]</values></NoIter>
```

```text
Items: 3; text items: 3; omitted items: 0
```

## Apply this to your schema

An absent automatically iterated range marked `exclude_on_empty` becomes empty. Add `optional` if an absent input field should retain an initializer or existing value instead. Use owning containers when the reader needs to reconstruct storage.
