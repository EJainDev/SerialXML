API reference
=============

Import the C++26 module with ``import serial_xml;``. All exported symbols live in
the ``serial_xml`` namespace. This reference describes the public declarations in
``include/serial_xml.cxx``; reflection helpers and parser internals are not public API.

For procedures, use :doc:`serialization` and :doc:`deserialization`. For the
complete annotation table and precedence, use :doc:`annotations`.

.. list-table:: Entry points
   :header-rows: 1
   :widths: 45 55

   * - Task
     - Symbol
   * - Write a document or fragment
     - :cpp:func:`serial_xml::to_xml`
   * - Read a new value or update an existing object
     - :cpp:func:`serial_xml::from_xml`
   * - Convert leaf text or provide a custom conversion
     - :cpp:func:`serial_xml::from_string`
   * - Format XML for display
     - :cpp:func:`serial_xml::prettify`
   * - Catch a built-in read failure
     - :cpp:class:`serial_xml::deserialization_error`

.. raw:: html

   <form class="api-search" role="search" action="search.html" method="get">
     <label for="api-search-input">Search API reference</label>
     <input id="api-search-input" name="q" type="search" placeholder="Find a symbol, e.g. from_xml or optional" autocomplete="off" aria-controls="api-search-results" aria-describedby="api-search-status">
     <button type="submit">Search</button>
     <p id="api-search-status" role="status" aria-live="polite">Type to find matching API declarations.</p>
     <ul id="api-search-results" hidden></ul>
   </form>

.. contents:: Table of contents
   :local:
   :depth: 1
   :class: this-will-duplicate-information-and-it-is-still-useful-here

.. cpp:namespace:: serial_xml

Serialization
-------------

.. cpp:function:: template <typename Schema = void, typename T> std::string to_xml(const T& value, bool first = true, const std::string& fixed_name = "")

   Serialize a class instance into compact UTF-8 XML.

   ``T`` must be a class type; ``Schema`` must be ``void`` or a class type.
   With ``Schema = void``, reflection uses ``T`` and its annotations. An external
   schema selects and annotates accessible fields and const getters by identifier.
   The schema itself is never constructed. Nested objects use their own schemas.

   :param value: Object to read. Serialization does not modify it.
   :param first: Include ``<?xml version="1.0" encoding="UTF-8"?>`` when true.
   :param fixed_name: Explicit root element name. An empty string uses a schema
      ``name`` annotation, then the actual type's identifier.
   :returns: The XML document or fragment.
   :throws std\:\:invalid_argument: An invalid XML name, XML character, or UTF-8
      sequence is encountered.

   Invalid annotation combinations and unresolved or ambiguous schema members are
   diagnosed at compile time. Formatting exceptions and exceptions from getters or
   custom formatter functions propagate to the caller.

   .. code-block:: cpp

      auto document = serial_xml::to_xml(person);
      auto fragment = serial_xml::to_xml(person, false, "person");
      auto custom = serial_xml::to_xml<PersonSchema>(person);

Deserialization
---------------

.. cpp:function:: template <typename T, typename Schema = void> T from_xml(std::string_view xml, const std::string& fixed_name = "")

   Value-initialize ``T``, read XML into it, and return the result.

   ``T`` must be a default-initializable class type. With ``Schema = void``,
   reflection uses ``T``; otherwise an external schema selects input members and
   provides their annotations. The expected root name follows the same rules as
   :cpp:func:`to_xml`.

   :param xml: Complete UTF-8 XML input, with or without a declaration.
   :param fixed_name: Override the expected root name.
   :returns: A populated instance of ``T``.
   :throws deserialization_error: Malformed XML, a mismatched root, missing required
      members, duplicate selected elements, or invalid built-in conversions.
   :throws std\:\:invalid_argument: The expected root name is not a valid XML name.

   Exceptions from custom conversions and setters propagate. Read-only getters are
   ignored; writable fields and getters with matching setters receive values.
   Extra object elements and attributes are ignored; iterated range contents are
   checked against the item name.

   .. code-block:: cpp

      auto person = serial_xml::from_xml<Person>(xml);
      auto custom = serial_xml::from_xml<Person, PersonSchema>(xml, "person");

.. cpp:function:: template <typename Schema = void, typename T> void from_xml(T& result, std::string_view xml, const std::string& fixed_name = "")

   Read XML into an existing instance. ``T`` is deduced, so an explicit template
   argument selects the schema: ``from_xml<PersonSchema>(person, xml)``.

   ``T`` must be a class type and ``T&`` must not be convertible to
   ``std::string_view``. Unlike the value-returning overload, this overload does
   not require default construction of the target object.

   :param result: Object to update.
   :param xml: Complete UTF-8 XML input.
   :param fixed_name: Override the expected root name.

   The input is copied before parsing, so it may refer to a string owned by
   ``result``. XML is fully parsed before member assignments. A conversion,
   assignment, or setter failure can leave ``result`` partially updated. An absent
   member annotated with :cpp:var:`optional` keeps its current value.
   The exception behavior is the same as the value-returning overload.

.. cpp:function:: template <typename T> T from_string(std::string_view text)

   Convert the decoded text of a scalar XML member to ``T``. This function also
   serves as the customization point for leaf types.

   :param text: Scalar text after XML entity decoding and XML newline normalization.
      Calling this function directly does not parse XML or decode entities.
   :returns: The converted value.
   :throws deserialization_error: The built-in converter cannot parse the complete
      representation, a number is out of range, or a container has the wrong size.

   Built-in conversions support:

   * ``std::string``, retaining whitespace.
   * ``bool``: ``true``, ``false``, ``1``, and ``0`` after trimming XML whitespace.
   * Arithmetic types via ``std::from_chars`` after trimming XML whitespace.
     ``char`` therefore uses its numeric representation.
   * ``std::optional<T>``, constructing an engaged optional from ``T``'s conversion.
   * Tuple-like types such as pairs and tuples in ``(value, value)`` form.
   * Owning, writable sequences in ``[value, value]`` form, sets in
     ``{value, value}`` form, and maps in ``{key: value}`` form.

   Nested containers and quoted strings use the standard formatting representation.
   Fixed-size arrays must receive exactly their size; fixed-capacity containers
   reject too many items. Views such as ``std::span`` are not owning input targets.
   Unsupported leaf types produce a compile-time diagnostic requesting a
   specialization.

   Supply a specialization for a custom formatted leaf. It receives the formatted
   text; a ``format`` annotation is not automatically inverted during input:

   .. code-block:: cpp

      struct Code {
        int value;
      };

      template <>
      Code serial_xml::from_string<Code>(std::string_view text) {
        return {serial_xml::from_string<int>(text)};
      }

   See :doc:`deserialization` for complete custom-format and setter examples.

.. cpp:class:: deserialization_error : public std::runtime_error

   Exception for XML parsing, input schema checks, and built-in text conversion
   failures. Inherits ``std::runtime_error`` constructors and ``what()``.
   Catch it to obtain the diagnostic message:

   .. code-block:: cpp

      try {
        auto person = serial_xml::from_xml<Person>(xml);
      } catch (const serial_xml::deserialization_error& error) {
        std::println("{}", error.what());
      }

Pretty printing
---------------

.. cpp:function:: std::string prettify(const std::string& xml)

   Validate XML and return a readable representation using two-space indentation
   for element-only content. Text-bearing subtrees, including whitespace and
   CDATA, and subtrees with an ``xml:space`` attribute retain their original bytes.

   :param xml: A complete XML document or single-root fragment.
   :returns: Formatted XML. No final newline is added automatically.
   :throws deserialization_error: The input is malformed or unsupported by the XML parser.

   Inserted whitespace becomes text when reparsed. Use compact :cpp:func:`to_xml`
   output when round-tripping raw scalar members alongside child elements or when
   every whitespace node matters.

Annotation value types
----------------------

These structural values are used in C++26 annotations such as
``[[= serial_xml::name{"person"}]]``. Array lengths include the null terminator;
class template argument deduction supplies them from string literals.

.. cpp:struct:: template <std::size_t N> name

   Override a member's XML element or attribute name, or a class/schema root name.

   .. cpp:var:: char value[N]

      The supplied null-terminated XML name, stored inline.

   .. cpp:function:: constexpr name(const char (&str)[N])

      Copy and validate ``str`` as an XML name. Empty names are invalid. Invalid
      names or UTF-8 throw ``std::invalid_argument`` during construction; in a
      constant annotation this prevents compilation.

   .. cpp:function:: static constexpr bool is_empty()

      Return whether ``N == 1``. This reports the template's array length;
      construction still rejects an empty name.

.. cpp:struct:: template <std::size_t N = 1, typename F = std::nullptr_t> format

   Specify a standard format specifier or a custom formatter function.

   .. cpp:var:: char value[N] = {}

      Inline format-specifier storage. Use a specifier without ``:`` or braces;
      the serializer builds ``{:specifier}``.

   .. cpp:var:: F function = {}

      Callable storage for function-based formatting. ``std::nullptr_t`` selects
      the string-specifier form.

   .. cpp:function:: constexpr format(const char (&str)[N])

      Available only when ``F`` is ``std::nullptr_t``. Copy the format specifier,
      for example ``format{".2f"}``. The empty specifier uses default formatting.

   .. cpp:function:: constexpr format(F formatter)

      Available only when ``F`` is not ``std::nullptr_t``. Store a callable that
      accepts the member value and returns a value constructible as
      ``std::string``. For example, ``format{[](int n) { return std::to_string(n); }}``.
      Function-based formatting disables automatic object unpacking.

   Formatting affects output. Input uses :cpp:func:`from_string` and may need a
   matching specialization. See :doc:`annotations` for interaction with iteration.

.. cpp:struct:: template <std::size_t N1 = 1, std::size_t N2 = 1> iter

   Iterate a range with configurable item and wrapper names. Applicable only to
   range members.

   .. cpp:var:: char single[N1] = ""

      Item element name. An empty string selects ``element``.

   .. cpp:var:: char multiple[N2] = ""

      Wrapper element name. An empty string uses the member identifier, or
      ``elements`` when an identifier is unavailable.

   .. cpp:function:: constexpr iter(const char (&s)[N1])

      Set the item name, leaving the wrapper name at its default.

   .. cpp:function:: constexpr iter(const char (&s)[N1], const char (&m)[N2])

      Set both names, for example ``iter{"person", "people"}``. Each nonempty
      name must be a valid XML name; invalid names throw ``std::invalid_argument``
      during construction. Empty strings request the defaults.

   There is no zero-argument constructor. Use ``iter{""}`` for default names.
   ``raw`` removes the wrapper, while keeping item elements.

Annotation markers
------------------

Each constant is an exported value of a distinct marker type. Use the constants
directly in annotations; the marker types are implementation details. See
:doc:`annotations` for valid combinations and precedence.

.. cpp:var:: constexpr attribute_ attribute

   Write/read a member as an XML attribute instead of a child element.
   ``name`` can override the attribute name. Attributes cannot be combined with
   ``raw``, ``cdata``, or object unpacking.

.. cpp:var:: constexpr skip_ skip

   Exclude a member from both serialization and deserialization. A skipped schema
   placeholder does not need a matching target member.

.. cpp:var:: constexpr raw_ raw

   Omit a scalar member's element tags and write escaped text into the parent.
   For iterated ranges, omit the wrapper while retaining item tags. This does not
   insert unescaped XML. Multiple raw scalar input members cannot be separated.

.. cpp:var:: constexpr cdata_ cdata

   Emit formatted text directly into the parent as CDATA. The serializer splits
   ``]]>`` safely and preserves carriage returns using character references.
   Input consumes CDATA fields in member order. Default arithmetic conversion
   takes precedence and keeps ordinary tagged numeric output.

.. cpp:var:: constexpr unpack_ unpack

   Reflect an object's members inside its enclosing element rather than formatting
   it as scalar text. Non-formattable class members are unpacked automatically.

.. cpp:var:: constexpr no_unpack_ no_unpack

   Disable automatic object unpacking and use formatted text. Input requires a
   compatible :cpp:func:`from_string` conversion.

.. cpp:var:: constexpr no_iter_ no_iter

   Disable automatic iteration of supported standard containers and use their
   formatted representation instead. Input converts that representation through
   :cpp:func:`from_string`.

.. cpp:var:: constexpr exclude_on_empty_ exclude_on_empty

   Omit empty automatically handled range output. Without this marker, an empty
   range emits a self-closing wrapper. When reading an absent automatically
   iterated range with this marker, value-initialize the container (empty for dynamic ranges).

.. cpp:var:: constexpr optional_ optional

   Allow an input field or selected setter to be absent. Leave its initializer or
   existing value unchanged, and do not invoke an absent setter. This marker does
   not change serialization and is distinct from a ``std::optional<T>`` member.

.. cpp:var:: constexpr setter_ setter

   Select a non-static, one-parameter method for deserialization. Ignored during
   serialization. Without a ``name`` override, ``set_name`` reads ``name`` and
   ``setName`` reads ``name``. See :doc:`deserialization` for automatic getter/setter
   matching and schema setters.
