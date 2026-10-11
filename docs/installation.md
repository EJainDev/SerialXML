# Install SerialXML

## Requirements

| Component | Minimum version | Notes |
| --- | --- | --- |
| Compiler | GCC 16.1 | C++26 reflection, annotations, and SIMD |
| CMake | 4.3.3 | Enable experimental `import std` before `project()` |
| Build system | Ninja | Used by the repository presets |
| C++ standard | 26 | Required by the library |

The dev container uses GCC 16.2.0 and CMake 4.4.3. Other compilers are not currently
supported. StructuralTuple is pinned to a tested commit.

## Create a consumer application

Create an empty application directory. Save this complete configuration as
`CMakeLists.txt`; it selects C++26, enables standard-library modules, fetches
SerialXML, and links the executable to its exported target.

### Configure CMake FetchContent

```{literalinclude} _snippets/CMakeLists.txt
:language: cmake
:caption: CMakeLists.txt
```

## Add the application code

Save this program as `main.cpp` beside `CMakeLists.txt`:

```cpp
import std;
import serial_xml;

struct Record {
  int value;
};

int main() {
  const auto xml = serial_xml::to_xml(Record{42}, false);
  std::println("{}", xml);
  const auto restored = serial_xml::from_xml<Record>(xml);
  std::println("Restored value: {}", restored.value);
}
```

You now have both files needed for the consumer. `import serial_xml` uses the
module built by CMake; linking the target supplies the reflection compiler options.

## Install from source

```bash
git clone https://github.com/EJainDev/SerialXML.git
cd SerialXML

cmake --preset release-gcc -DBUILD_BENCHMARKS=OFF
cmake --build --preset build-release
cmake --install build/release --prefix "$HOME/.local"
```

Use the same compiler and standard-module setup shown above, then replace the
FetchContent block with:

```cmake
find_package(SerialXML 1.0 CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE serial_xml::serial_xml)
```

Configure your application with `-DCMAKE_PREFIX_PATH="$HOME/.local"` for this
install prefix. A source install also installs the fetched StructuralTuple dependency;
if using an existing StructuralTuple package, make its prefix available to consumers too.
Reflection compiler options and the C++26 requirement propagate from the library target.
FetchContent builds default to disabling examples and benchmarks when embedded.

## Configure and build the consumer

Save the CMake configuration as `CMakeLists.txt` alongside your `main.cpp`.
With the required GCC selected as your C++ compiler, run:

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/my_app
```

The consumer program above prints:

```xml
<Record><value>42</value></Record>
```

```text
Restored value: 42
```

## Troubleshoot integration

| Symptom | Check |
| --- | --- |
| Reflection syntax is rejected | Use the required GCC build; a `-std=c++26` flag alone does not supply reflection support. |
| CMake rejects the experimental `import std` key | Match the opt-in key to your CMake version, before `project()`. The configuration above covers the repository's supported versions. |
| `serial_xml` cannot be imported | Link the executable to `serial_xml::serial_xml` and let CMake build the module. |
| A package cannot be found after source installation | Set `CMAKE_PREFIX_PATH` to the installation prefix and make StructuralTuple available too. |
| Benchmarks fail while building the repository | Configure with `-DBUILD_BENCHMARKS=OFF` if Boost Serialization or cereal is unavailable. |

After changing the compiler, configure a new build directory to avoid reusing
module artifacts from another toolchain.
