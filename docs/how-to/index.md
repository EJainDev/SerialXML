# How-to guides

Choose the result you need. These guides assume you understand C++ types and have
completed the [first round trip](../quickstart.md) or already use SerialXML.
Each recipe presents a complete program in portions, explains the schema choices,
and includes build commands and expected output.

## Set up and run

- [Install SerialXML in a CMake project](../installation.md).
- [Run the example programs](../examples.md#run-the-accompanying-examples).
- [Run the benchmarks](../benchmarking.md).

## Write and shape XML

- [Write a document, fragment, or named root](../serialization.md).
- [Serialize a simple record](simple-record.md).
- [Put a value in an attribute](attributes.md).
- [Match an XML naming scheme](naming.md).
- [Leave a field out of the schema](skip-fields.md).
- [Serialize nested objects](nested-objects.md).
- [Adapt a class with an external schema](external-schema.md).
- [Combine nesting and annotations](combined-schema.md).

## Represent collections and text

- [Choose a container representation](containers.md).
- [Choose item and container names](item-names.md).
- [Omit an absent optional value](optional-values.md).
- [Write direct text or CDATA](text-and-cdata.md).
- [Format a numeric value](numeric-formatting.md).
- [Preserve XML-special characters](escaping.md).

## Read and update objects

- [Read XML into a new or existing object](../deserialization.md).
- [Retain defaults for missing fields](../deserialization.md#retain-defaults-when-a-field-is-missing).
- [Round-trip a custom leaf representation](custom-conversion.md).
- [Read private state through setters](../deserialization.md#read-through-setters).
- [Diagnose a read failure](../deserialization.md#diagnose-a-read-failure).

Need an exact rule rather than a procedure? Open the [reference](../reference/index.md).

```{toctree}
:hidden:
:maxdepth: 1

../installation
../serialization
simple-record
attributes
naming
skip-fields
nested-objects
external-schema
combined-schema
containers
item-names
optional-values
text-and-cdata
numeric-formatting
escaping
../deserialization
custom-conversion
../examples
../benchmarking
```
