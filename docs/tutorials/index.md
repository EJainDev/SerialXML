# Tutorials

<div class="hero-kicker">Learn by building</div>

## From your first record to a working XML application

Start with a small round trip, then build a catalogue, load application settings,
and define a custom product code. Each lesson gives you complete code, steps you
can run, and exact output to check along the way.

<div class="tutorial-start">
<p><strong>New to SerialXML? Start with one record.</strong><br>The first lesson sets up your CMake application and takes you from a C++ struct to XML and back.</p>
<a class="tutorial-start-link" href="../quickstart.html">Start your first round trip →</a>
</div>

## Your learning path

Follow the lessons in order. After the first lesson, reuse the same CMake project
and change `main.cpp` as each project directs. You can revisit any lesson when
you want to practice that part of the mapping.

<ol class="tutorial-path">
<li><a href="../quickstart.html"><span class="lesson-number">01</span><span class="lesson-content"><strong>Your first XML round trip</strong><span>Build your application, write a Person, and restore its values.</span><span class="lesson-skills">Project setup · Serialization · Deserialization</span></span><span class="lesson-arrow" aria-hidden="true">→</span></a></li>
<li><a href="schema.html"><span class="lesson-number">02</span><span class="lesson-content"><strong>Shape an XML document</strong><span>Turn age into an attribute, rename a child, and add named hobby items.</span><span class="lesson-skills">Annotations · XML names · Repeated items</span></span><span class="lesson-arrow" aria-hidden="true">→</span></a></li>
<li><a href="catalog.html"><span class="lesson-number">03</span><span class="lesson-content"><strong>Build a book catalogue</strong><span>Combine nested records and a collection, then read every book back.</span><span class="lesson-skills">Nested objects · Collections · Round trips</span></span><span class="lesson-arrow" aria-hidden="true">→</span></a></li>
<li><a href="settings.html"><span class="lesson-number">04</span><span class="lesson-content"><strong>Load and update application settings</strong><span>Keep defaults, update current values, and report invalid input.</span><span class="lesson-skills">Optional fields · In-place updates · Errors</span></span><span class="lesson-arrow" aria-hidden="true">→</span></a></li>
<li><a href="product-code.html"><span class="lesson-number">05</span><span class="lesson-content"><strong>Create a custom product code</strong><span>Write SKU-42, reconstruct its value, and reject an unexpected prefix.</span><span class="lesson-skills">Custom formatting · Input conversion · Validation</span></span><span class="lesson-arrow" aria-hidden="true">→</span></a></li>
</ol>

## Before you begin

You need basic C++, GCC 16.1+ with reflection support, CMake 4.3.3+, and Ninja.
The repository's dev container provides the toolchain. The first lesson includes
the application configuration; no XML experience is required.

```{toctree}
:hidden:
:maxdepth: 1

../quickstart
schema
catalog
settings
product-code
```
