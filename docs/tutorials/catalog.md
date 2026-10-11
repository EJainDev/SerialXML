# Build a book catalogue

Create a document containing two books, then reconstruct the collection and print
its entries. You will learn how nested records and named range items combine in
one XML document.

Use the application from [Shape an XML document](schema.md). Keep its
`CMakeLists.txt`; you will replace `main.cpp` for this project. You need the same
GCC reflection toolchain. Run the build commands from your application directory.

## 1. Write one book

Replace `main.cpp` with this program:

```{literalinclude} ../_snippets/tutorials/catalog-1.cpp
:language: cpp
```

Build and run:

```bash
cmake --build build
./build/my_app
```

You should see:

```xml
<Book id="1"><title>The XML Handbook</title></Book>
```

The identifier is an attribute on the book; the title is a child. Keep the
imports and `Book` declaration for the next step.

## 2. Put two books in a catalogue

Add this declaration after `Book`, before `main`:

```cpp
struct [[= serial_xml::name{"catalog"}]] Catalog {
  [[= serial_xml::iter{"book"}]] std::vector<Book> books;
};
```

The root annotation gives the document a lowercase name. The `iter` annotation
names each repeated item `book`; the enclosing member keeps its name `books`.
Replace `main` with:

```{literalinclude} ../_snippets/tutorials/catalog-2.cpp
:language: cpp
:lines: 13-
```

Build and run again. This time the output is:

```xml
<catalog>
  <books>
    <book id="1">
      <title>The XML Handbook</title>
    </book>
    <book id="2">
      <title>Reflection in C++</title>
    </book>
  </books>
</catalog>
```

The `Book` annotations apply inside each `book` wrapper. There is no separate
`Book` element inside it. `prettify` makes the nesting visible for display.

## 3. Read the collection back

Replace `main` once more to read the compact document and print each entry:

```{literalinclude} ../_snippets/tutorials/catalog.cpp
:language: cpp
:lines: 13-
```

Build and run. After the XML, you should see:

```text
Restored books: 2
1: The XML Handbook
2: Reflection in C++
```

The reader reconstructs the vector in item order. Change the second title in the
initializer, rebuild, and confirm that both the XML and the restored entry show
your new title.

## Completed program

Here is the complete `main.cpp`, including both record declarations:

```{literalinclude} ../_snippets/tutorials/catalog.cpp
:language: cpp
```

You have now written and read nested records in a named collection. Continue with
[Load and update application settings](settings.md) to read documents that omit
some fields and to handle invalid input.
