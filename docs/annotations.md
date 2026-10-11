# Annotation reference

This library is annotation driven. Most customization points are exposed via
C++26 annotations, so the XML structure stays visually associated with the C++
definition.

By default, members become child elements named after the member. Classes that
cannot be formatted are unpacked into their own members. Many STL ranges and
`std::optional` are handled automatically; an empty `std::optional` is omitted.

For example, here's how to make `age` an attribute and rename `favorite_food`:

```cpp
struct Person {
  [[= serial_xml::attribute]] int age;
  [[= serial_xml::name{"food"}]] std::string favorite_food;
};

const auto xml = serial_xml::to_xml(Person{3, "pizza"}, false);
// <Person age="3"><food>pizza</food></Person>
```

The annotations below live in the `serial_xml` namespace. Use the qualified name,
as above, or bring the annotations into scope.

| Annotation | What it does | Details |
| --- | --- | --- |
| `[[=attribute]]` | Emit a member as an XML attribute. | Uses the member name unless overridden by `name`. |
| `[[=name{"custom_name"}]]` | Rename an attribute or element. | Also works on a class or struct to rename its root element. |
| `[[=skip]]` | Omit a member. | Ignored by serialization and deserialization. |
| `[[=raw]]` | Emit text without the member's enclosing tag. | For ranges, removes the outer range tag, not the item tags. Text is still XML-escaped. |
| `[[=cdata]]` | Emit the value in a CDATA section. | Produces `<![CDATA[your_content]]>`. |
| `[[=unpack]]` | Serialize an object's members inside an enclosing element. | Uses reflection instead of `std::format`. |
| `[[=no_unpack]]` | Format an object as text. | Disables unpacking. |
| `[[=iter{a, b}]]` | Iterate a range instead of formatting it as text. | Optional `a` names each item; optional `b` names the enclosing range tag. |
| `[[=no_iter]]` | Disable automatic range iteration. | Uses the range's formatted representation. |
| `[[=exclude_on_empty]]` | Omit tags for an empty range. | See the precedence rules below. |
| `[[=format{"format_specifier"}]]` | Pass a format specifier to `std::format`. | Leave out the leading `:`; SerialXML adds it. |
| `[[=format{format_function}]]` | Use a custom formatting function. | Accepts the member value and returns a string-like value. |
| `[[=optional]]` | Allow a field or selected setter to be absent when reading. | An absent member keeps its initializer or current value. |
| `[[=setter]]` | Select a one-parameter method for XML input. | Ignored by `to_xml`; see [Setters and encapsulation](deserialization.md#read-through-setters). |

## Precedence and automatic handling

Some STL containers are automatically iterated. If you want their formatted text
instead, add `[[=serial_xml::no_iter]]` to the member. The automatically iterated
containers are `std::vector`, `std::array`, `std::inplace_vector`, `std::deque`,
`std::forward_list`, `std::span`, and `std::valarray`.

When annotations overlap, precedence matters:

| Context | Precedence (highest first) |
| --- | --- |
| Automatically handled STL ranges | `exclude_on_empty` → `raw` → `cdata` |
| Child members | STL handling → `raw` → iteration → unpacking → `cdata` |

For ranges, `raw` only removes the outer layer of tags. Individual elements keep
their tags. `format` is ignored for unpacked or iterated members.

## Recipes by annotation

| Annotation | Task guide |
| --- | --- |
| `attribute` | [Put a value in an attribute](how-to/attributes.md) |
| `name` | [Match an XML naming scheme](how-to/naming.md) |
| `skip` | [Leave a field out of the schema](how-to/skip-fields.md) |
| `iter`, `no_iter`, `exclude_on_empty` | [Choose a container representation](how-to/containers.md) · [Name items and wrappers](how-to/item-names.md) |
| `raw`, `cdata` | [Write direct text or CDATA](how-to/text-and-cdata.md) |
| `format` | [Format numbers](how-to/numeric-formatting.md) · [Round-trip custom text](how-to/custom-conversion.md) |
| `optional` | [Retain defaults for missing fields](deserialization.md#retain-defaults-when-a-field-is-missing) |
| `setter` | [Read private state through setters](deserialization.md#read-through-setters) |

For constructors and exported marker declarations, see the [API reference](api.rst).
