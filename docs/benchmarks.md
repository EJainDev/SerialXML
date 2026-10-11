# Benchmarks

The repository compares SerialXML with Boost.Serialization, cereal, and pugixml
using an order with customer details, a shipping address, line items, and integer
tags. The benchmark checks round trips before timing. Deserialization includes
parsing, numeric conversion, and owning object construction.

## Recorded results

In the order benchmark reported in the project README, SerialXML deserialization
was **3.8× faster than cereal, 16.8× faster than Boost, and 9% faster than pugixml**.

| Library | Serialization (ns/order) | Deserialization (ns/order) |
| --- | ---: | ---: |
| **SerialXML** | **830.308** | **849.361** |
| Boost.Serialization | 6959.951 | 14250.021 |
| cereal | 7699.705 | 3195.171 |
| pugixml | 1523.398 | 936.498 |

Lower is better. These measurements were recorded on an Intel Core 7 240H with
GCC 16.2.0. They describe this workload and environment; timings and relative
performance can change with hardware, compiler settings, data, and XML structure.
The libraries produce different incidental archive metadata and container names,
so compare logical orders per second rather than XML bytes per second.

For build commands and CPU pinning, follow [Run the benchmarks](benchmarking.md).
