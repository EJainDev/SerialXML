# XML input reference

This page specifies the XML syntax and reconstruction rules accepted by
`serial_xml::from_xml`. Function signatures are in the [API reference](api.rst).
For task-oriented examples, see [Deserialize XML](deserialization.md).

## Supported syntax

| Input feature | Behavior |
| --- | --- |
| XML version and encoding | UTF-8 XML 1.0 only. A UTF-8 BOM is accepted. |
| Declaration | Optional; when present, starts with `version="1.0"`. Optional `encoding` must be UTF-8 (case insensitive), followed by optional `standalone="yes"` or `"no"`. |
| Elements | Exactly one root; matching opening/closing names and self-closing elements are accepted. |
| Attributes | Single- or double-quoted values; XML whitespace separates attributes. |
| Comments and processing instructions | Accepted around the root and within elements. Ignored for member reconstruction. The processing-instruction target `xml` is reserved. |
| CDATA | Accepted within elements. Scalar tagged strings concatenate CDATA and ordinary text. |
| Predefined entities | `&amp;`, `&lt;`, `&gt;`, `&quot;`, and `&apos;`. |
| Character references | Decimal `&#65;` and hexadecimal `&#x41;`; decoded code points must be valid XML 1.0 characters. |
| Names | Valid XML 1.0 names, including Unicode, underscores, and literal namespace prefixes. |
| Namespace processing | Names are matched literally. Namespace declarations are not resolved; `a:item` and `b:item` remain different names. |
| Nesting | At most 256 element levels, counting the root as the first level. |
| DTDs and external entities | Unsupported. No external resource is loaded. |

The parser rejects malformed or overlong UTF-8, forbidden XML characters,
mismatched tags, unterminated structures, invalid entities or character references,
malformed comments, multiple roots, and trailing non-miscellaneous content. These
failures throw `serial_xml::deserialization_error`; parser diagnostics include a
byte position.

## Name matching and member selection

The expected root name is selected in this order:

1. A nonempty `fixed_name` argument.
2. A `name` annotation on the selected schema (the target type when no external
   schema is specified).
3. The actual target type's identifier, or its class-template identifier.

A root-name mismatch throws `deserialization_error`. An invalid expected root name
throws `std::invalid_argument`.

Selected fields and getters use their member identifier unless `name` overrides
it. Attributes read an attribute with that name; ordinary members read a child
element. External schemas select direct accessible members by identifier and supply
their annotations. Their placeholder types do not determine the actual conversion
type. Unselected and skipped fields keep their existing or initialized values.

| Member kind | Input behavior |
| --- | --- |
| Assignable data member | Convert its XML value and assign it. |
| Const getter with a matching setter | Convert using the getter's value type and invoke the setter. Getter annotations define the XML representation. |
| Getter without a matching setter | Ignore it during input. It may still participate in output. |
| Method annotated `setter` | Convert using its single parameter type and invoke it. Its own annotations define the XML representation. |
| Nonassignable data member | Compile-time error unless skipped or unselected. |

Automatic setter matching accepts the same member name with one parameter, or the
following naming conventions: `value()` → `set_value(T)`, `get_value()` →
`set_value(T)`, and `getValue()` → `setValue(T)`. A candidate must be accessible,
non-static, non-const, callable on an lvalue, and have exactly one parameter of
the getter's value type after removing references, cv qualifiers, and aliases.
Multiple matching candidates cause a compile-time error.

An explicit `setter` requires an accessible non-static one-parameter method. By
default, `set_value` reads `value`, `setValue` reads `value`, and other method names
are used literally. `name` overrides that default. An explicitly selected setter
takes precedence over automatic reconstruction through its matching getter, so
the method is invoked once. Setter annotations can reside on an external schema;
placeholder methods do not require definitions. Setters do not participate in
serialization.

## Missing and empty values

The explicit `optional` annotation has precedence over the natural absence rules
of `std::optional` and `exclude_on_empty`.

| Member when absent | Value-returning `from_xml<T>(xml)` | In-place `from_xml(object, xml)` |
| --- | --- | --- |
| Ordinary required member | Throws `deserialization_error`. | Throws `deserialization_error`. |
| Member or setter annotated `optional` | Keeps its initializer; setter is not called. | Keeps its current value; setter is not called. |
| `std::optional<T>` without `optional` annotation | Assigns `std::nullopt`. | Assigns `std::nullopt`, replacing any previous value. |
| Automatically iterated range annotated `exclude_on_empty`, without `optional` or explicit `iter` | Assigns a value-initialized range (empty for dynamic ranges). | Assigns a value-initialized range, replacing any previous contents. |
| Skipped field or unselected schema field | Keeps its initialized value. | Keeps its current value. |
| Read-only getter | Ignored. | Ignored. |

An empty element counts as present. It yields an empty string or range, while an
empty numeric element fails conversion. Present fixed-size arrays require the
exact number of elements. An absent `std::optional<T>` differs from a present
empty element: a present value attempts `T`'s conversion and produces an engaged
optional if conversion succeeds.

Raw ranges have no wrapper to require. With no matching item elements they produce
an empty range, or keep their value if annotated `optional`. Empty fixed-size
arrays still fail the size check. The `exclude_on_empty` absence rule applies to
automatically iterated ranges; an explicit `iter` wrapper remains required unless
the member is also annotated `optional`.

## Order, unknown content, and duplicates

* XML attribute and ordinary child-element order is independent of C++ member
  order. Members are assigned or passed to setters in schema order.
* Unknown attributes and child elements of an object are ignored after the entire
  XML document has been parsed and validated.
* Duplicate XML attributes are rejected, including attributes not selected by the
  schema.
* Duplicate matched singleton elements or range wrappers are rejected. Unknown
  repeated child elements are ignored.
* Repeated matching item tags in an iterated range are the range's elements and
  preserve their input order.
* A wrapped range rejects unexpected item tags and non-whitespace direct text.
  A raw range selects its matching item tags from the parent and ignores unrelated
  siblings and direct text.
* Scalar tagged values reject nested child elements. Ordinary text and CDATA within
  a scalar tagged element are concatenated; comments and processing instructions
  do not contribute text.

## Text and attribute normalization

| Source text | Reconstructed text |
| --- | --- |
| Literal CRLF or CR in element text and CDATA | LF. |
| Literal tab, LF, CR, or CRLF in an attribute | Space; CRLF becomes one space. |
| Character references such as `&#13;`, `&#10;`, and `&#9;` | The referenced character, without literal-whitespace normalization. |
| XML entities in ordinary text or attributes | Their decoded characters. |
| Entity-looking text within CDATA | Literal text; entities are not decoded. |

Strings retain whitespace after XML normalization. Arithmetic and boolean
conversion trims surrounding spaces, tabs, carriage returns, and line feeds.
Pretty printing can introduce additional direct text between child elements; use
compact `to_xml` output when round-tripping raw scalar fields or when every text
node matters.

On output, SerialXML rejects forbidden XML 1.0 characters and malformed UTF-8,
including custom formatter output, with `std::invalid_argument`. Text carriage
returns and attribute tabs, newlines, and carriage returns are written as character
references to preserve their values. Annotation names are validated at compile
time; invalid runtime root overrides throw `std::invalid_argument`.

## Leaf conversion representations

Non-unpacked leaves use `serial_xml::from_string<T>(std::string_view)` after XML
decoding. Calling `from_string` directly does not parse XML or decode entities.

| Type | Accepted representation |
| --- | --- |
| `std::string` | Complete decoded text, retaining whitespace. |
| `bool` | `true`, `false`, `1`, or `0` after whitespace trimming. |
| Arithmetic types | A complete `std::from_chars` representation after trimming. Empty, invalid, trailing, or out-of-range input fails. |
| `char` | Numeric character value for a scalar leaf, matching `to_xml`. Quoted character syntax is also handled inside formatted containers. |
| `std::optional<T>` | A present textual value converts to `T` and creates an engaged optional. |
| Pair or tuple | `(value, value)` with the exact tuple arity. |
| Owning sequence | `[value, value]`. |
| Owning set | `{value, value}`. |
| Owning map | `{key: value, key: value}`. |

Container representations support nested containers and quoted strings, including
escaped quotes, backslashes, newline/tab/carriage-return escapes, and braced Unicode
escapes. Fixed arrays require exactly their size; fixed-capacity containers reject
too many items. Writable forward lists and valarrays are supported.

XML item iteration is automatic for `vector`, `array`, `inplace_vector`, `deque`,
`forward_list`, and `valarray`. `iter` selects item iteration for other owning
ranges; `no_iter` selects formatted container text. Borrowed views such as `span`,
`string_view`, and pointers have no built-in reconstruction. A custom implementation
must provide an appropriate storage lifetime.

Custom leaf types require an explicit `from_string<T>` specialization declared
before the first call that needs it. Unpacked classes use reflection instead of
leaf conversion.

`format` controls output representation; deserialization does not invert a custom
formatter or lossy format specifier. Decimal zero padding works with built-in
numeric conversion. Prefixes, hexadecimal output, alignment fill, and other
representations require matching conversion. Lost precision cannot be recovered.
See [custom leaf conversion](deserialization.md) for an implementation example.

## Raw text and CDATA

Raw scalar fields consume the parent's direct ordinary text, excluding CDATA.
More than one selected raw scalar field cannot be separated and throws
`deserialization_error`. Raw ranges consume matching item tags directly from the
parent. Unpacked objects and automatically handled optionals retain tagged
representations even with `raw`, following serialization precedence.

CDATA fields consume the parent's CDATA sections in member order. Default
arithmetic output uses tagged numeric elements even with `cdata`, and input follows
that representation. The writer splits `]]>` across CDATA sections and emits
carriage returns as character references; the reader rejoins those canonical
continuations.

Adjacent fields can be ambiguous when their boundary resembles a canonical split:
one value ends in `]]` and the next starts in `>`. Optional CDATA fields and
externally split CDATA can also make boundaries ambiguous. Tagged string fields
provide an unambiguous boundary and can contain both ordinary text and CDATA.

## Failure and update behavior

Malformed XML, missing required input, duplicate selected elements, and built-in
conversion failures throw `serial_xml::deserialization_error`. Exceptions from
custom conversions, assignment operators, or setters propagate.

The value-returning overload starts from `T{}`. The in-place overload first copies
the input so a field that owns the XML can safely be replaced. Both parse the
complete XML document before updating members. Conversion, assignment, and setter
execution happen afterward in schema order; a failure can leave an existing
object partially updated. No rollback is performed.
