# Run the benchmarks

Measure serialization and deserialization on the repository’s order workload.
The executable verifies round trips before timing and reports timings per order.
Use a release build so compiler optimization is enabled.

## Prepare the build

Use the dev container’s GCC and CMake toolchain. Install Boost Serialization and
cereal in that environment; the benchmark configuration fetches pugixml 1.16.
From the repository root, configure the release build with tests and benchmarks:

```bash
cmake --preset release-test-gcc -DBUILD_BENCHMARKS=ON
```

If configuration cannot find a required dependency, install it before continuing.
Disabling benchmarks would omit the executable you need for this task.

## Build and validate the workload

```bash
cmake --build --preset build-release-test
ctest --test-dir build/release --output-on-failure
```

Continue after the build and tests succeed. The benchmark also checks its own
round-trip results before recording measurements, so an invalid conversion is
reported before timing begins.

## Run a measurement

Run without CPU pinning first:

```bash
./build/release/benchmarks/xml_serialization_benchmarks 100000 1000
```

The first argument requests 100,000 measured iterations; the second requests
1,000 warm-up iterations. These are also the defaults if you omit both arguments.
Output compares SerialXML, Boost.Serialization, cereal, and pugixml for the same
logical order workload. Smaller nanoseconds-per-order values indicate less time
spent per operation.

## Repeat with a fixed CPU

On Linux, find the CPUs available to your process:

```bash
taskset -pc $$
```

Choose a CPU from that list. For example, if CPU 4 is available:

```bash
for run in 1 2 3 4 5; do
  taskset -c 4 ./build/release/benchmarks/xml_serialization_benchmarks 100000 1000
done
```

Run the measurements sequentially and compare the median of each library’s five
results. Record the CPU model, compiler version, build options, and iteration
counts with your measurements. Hardware, compiler settings, and XML shape affect
both absolute timings and relative performance.

The libraries emit different archive metadata and container names. Compare time
per logical order rather than XML bytes per second. The [recorded benchmark results](benchmarks.md)
provide the repository’s workload and environment for comparison.
