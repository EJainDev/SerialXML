#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <pugixml.hpp>

import std;

import serial_xml;

namespace {

// Each library serializes this same logical payload: an order with customer
// details, line items, and an integer tag list. The generated XML differs in
// incidental archive metadata and container naming, so benchmark results
// should be compared as serialization throughput rather than output size.

struct Address {
  std::string street;
  std::string city;
  std::string postal_code;
};

struct LineItem {
  std::string sku;
  std::string description;
  int quantity;
  double unit_price;
};

struct Order {
  std::uint64_t id;
  std::string customer;
  Address shipping_address;
  std::vector<LineItem> items;
  std::vector<int> tags;
};

struct BoostAddress {
  std::string street;
  std::string city;
  std::string postal_code;

  template <class Archive>
  void serialize(Archive& archive, const unsigned int) {
    archive& BOOST_SERIALIZATION_NVP(street);
    archive& BOOST_SERIALIZATION_NVP(city);
    archive& BOOST_SERIALIZATION_NVP(postal_code);
  }
};

struct BoostLineItem {
  std::string sku;
  std::string description;
  int quantity;
  double unit_price;

  template <class Archive>
  void serialize(Archive& archive, const unsigned int) {
    archive& BOOST_SERIALIZATION_NVP(sku);
    archive& BOOST_SERIALIZATION_NVP(description);
    archive& BOOST_SERIALIZATION_NVP(quantity);
    archive& BOOST_SERIALIZATION_NVP(unit_price);
  }
};

struct BoostOrder {
  std::uint64_t id;
  std::string customer;
  BoostAddress shipping_address;
  std::vector<BoostLineItem> items;
  std::vector<int> tags;

  template <class Archive>
  void serialize(Archive& archive, const unsigned int) {
    archive& BOOST_SERIALIZATION_NVP(id);
    archive& BOOST_SERIALIZATION_NVP(customer);
    archive& BOOST_SERIALIZATION_NVP(shipping_address);
    archive& BOOST_SERIALIZATION_NVP(items);
    archive& BOOST_SERIALIZATION_NVP(tags);
  }
};

struct CerealAddress {
  std::string street;
  std::string city;
  std::string postal_code;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("street", street), cereal::make_nvp("city", city),
            cereal::make_nvp("postal_code", postal_code));
  }
};

struct CerealLineItem {
  std::string sku;
  std::string description;
  int quantity;
  double unit_price;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("sku", sku), cereal::make_nvp("description", description),
            cereal::make_nvp("quantity", quantity), cereal::make_nvp("unit_price", unit_price));
  }
};

struct CerealOrder {
  std::uint64_t id;
  std::string customer;
  CerealAddress shipping_address;
  std::vector<CerealLineItem> items;
  std::vector<int> tags;

  template <class Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("id", id), cereal::make_nvp("customer", customer),
            cereal::make_nvp("shipping_address", shipping_address),
            cereal::make_nvp("items", items), cereal::make_nvp("tags", tags));
  }
};

Order make_serial_xml_order() {
  return {42,
          "Ada Lovelace",
          {"12 Analytical Engine Way", "London", "SW1A 1AA"},
          {{"BOOK-001", "Notes on the Analytical Engine", 2, 19.95},
           {"PEN-002", "Mechanical pencil", 3, 4.50},
           {"PAPER-003", "Grid notebook", 1, 12.00}},
          {7, 11, 42, 99}};
}

BoostOrder make_boost_order() {
  return {42,
          "Ada Lovelace",
          {"12 Analytical Engine Way", "London", "SW1A 1AA"},
          {{"BOOK-001", "Notes on the Analytical Engine", 2, 19.95},
           {"PEN-002", "Mechanical pencil", 3, 4.50},
           {"PAPER-003", "Grid notebook", 1, 12.00}},
          {7, 11, 42, 99}};
}

CerealOrder make_cereal_order() {
  return {42,
          "Ada Lovelace",
          {"12 Analytical Engine Way", "London", "SW1A 1AA"},
          {{"BOOK-001", "Notes on the Analytical Engine", 2, 19.95},
           {"PEN-002", "Mechanical pencil", 3, 4.50},
           {"PAPER-003", "Grid notebook", 1, 12.00}},
          {7, 11, 42, 99}};
}

struct PugiStringWriter final : pugi::xml_writer {
  std::string output;

  void write(const void* data, std::size_t size) override {
    output.append(static_cast<const char*>(data), size);
  }
};

std::string pugixml_serialize(const Order& order) {
  pugi::xml_document document;
  auto root = document.append_child("order");

  root.append_child("id").text().set(order.id);
  root.append_child("customer").text().set(order.customer.c_str());

  auto address = root.append_child("shipping_address");
  address.append_child("street").text().set(order.shipping_address.street.c_str());
  address.append_child("city").text().set(order.shipping_address.city.c_str());
  address.append_child("postal_code").text().set(order.shipping_address.postal_code.c_str());

  auto items = root.append_child("items");
  for (const auto& item : order.items) {
    auto node = items.append_child("item");
    node.append_child("sku").text().set(item.sku.c_str());
    node.append_child("description").text().set(item.description.c_str());
    node.append_child("quantity").text().set(item.quantity);
    node.append_child("unit_price").text().set(item.unit_price);
  }

  auto tags = root.append_child("tags");
  for (const auto tag : order.tags) {
    tags.append_child("tag").text().set(tag);
  }

  PugiStringWriter writer;
  document.save(writer, "", pugi::format_raw | pugi::format_no_declaration);
  return std::move(writer.output);
}

template <typename Function>
void benchmark(std::string_view name, Function&& function, std::size_t iterations,
               std::size_t warmup_iterations) {
  // Warm up caches, allocation paths, branch predictors, etc.
  for (std::size_t i = 0; i < warmup_iterations; ++i) {
    std::invoke(function);
  }

  const auto start = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < iterations; ++i) {
    std::invoke(function);
  }

  const auto end = std::chrono::steady_clock::now();

  const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  const double total_ms = elapsed.count() / 1'000'000.0;
  const double ns_per_iteration = static_cast<double>(elapsed.count()) / iterations;

  std::println("{}:", name);
  std::println("  iterations: {}", iterations);
  std::println("  total:      {:.3f} ms", total_ms);
  std::println("  per iter:   {:.3f} ns", ns_per_iteration);
  std::println();
}

}  // namespace

int main(int argc, char** argv) {
  std::size_t iterations = 100'000;
  std::size_t warmup_iterations = 1'000;

  if (argc > 1) {
    iterations = std::stoull(argv[1]);
  }

  if (argc > 2) {
    warmup_iterations = std::stoull(argv[2]);
  }

  const auto serial_order = make_serial_xml_order();
  const auto boost_order = make_boost_order();
  const auto cereal_order = make_cereal_order();

  benchmark(
      "serial_xml",
      [&] {
        auto xml = serial_xml::to_xml(serial_order, false, "order");

        // Keep the result observable.
        asm volatile("" : : "g"(xml.data()), "g"(xml.size()) : "memory");
      },
      iterations, warmup_iterations);

  benchmark(
      "boost_xml",
      [&] {
        std::ostringstream output;
        boost::archive::xml_oarchive archive(output, boost::archive::no_header);
        archive << boost::serialization::make_nvp("order", boost_order);

        const auto xml = output.str();

        asm volatile("" : : "g"(xml.data()), "g"(xml.size()) : "memory");
      },
      iterations, warmup_iterations);

  benchmark(
      "cereal_xml",
      [&] {
        std::ostringstream output;
        cereal::XMLOutputArchive archive(output);
        archive(cereal::make_nvp("order", cereal_order));

        const auto xml = output.str();

        asm volatile("" : : "g"(xml.data()), "g"(xml.size()) : "memory");
      },
      iterations, warmup_iterations);

  benchmark(
      "pugixml",
      [&] {
        auto xml = pugixml_serialize(serial_order);
        asm volatile("" : : "g"(xml.data()), "g"(xml.size()) : "memory");
      },
      iterations, warmup_iterations);
}