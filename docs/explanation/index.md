# Explanation

Understand why SerialXML maps a type to a particular document and what makes a
round trip reliable. These pages discuss the model and its tradeoffs; use the
[how-to guides](../how-to/index.md) when you need implementation steps.

## The mapping model

[How SerialXML maps C++ to XML](../design.md) connects reflection, annotations,
external schemas, and conversion. Read it to understand how your C++ declarations
determine the document's structure.

## Round-trip tradeoffs

[Designing reliable round trips](round-trips.md) discusses optionality, textual
formatting, ownership, whitespace, and update failures. Read it when choosing a
schema that needs to be both written and read.

```{toctree}
:hidden:
:maxdepth: 1

../design
round-trips
```
