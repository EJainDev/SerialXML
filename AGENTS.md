# Repository Guidelines

## Project Structure & Module Organization

`include/serial_xml.cxx` is the C++26 `serial_xml` module and the library's main implementation. Add focused usage samples under `examples/`, Google Test cases in `tests/test_serial_xml.cpp`, and performance comparisons in `benchmarks/`. `assets/` holds README images; `cmake/` holds package configuration. The root `CMakeLists.txt` and `CMakePresets.json` define builds. Generated files belong in the ignored `build/` directory.

## Build, Test, and Development Commands

Use CMake 4.3.3 or newer, Ninja, and GCC 16 with C++26 reflection support. The dev container provides a matching environment. The current preset names are `debug-gcc`, `debug-test-gcc`, `release-gcc`, and `release-test-gcc`.

```sh
cmake --preset debug-test-gcc             # Configure a debug build with tests
cmake --build --preset build-debug-test   # Build library, examples, benchmarks, and tests
ctest --test-dir build/debug --output-on-failure
./build/debug/examples/hello_world       # Run one example
```

Tests are disabled in the plain `debug-gcc` and `release-gcc` presets. Benchmarks are enabled by default and require Boost Serialization and cereal; configure with `-DBUILD_BENCHMARKS=OFF` when those dependencies are unavailable.

## Coding Style & Naming Conventions

Format changed C++ files with `clang-format -i <file>`; `.clang-format` uses Google style and a 100-column limit (two-space indentation). Follow the existing `snake_case` module, function, and file names. Use `.clang-tidy` for modernize, readability, performance, bugprone, C++ Core Guidelines, and analyzer checks. Explain any suppressed check in the pull request.

## Testing Guidelines

Use Google Test's `TEST(Suite, Behavior)` pattern and add cases to `tests/test_serial_xml.cpp`. Cover new behavior and regressions with assertions on the exact XML output where practical. Run the full `ctest` command above before submitting; all tests must pass. No numeric coverage threshold is specified.

## Commit & Pull Request Guidelines

Recent commits use Conventional Commits, such as `feat(format): support custom formatter functions`, `fix: ...`, and `test: ...`. Keep each pull request focused on one feature or fix, link related issues, describe behavior changes, and update examples or documentation when the API changes. Include the build and test commands you ran; add a minimal reproducer for bug fixes. See `CONTRIBUTING.md` and `CODE_OF_CONDUCT.md` for the full contribution process.
