# Designing reliable round trips

A document that can be written is not always enough to reconstruct the original
object. A reliable round trip needs both a structural mapping and a reversible
text representation. The [mapping model](../design.md) explains the structure;
this page discusses the choices that can lose information or change state.

## Text needs a reversible conversion

A `format` annotation determines output text. The reader uses `from_string` and
cannot infer the inverse of an arbitrary formatter. Zero-padded decimal numbers
remain readable by the default numeric parser; a custom prefix or hexadecimal
representation needs a matching conversion. Rounded precision is already lost
before XML is written.

The same distinction applies to containers: repeated XML items encode structure,
while a formatted container is one leaf value. The representation determines
which input conversion runs.

## Absence expresses a policy

A required field rejects incomplete input. An absent `std::optional` resets to
`nullopt`. An `optional` annotation preserves an initializer or the current value.
These policies answer different questions: whether a document must supply a
value, whether a value exists, and whether an update should retain old state.
An empty element is still present; it can represent an empty string but cannot
supply a valid number.

## Reconstructed values need storage

Owning strings and containers can retain decoded input. Borrowed views such as
`std::string_view` and `std::span` do not allocate storage, so they have no built-in
reconstruction. A custom conversion must give its result an appropriate lifetime.
The in-place reader's input snapshot protects parsing from field updates; it does
not give borrowed outputs permanent storage.

## Formatting a document can change its whitespace

Pretty printing adds indentation between elements. Text-bearing subtrees retain
their bytes because whitespace can be part of a string or mixed content. CDATA
and `xml:space` also require preservation.

For raw scalar fields beside child elements, the parent's direct text is data.
Added indentation can become part of that data when read back. Keep compact
`to_xml` output as your stored representation when whitespace matters, and use
`prettify` for display.

CDATA is another representation with boundaries that can be ambiguous between
adjacent fields. Tagged string members provide explicit boundaries and are easier
to reconstruct reliably. The [XML input reference](../xml-input.md) describes the
canonical split behavior and supported XML subset.

## Reading into an object is an update

The value-returning `from_xml<T>` initializes an object and fills its members.
The in-place overload updates an existing object, which also allows types without
a default constructor. It snapshots the input before parsing so changing a field
that owns the input cannot invalidate the parser's text.

Parsing completes before assignments begin. Conversion and setter failures can
still happen during assignment, leaving earlier members updated. When application
logic needs all-or-nothing replacement, deserialize into a separate value and
replace the destination after successful conversion.

## Choose the contract before the representation

Tagged strings provide explicit boundaries. Compact XML preserves mixed-content
whitespace. A reversible leaf format preserves values. A separate temporary
object allows replacement only after a successful read. These choices follow
from the data your application needs to retain.

See the [XML input reference](../xml-input.md) for exact guarantees, or
[read XML into objects](../deserialization.md) to apply these choices.
