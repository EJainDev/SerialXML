# Test scope and validation

The runtime suite uses AnnoTest. Run every test, including compiler rejection
checks, with `ctest --test-dir build/debug --output-on-failure --no-tests=error`.
Use `./build/debug/tests/serial_xml_tests --list` to inspect individual runtime
cases and `--filter 'FromXmlTests.*'` to select a suite.

| Area | Checked behavior |
| --- | --- |
| XML syntax | Malformed elements, attributes, closing tags, declarations, comments, processing instructions, CDATA, entities, duplicate members, trailing content, unsupported DTD/entity declarations; targeted cases check diagnostic categories. |
| Unicode | Every XML 1.0-allowed code point round-trips through attributes, children, raw text, and CDATA; XML-forbidden controls, surrogates, and BMP noncharacters are rejected; malformed UTF-8 and multibyte splits at word/SIMD/arena boundaries are exercised. |
| Names | XML Name range endpoints, continuation-only characters, explicit names, root overrides, Unicode/prefixed names, and literal namespace matching. |
| Numbers | Every standard signed/unsigned integer width at zero/minimum/maximum and overflow/underflow, float/double/long-double extrema, subnormals, negative zero, malformed exponents, NaN/infinity, and strict Boolean spellings. |
| Containers | Empty/nested sequences, maps/sets/tuples, escaped keys and strings, exact fixed capacities and array lengths including zero, associative duplicates, explicit and raw iteration. |
| Annotations | Names, attributes, skip, raw, CDATA, optional/default/empty handling, formatting, unpacking, getters/setters, mock schema ordering and target dispatch. |
| Lifetimes/failures | Owning results after source destruction, aliased in-place input, arena overflow, malformed XML before assignment, conversion/setter partial updates and recovery, retained setter side effects, independent concurrent callers. |
| Pretty printing | Exact indentation, mixed content, quoted markup, comments/PIs/CDATA, BOMs, `xml:space`, and idempotence. |
| Compile contracts | 22 invalid schemas cover annotation conflicts, non-range iteration, missing/ambiguous/mismatched mock members, invalid setters, const targets, unsupported/non-owning values. Each must fail with the intended diagnostic, not merely any compiler error. Existing consumer checks also reject six invalid XML name schemas. |
| Generated inputs | Fixed-seed 1,000 Unicode round trips and 4,000 malformed/byte-mutated documents run in the normal suite. Traces identify reproducing inputs. |
| Mutation checks | The explicit four-defect sample requires assertion failures for strict numeric consumption, depth/capacity off-by-one defects, and missing quote escaping. This is a sampled effectiveness check, not a complete mutation score. |

## Coverage measurement

Use a gcov binary matching the compiler. For example, with this workspace's
`/usr/local/bin/g++-16` (which reports GNU 17 experimental):

```sh
cmake --preset debug-test-gcc -B build/edge-coverage \
  -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-16 \
  -DBUILD_EXAMPLES=OFF -DBUILD_BENCHMARKS=OFF \
  -DSERIAL_XML_TEST_COVERAGE=ON
cmake --build build/edge-coverage --parallel 2
ctest --test-dir build/edge-coverage --output-on-failure --no-tests=error
python3 tests/tools/coverage_report.py --build-dir build/edge-coverage \
  --gcov /usr/local/bin/gcov-16.1
```

Coverage instrumentation uses atomic counter updates so concurrent tests do not race on gcov counters. The report merges module and test-instantiated template counters by source line.
It writes `coverage-summary.json` inside the selected build directory, listing
uncovered lines and raw branch totals. Raw branches include compiler-generated
and exception edges, with separate counts for each template instantiation.
Line coverage does not establish condition coverage, compile-time reflection
coverage, or exhaustive coverage of all possible documents and user-defined
C++ types. No lines are hidden or excluded to inflate the result.

For mutation checks:

```sh
python3 tests/tools/mutation_check.py --build-dir build/debug
```

The runner verifies a passing isolated baseline, then builds each mutant and
requires a recorded assertion/expectation failure. Compiler errors, crashes,
empty selections, and unrelated uncaught exceptions do not count as detections.
Results and individual logs are under `build/mutation-checks` by default.

## Known toolchain boundary

GNU 17.0.0 experimental (20260501) rejects reflection template arguments naming
members of a class local to a generic lambda when multiple lambda specializations
are instantiated. `compiler_issues/local_generic_reflection.cpp` reproduces the
mangling conflict without SerialXML or AnnoTest. The numeric-width tests use a
namespace-scope class template to exercise the library paths independently of
that compiler defect. Local generic-lambda classes remain unverified on this
toolchain; the reproducer is not an expected-failure test that would lock in the
compiler bug. The standalone reproducer also fails with `-fabi-version=0` on this toolchain.
