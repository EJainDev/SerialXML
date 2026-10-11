# Read XML into objects

Construct a new object from XML, update an existing object, or load private state
through a setter. The programs below show each task in full, with their imports,
types, and application bodies.

Use a CMake application linked to `serial_xml::serial_xml`; see
[installation](installation.md) if needed. Save one program at a time as `main.cpp`
and build it with:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

## Define the input record

Start the first program with these imports and declarations:

```{literalinclude} _snippets/how-to/read-objects.cpp
:language: cpp
:lines: 1-8
```

The reader expects `age` as an attribute and `name` as a child. The nickname can
be omitted because it carries the `optional` annotation.

## Update an existing object

Append this complete application body:

```{literalinclude} _snippets/how-to/read-objects.cpp
:language: cpp
:lines: 10-
```

The first `from_xml<Person>` constructs a new, value-initialized object. The
in-place call instead takes `person` first and updates it. This form also supports
types without a default constructor. The last read constructs a separate value
before replacing the destination.

## Retain defaults when a field is missing

The program prints:

```text
Alex: age 21; nickname unknown
Sam: age 22; nickname Ace
Replaced: Alex; nickname unknown
```

An absent annotated `optional` field retains its initializer in a new object or
its current value in an existing object. A plain `std::optional<T>` instead resets
to `nullopt` when absent. An empty element is present: it can supply an empty
string, but not a valid integer.

The in-place reader snapshots and parses input before changing members. A later
conversion or setter failure can still leave earlier members updated. When you
need to defer replacement until conversion succeeds, read into a temporary as
shown above. The final assignment follows your type’s own exception guarantees.

## Convert a custom leaf type

Built-in leaf conversions cover arithmetic values, owning strings, and supported
owning containers and tuples. When your text has a custom prefix or other
representation, supply a `from_string` specialization before its first use and
pair it with an output formatter. The [custom-conversion guide](how-to/custom-conversion.md)
shows the full program, including validation and error handling.

A `format` annotation controls output text; the reader cannot infer its inverse.
Borrowed views such as `string_view` and `span` have no built-in reconstruction
because they do not own storage.

## Read through setters

For the next program, replace `main.cpp` with these declarations and the
application body below them:

```{literalinclude} _snippets/how-to/read-setters.cpp
:language: cpp
:lines: 1-13
```

`setter` selects the accessible one-parameter method for input. The explicit
`name` tells it to read the `balance` child. `skip` keeps the getter out of the
mapping so the selected setter handles that input directly.

```{literalinclude} _snippets/how-to/read-setters.cpp
:language: cpp
:lines: 15-
```

After building and running, the program prints:

```text
Balance: 125
```

You can also rely on automatic getter/setter pairing: `value()` pairs with
`value(T)` or `set_value(T)`, `get_value()` with `set_value(T)`, and `getValue()`
with `setValue(T)`. The value types must match, ignoring references and cv
qualifiers. Getters without a matching setter are ignored during input. Explicitly
selected setters take precedence over automatic pairing and are ignored by
`to_xml`.

## Diagnose a read failure

For the final program, define a record with a required numeric child:

```{literalinclude} _snippets/how-to/read-errors.cpp
:language: cpp
:lines: 1-6
```

Then append the application body:

```{literalinclude} _snippets/how-to/read-errors.cpp
:language: cpp
:lines: 8-
```

The XML is well formed, but its value cannot be converted to an integer. Catching
`deserialization_error` lets the application report the diagnostic:

```text
Read failed
Invalid or out-of-range numeric value: not-a-number
```

The same exception covers malformed XML, mismatched roots, missing required
fields, and failed built-in conversions. Invalid runtime root names throw
`std::invalid_argument`. Exceptions from custom converters and setters propagate
to the caller.
