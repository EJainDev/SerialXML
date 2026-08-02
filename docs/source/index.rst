.. SerialXML documentation master file, created by
   sphinx-quickstart on Sun Aug  2 22:38:21 2026.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

SerialXML Documentation
=======================

.. image:: _static/SerialXML.png
   :alt: SerialXML Logo
   :align: center

Welcome to SerialXML's official documentation home page! This documentation provides comprehensive information about SerialXML, including installation instructions, usage guides, examples, and API references.

SerialXML is a C++26 reflection based serialization library for XML. All you need is a formattable struct, and SerialXML will do the rest.

Quickstart
----------

Getting started is easy. Simply import the module and call `to_xml` on any regular C++ struct. No modifications required to anything!

.. code-block:: cpp

   // main.cpp

   import std;
   import serial_xml;

   struct Person {
      int age;
      std::string favorite_food;
   }

   int main() {
      std::print(to_xml(Person{3, "pizza"}));
   }

That's all you need: one function call and SerialXML does the rest.

Installation
------------

CMake FetchContent (Recommended)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cmake

   FetchContent_Declare(
      serial_xml
      GIT_REPOSITORY https://github.com/EJainDev/SerialXML.git
      GIT_TAG main
   )
   FetchContent_MakeAvailable(serial_xml)

   add_executable(my_tests test.cpp)
   target_link_libraries(my_tests PRIVATE serial_xml::serial_xml)

Install from source
~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   git clone https://github.com/EJainDev/SerialXML.git
   cd SerialXML

   cmake --preset "release-gcc-16"
   cmake --build build

   cmake --install build

Then, in your `CMakeLists.txt`, put:

.. code-block:: cmake

   find_package(SerialXML REQUIRED)
   target_link_libraries(my_app PRIVATE serial_xml::serial_xml)

Requirements
------------

============ =========== ==============================================
Component    Min Version Notes
============ =========== ==============================================
Compiler     GCC 16.1    C++26 SIMD, Reflection, and more
CMake        4.3         Change `std` experiment key for lower versions
C++ Standard 26          SIMD, Reflection, Annotations
============ =========== ==============================================

Add your content using ``reStructuredText`` syntax. See the
`reStructuredText <https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html>`_
documentation for details.


.. toctree::
   :maxdepth: 3
   :caption: Contents:

