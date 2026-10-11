# SerialXML

```{image} ../assets/SerialXML.png
:alt: SerialXML — Modern C++ XML Serialization, with C++26 reflection and native modules
:class: serialxml-banner
```

<div class="hero-kicker">C++26 · XML · Reflection</div>

## Your types. Your XML.

Write ordinary C++ objects as XML and read them back. Start with reflection's
default mapping, then use annotations to match the schema your application needs.

<div class="hero-actions">
<a class="primary-action" href="quickstart.html">Build your first round trip →</a>
<a class="secondary-action" href="api.html">Look up the API</a>
</div>

```cpp
import std;
import serial_xml;

struct Person {
  [[= serial_xml::attribute]] int age;
  std::string name;
};

const auto xml = serial_xml::to_xml(Person{21, "Alex"}, false);
// <Person age="21"><name>Alex</name></Person>
const auto person = serial_xml::from_xml<Person>(xml);
```

## What do you want to do?

<div class="doc-grid">
<a class="doc-card" href="tutorials/index.html"><span class="card-kind">Learn</span><strong>Tutorials</strong><span>Build a working application, then shape its XML. Follow a guided path with expected output at each step.</span><span class="card-link">Start learning →</span></a>
<a class="doc-card" href="how-to/index.html"><span class="card-kind">Solve a task</span><strong>How-to guides</strong><span>Install the library, match an existing schema, read private state, or customize a conversion.</span><span class="card-link">Find your task →</span></a>
<a class="doc-card" href="reference/index.html"><span class="card-kind">Look it up</span><strong>Reference</strong><span>Find signatures, annotation rules, supported XML syntax, and failure behavior.</span><span class="card-link">Check the details →</span></a>
<a class="doc-card" href="explanation/index.html"><span class="card-kind">Understand</span><strong>Explanation</strong><span>Explore the mapping model and the tradeoffs behind structure, text, defaults, and round trips.</span><span class="card-link">Explore the model →</span></a>
</div>

## Common destinations

| I need to… | Go to |
| --- | --- |
| Add SerialXML to my CMake project | [Installation](installation.md) |
| Rename a tag or make it an attribute | [XML naming](how-to/naming.md) · [Attributes](how-to/attributes.md) |
| Serialize a class I cannot change | [External schemas](how-to/external-schema.md) |
| Keep defaults when input fields are missing | [Read XML](deserialization.md#retain-defaults-when-a-field-is-missing) |
| Understand why reading XML fails | [Diagnose a read failure](deserialization.md#diagnose-a-read-failure) |
| Find a function or annotation | [API reference](api.rst) · [Annotation reference](annotations.md) |

```{note}
**Toolchain:** GCC 16.1+ with C++26 reflection, CMake 4.3.3+, and Ninja.
The repository's dev container supplies a matching environment.
See [installation](installation.md) before building your application.
```

```{toctree}
:hidden:
:maxdepth: 2

tutorials/index
how-to/index
reference/index
explanation/index
project/index
```

[Source code](https://github.com/EJainDev/SerialXML) ·
[Report an issue](https://github.com/EJainDev/SerialXML/issues) ·
[Contribute](contributing.md)
