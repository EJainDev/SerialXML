# How SerialXML maps C++ to XML

SerialXML uses C++26 reflection to discover accessible members at compile time.
The resulting mapping connects an object's type to its XML shape. This removes
the need to write a separate serializer for each ordinary struct.

## Types describe structure; annotations describe intent

The default mapping uses a type's identifier as the root name and member
identifiers as element names. Accessible const getters can expose values too.
Attributes, skipped fields, and renamed tags are deliberate schema choices, so
annotations attach those choices directly to declarations.

Classes that cannot be formatted are unpacked into members. A formattable value
can instead become leaf text. `unpack` and `no_unpack` let a schema make this
choice explicitly. This distinction matters for custom types: structured objects
are reconstructed through reflection, while leaves are reconstructed through
`from_string`.

## Structure and textual formatting are different contracts

Many common sequences are automatically iterated into item elements. Other
containers may appear as their standard formatted text unless iteration is
requested. `no_iter` chooses text for an automatically handled range; `iter`
chooses XML elements and can name each item and its wrapper.

These two representations need different readers. Iterated XML reconstructs
items from elements. Text representations use `from_string`, including its
container and tuple parsers. Borrowed views do not own storage, so they have no
built-in reconstruction.

A custom `format` annotation changes a textual representation. It does not create
an inverse conversion: a custom prefix or hexadecimal format needs a matching
`from_string` specialization, and rounded-away precision cannot be restored.
Use the [annotation reference](annotations.md) and [XML input reference](xml-input.md)
for the precise rules and precedence.

## Missing is different from empty

A missing field can mean an incomplete document, an omitted optional value, or an
intentional choice to retain a default. SerialXML therefore distinguishes required
members, `std::optional`, and the `optional` annotation.

Required members enforce the expected schema. An absent `std::optional` resets to
`nullopt`. An annotated optional member preserves its initializer or the existing
value in an in-place update. An empty element is still present, so it is suitable
for an empty string but cannot be interpreted as a number.

## External schemas adapt existing classes

A mock schema declares the accessible field and getter identifiers to expose,
plus their annotations and output order. It is never constructed; values come
from the real object. This is useful when you cannot modify the original type or
want a particular XML representation for it.

The schema's annotations replace those on the selected target members. Nested
objects use their own annotations. Mapping uses directly accessible members;
inherited members are not included. Compile-time validation catches missing,
ambiguous, or incompatible mappings before a document is written.

## Formatting a document can change its whitespace

Whitespace can be data, particularly with direct text and CDATA. See
[whitespace and round trips](explanation/round-trips.md#formatting-a-document-can-change-its-whitespace)
for the tradeoff between readable output and faithful reconstruction.

## Reading into an object is an update

Conversion and setter failures can leave earlier members updated. See
[update behavior](explanation/round-trips.md#reading-into-an-object-is-an-update)
for the implications and the temporary-object approach.

SerialXML validates its XML subset and matches namespace prefixes literally;
it does not resolve XML namespaces or process DTDs. It is a type-to-document
mapper rather than an XML schema validator. See the [API reference](api.rst) for
entry points and the [read guide](deserialization.md) for practical usage.
