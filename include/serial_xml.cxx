module;

#include <cstdint>
#include <simd>  // GCC 16.1 does not have in this in the std module

export module serial_xml;

import std;

import structural_tuple;

namespace serial_xml {
export template <std::size_t N>
struct name {
  char value[N];

  constexpr name(const char (&str)[N]) {
    for (std::size_t i = 0; i < N; ++i) value[i] = str[i];
  }

  static constexpr bool is_empty() { return N == 1; }
};

struct skip_ {};
export constexpr const skip_ skip;

struct attribute_ {};
export constexpr const attribute_ attribute;

struct no_unpack_ {};
export constexpr const no_unpack_ no_unpack;

struct unpack_ {};
export constexpr const unpack_ unpack;

struct no_iter_ {};
export constexpr const no_iter_ no_iter;

struct raw_ {};
export constexpr const raw_ raw;

struct cdata_ {};
export constexpr const cdata_ cdata;

constexpr bool is_alpha(const char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

constexpr bool is_num(const char c) { return (c >= '0' && c <= '9'); }

constexpr bool is_alnum(const char c) { return is_alpha(c) || is_num(c); }

export template <std::size_t N>
struct format {
  char value[N];

  constexpr format(const char (&str)[N]) {
    for (std::size_t i = 0; i < N; ++i) value[i] = str[i];
  }
};

export template <std::size_t N1 = 1, std::size_t N2 = 1>
struct iter {
  char single[N1] = "";
  char multiple[N2] = "";

  constexpr iter(const char (&s)[N1]) {
    for (std::size_t i = 0; i < N1; ++i) single[i] = s[i];
  }
  constexpr iter(const char (&s)[N1], const char (&m)[N2]) {
    for (std::size_t i = 0; i < N1; ++i) single[i] = s[i];
    for (std::size_t i = 0; i < N2; ++i) multiple[i] = m[i];
  }
};

template <std::meta::info m>
consteval std::meta::info get_namespace() {
  if constexpr (std::meta::is_namespace(m)) {
    return m;
  }
  if constexpr (std::meta::has_parent(m)) {
    return get_namespace<std::meta::parent_of(m)>();
  }
  return m;
}

template <std::meta::info m>
consteval bool is_stl_handled() {
  if constexpr (get_namespace<m>() == ^^std) {
    static constexpr auto m_t = std::meta::template_of(std::meta::dealias(m));

    if constexpr (m_t == ^^std::vector || m_t == ^^std::array || m_t == ^^std::inplace_vector ||
                  m_t == ^^std::deque || m_t == ^^std::forward_list || m_t == ^^std::span ||
                  m_t == ^^std::valarray || m_t == ^^std::set || m_t == ^^std::unordered_set ||
                  m_t == ^^std::multiset || m_t == ^^std::unordered_multiset ||
                  m_t == ^^std::optional) {
      return true;
    }
  }

  return false;
}

template <std::meta::info m>
consteval auto get_annotations()
    -> structural_tuple::tuple<bool, bool, bool, bool, bool, bool, char const*,
                               std::pair<char const*, char const*>, char const*> {
  static constexpr auto annotations = std::define_static_array(std::meta::annotations_of(m));

  bool is_attribute = false;
  bool is_cdata = false;
  bool is_no_iter = false;
  bool is_raw = false;
  bool is_skip = false;
  bool is_unpack = std::meta::is_class_type(std::meta::type_of(m)) &&
                   !std::formattable<typename[:std::meta::type_of(m):], char> &&
                   !is_stl_handled<std::meta::type_of(m)>();

  std::optional<std::string> custom_format;
  std::optional<std::pair<std::string, std::string>> iter_names;

  std::string name;

  template for (constexpr auto a : annotations) {
    static constexpr auto a_t = std::meta::type_of(a);
    if constexpr (a_t == ^^decltype(::serial_xml::attribute)) {
      is_attribute = true;
    } else if constexpr (a_t == ^^decltype(::serial_xml::cdata)) {
      is_cdata = true;
    } else if constexpr (a_t == ^^decltype(::serial_xml::no_iter)) {
      is_no_iter = true;
    } else if constexpr (a_t == ^^decltype(::serial_xml::raw)) {
      is_raw = true;
    } else if constexpr (a_t == ^^decltype(::serial_xml::skip)) {
      is_skip = true;
    } else if constexpr (a_t == ^^decltype(::serial_xml::unpack)) {
      is_unpack = true;
    } else if constexpr (a_t == ^^decltype(::serial_xml::no_unpack)) {
      is_unpack = false;
    } else if constexpr (std::meta::template_of(a_t) == ^^::serial_xml::format) {
      static constexpr auto format_value = std::meta::extract<typename[:a_t:]>(a);
      custom_format = std::string(format_value.value);
    } else if constexpr (std::meta::template_of(a_t) == ^^::serial_xml::iter) {
      static constexpr auto iter_value = std::meta::extract<typename[:a_t:]>(a);

      std::string multiple_name;
      std::string single_name;

      if constexpr (sizeof(iter_value.multiple) == 1) {
        if constexpr (std::meta::has_identifier(m)) {
          static constexpr auto temp_name = std::meta::identifier_of(m);
          multiple_name = std::string(temp_name);
        } else {
          multiple_name = "elements";
        }
      } else {
        static constexpr auto temp_name = iter_value.multiple;
        multiple_name = std::string(temp_name);
      }

      if constexpr (sizeof(iter_value.single) == 1) {
        single_name = "element";
      } else {
        static constexpr auto temp_name = iter_value.single;
        single_name = std::string(temp_name);
      }

      iter_names = std::make_pair(std::move(single_name), std::move(multiple_name));
    } else if constexpr (std::meta::template_of(a_t) == ^^::serial_xml::name) {
      static constexpr auto name_value = std::meta::extract<typename[:a_t:]>(a);
      name = std::string(name_value.value);
    }
  }

  if constexpr (std::ranges::range<typename[:std::meta::type_of(m):]>) {
    if (iter_names.has_value()) {
      using m_t = std::ranges::range_value_t<typename[:std::meta::type_of(m):]>;
      is_unpack = std::is_class_v<m_t> && !std::formattable<m_t, char>;
    }
  }

  if (name.empty()) {
    if constexpr (std::meta::has_identifier(m)) {
      static constexpr auto temp_name = std::meta::identifier_of(m);
      name = std::string(temp_name);
    } else {
      name = "field";
    }
  }

  return structural_tuple::tuple{
      is_attribute,
      is_cdata,
      is_no_iter,
      is_raw,
      is_skip,
      is_unpack,
      (custom_format.has_value() ? std::define_static_string(custom_format.value()) : nullptr),
      (iter_names.has_value()) ? std::make_pair(std::define_static_string(iter_names->first),
                                                std::define_static_string(iter_names->second))
                               : std::make_pair<char const*, char const*>(nullptr, nullptr),
      std::define_static_string(name)};
}

template <std::meta::info container>
consteval auto get_members() {
  static constexpr auto members = std::define_static_array(
      std::meta::nonstatic_data_members_of(container, std::meta::access_context::current()));

  std::vector<std::pair<std::meta::info,
                        structural_tuple::tuple<bool, bool, bool, bool, char const*,
                                                std::pair<char const*, char const*>, char const*>>>
      child_annotations;
  std::vector<std::pair<std::meta::info, structural_tuple::tuple<char const*, char const*>>>
      attribute_annotations;

  template for (constexpr auto m : members) {
    static constexpr auto m_annotations = get_annotations<m>();
    if constexpr (structural_tuple::get<4>(m_annotations)) {
      continue;
    }

    if constexpr (structural_tuple::get<0>(m_annotations)) {
      static_assert(
          !structural_tuple::get<5>(m_annotations),
          "Cannot have both serial_xml::attribute and serial_xml::unpack annotations on the same "
          "member. If you did not add the unpack annotation, add the serial_xml::no_unpack "
          "annotation to the member. Member name: " +
              std::string(std::meta::identifier_of(m)));

      attribute_annotations.push_back(
          std::make_pair(m, structural_tuple::tuple{structural_tuple::get<6>(m_annotations),
                                                    structural_tuple::get<8>(m_annotations)}));
    } else {
      child_annotations.push_back(std::make_pair(
          m, structural_tuple::tuple{
                 structural_tuple::get<1>(m_annotations), structural_tuple::get<2>(m_annotations),
                 structural_tuple::get<3>(m_annotations), structural_tuple::get<5>(m_annotations),
                 structural_tuple::get<6>(m_annotations), structural_tuple::get<7>(m_annotations),
                 structural_tuple::get<8>(m_annotations)}));
    }
  }

  return std::make_pair(std::define_static_array(attribute_annotations),
                        std::define_static_array(child_annotations));
}

template <auto name>
consteval auto get_attribute_prefix() {
  std::string prefix;
  prefix.reserve(64);

  prefix += ' ';
  prefix += name;
  prefix += "=\"";

  return std::make_tuple(std::define_static_string(prefix), prefix.size());
}

template <std::size_t simd_size>
consteval auto get_type() -> std::meta::info {
  static_assert(simd_size >= 8, "SIMD mask size must be at least 8 elements.");

  if constexpr (simd_size <= 8) {
    return ^^uint8_t;
  }
  if constexpr (simd_size <= 16) {
    return ^^uint16_t;
  }
  if constexpr (simd_size <= 32) {
    return ^^uint32_t;
  }
  return ^^uint64_t;
}

template <typename T>
class Allocator8ByteAligned {
 public:
  using value_type = T;

  Allocator8ByteAligned() noexcept = default;

  template <typename U>
  Allocator8ByteAligned(const Allocator8ByteAligned<U>&) noexcept {}

  [[nodiscard]] T* allocate(const std::size_t n) {
    if (n == 0) {
      return nullptr;
    }

    return static_cast<T*>(std::aligned_alloc(8, n * sizeof(T)));
  }

  void deallocate(T* p, const std::size_t n) noexcept { std::free(p); }
};

template <typename T, typename U>
constexpr bool operator==(const Allocator8ByteAligned<T>&,
                          const Allocator8ByteAligned<U>&) noexcept {
  return true;
}

// clang-format off
thread_local std::vector<typename[:get_type<std::simd::vec<char>::size()>():],  Allocator8ByteAligned<typename[:get_type<std::simd::vec<char>::size()>():]>>
    escape_flags;
// clang-format on

auto get_escape_bitmask(std::string_view input) -> std::size_t {
  using simd_t = std::simd::vec<char>;

  std::size_t padded_size [[indeterminate]];
  if ((input.size() % simd_t::size()) != 0) {
    padded_size = (input.size() / simd_t::size()) + 1;
  } else {
    padded_size = input.size() / simd_t::size();
  }

  using FlagsT = typename[:get_type<simd_t::size()>():];
  escape_flags.resize(std::max(sizeof(uint64_t) / sizeof(FlagsT), padded_size));

  std::size_t i{0};

  static constexpr auto idx_seq = std::make_index_sequence<simd_t::size()>{};

  if (static_cast<std::size_t>(simd_t::size()) <= input.size()) {
    const auto loop_end = input.size() - simd_t::size();
    for (i = 0; i < loop_end; i += simd_t::size()) {
      auto v = std::simd::unchecked_load<simd_t>(input.data() + i, simd_t::size());

      auto mask = (v == '<') | (v == '>') | (v == '&') | (v == '"') | (v == '\'');

      template for (constexpr auto j : idx_seq) {
        escape_flags[i / (sizeof(FlagsT) * 8)] |= mask[static_cast<int>(j)] << j;
      }
    }

    const auto remaining = simd_t::size() - (i - input.size());
    auto v = std::simd::partial_load<simd_t>(input.data() + (input.size() - remaining), remaining);

    auto mask = (v == '<') | (v == '>') | (v == '&') | (v == '"') | (v == '\'');

    static constexpr auto n_idx_seq = std::make_index_sequence<simd_t::size() & 7>{};

    template for (constexpr auto j : n_idx_seq) {
      escape_flags[i / (sizeof(FlagsT) * 8)] |= mask[static_cast<int>(j)] << j;
    }
  } else {
    auto v = std::simd::partial_load<simd_t>(input.data(), input.size());

    auto mask = (v == '<') | (v == '>') | (v == '&') | (v == '"') | (v == '\'');

    template for (constexpr auto j : idx_seq) { escape_flags[0] |= mask[static_cast<int>(j)] << j; }
  }

  return padded_size;
}

auto count_escapes(std::size_t padded_size) -> int {
  using T = std::ranges::range_value_t<decltype(escape_flags)>;

  auto* escape_flags_ptr = reinterpret_cast<uint64_t*>(escape_flags.data());
  int count = 0;
  for (int i = 0;
       i <
       static_cast<int>(std::ceil(static_cast<double>(padded_size) * sizeof(T) / sizeof(uint64_t)));
       ++i) {
    count += std::popcount(escape_flags_ptr[i]);
  }
  return count;
}

auto copy_with_escapes(char* buf, std::string_view input, std::size_t padded_size) -> std::size_t {
  using T = std::ranges::range_value_t<decltype(escape_flags)>;

  const char* original_buf = buf;

  auto* escape_flags_ptr = reinterpret_cast<std::uint64_t*>(escape_flags.data());

  for (std::size_t i = 0;
       i <
       static_cast<int>(std::ceil(static_cast<double>(padded_size) * sizeof(T) / sizeof(uint64_t)));
       ++i) {
    std::size_t last = i * sizeof(std::uint64_t);
    int prev_idx = 0;
    int idx [[indeterminate]];

    while (escape_flags_ptr[i] != 0) {
      idx = std::countr_zero(escape_flags_ptr[i]);

      std::memcpy(buf, input.data() + last + prev_idx, idx - prev_idx);
      buf += (idx - prev_idx);

      prev_idx = idx + 1;

      switch (input[last + idx]) {
        case '<':
          std::memcpy(buf, "&lt;", 4);
          buf += 4;
          break;
        case '>':
          std::memcpy(buf, "&gt;", 4);
          buf += 4;
          break;
        case '&':
          std::memcpy(buf, "&amp;", 5);
          buf += 5;
          break;
        case '"':
          std::memcpy(buf, "&quot;", 6);
          buf += 6;
          break;
        case '\'':
          std::memcpy(buf, "&apos;", 6);
          buf += 6;
          break;
        default:
          throw std::logic_error("Unexpected character for escaping");
      }

      escape_flags_ptr[i] &= (escape_flags_ptr[i] - 1);
    }

    if (static_cast<std::size_t>(prev_idx) < 63) {
      std::memcpy(buf, input.data() + last + prev_idx,
                  std::min(63uz - prev_idx, input.size() - last - prev_idx));
      buf += std::min(63uz - prev_idx, input.size() - last - prev_idx);
    }
  }

  return buf - original_buf;
}

template <char const* name, char const* format>
void add_attribute(std::string& result, std::string& buffer, const auto& value) {
  using T = std::decay_t<decltype(value)>;
  static constexpr auto m_t = ^^std::decay_t<decltype(value)>;

  static constexpr auto prefix_result = get_attribute_prefix<name>();
  static constexpr auto prefix = std::get<0>(prefix_result);
  static constexpr auto prefix_size = std::get<1>(prefix_result);
  static constexpr auto gen_format = std::define_static_string(std::string("{:") + format + "}");

  if constexpr (std::is_arithmetic_v<T> && sizeof(T) <= 64 && std::strcmp(format, "") == 0) {
    if constexpr (std::is_floating_point_v<T>) {
      static constexpr auto format_resize = 311 + prefix_size + 1;

      const auto original_size = result.size();

      result.resize_and_overwrite(result.size() + format_resize,
                                  [&](char* buf, std::size_t max_size) {
                                    buf += original_size;

                                    std::memcpy(buf, prefix, prefix_size);

                                    buf += prefix_size;

                                    auto [ptr, _] = std::to_chars(buf, buf + 311, value);

                                    *ptr = '"';

                                    return original_size + prefix_size + ((ptr + 1) - buf);
                                  });
    } else if constexpr (std::is_integral_v<T>) {
      static constexpr auto format_resize = 20 + prefix_size + 1;

      const auto original_size = result.size();

      result.resize_and_overwrite(result.size() + format_resize, [&](char* buf, std::size_t) {
        buf += original_size;

        std::memcpy(buf, prefix, prefix_size);

        buf += prefix_size;

        auto [ptr, _] = std::to_chars(buf, buf + 20, value);

        *ptr = '"';

        return original_size + prefix_size + ((ptr + 1) - buf);
      });
    }
  } else {
    if constexpr (std::is_same_v<T, std::string> && std::strcmp(format, "") == 0) {
      buffer = std::ref(value);
    } else {
      buffer.clear();
      std::format_to(std::back_inserter(buffer), std::dynamic_format(gen_format), value);
    }

    auto padded_size = get_escape_bitmask(buffer);

    auto num_escapes = count_escapes(padded_size);

    const auto original_size = result.size();

    result.resize_and_overwrite(original_size + prefix_size + buffer.size() + 1 + (num_escapes * 5),
                                [&](char* buf, std::size_t) {
                                  buf += original_size;

                                  const char* original_buf = buf;

                                  std::memcpy(buf, prefix, prefix_size);

                                  buf += prefix_size;

                                  buf += copy_with_escapes(buf, buffer, padded_size);

                                  *buf = '"';

                                  return original_size + (buf + 1) - original_buf;
                                });
  }
}

template <auto name>
consteval auto get_tags() {
  std::string opening_tag;
  opening_tag.reserve(std::strlen(name) + 2);
  opening_tag += '<';
  opening_tag += name;
  opening_tag += '>';
  std::string closing_tag;
  closing_tag.reserve(std::strlen(name) + 3);
  closing_tag += "</";
  closing_tag += name;
  closing_tag += '>';

  return std::tuple{std::define_static_string(opening_tag), opening_tag.size(),
                    std::define_static_string(closing_tag), closing_tag.size()};
}

template <char const* name, bool is_cdata, char const* format>
void add_child(std::string& result, std::string& buffer, const auto& value) {
  using T = typename std::decay_t<decltype(value)>;
  static constexpr auto m_t = ^^T;

  static constexpr auto tags = get_tags<name>();
  static constexpr auto opening_tag = std::get<0>(tags);
  static constexpr auto opening_tag_size = std::get<1>(tags);
  static constexpr auto closing_tag = std::get<2>(tags);
  static constexpr auto closing_tag_size = std::get<3>(tags);
  static constexpr auto combined_size = opening_tag_size + closing_tag_size;

  const auto original_size = result.size();

  if constexpr (std::is_arithmetic_v<T> && sizeof(T) <= 64 && std::strcmp(format, "") == 0) {
    if constexpr (std::is_floating_point_v<T>) {
      static constexpr auto format_resize = 311 + combined_size;
      result.resize_and_overwrite(original_size + format_resize,
                                  [&](char* buf, std::size_t max_size) {
                                    buf += original_size;

                                    std::memcpy(buf, opening_tag, opening_tag_size);

                                    buf += opening_tag_size;

                                    auto [ptr, _] = std::to_chars(buf, buf + 311, value);

                                    std::memcpy(ptr, closing_tag, closing_tag_size);

                                    return (combined_size + (ptr - buf));
                                  });
    } else if constexpr (std::is_integral_v<T>) {
      static constexpr auto format_resize = 20 + combined_size;

      result.resize_and_overwrite(original_size + format_resize, [&](char* buf, std::size_t) {
        buf += original_size;

        std::memcpy(buf, opening_tag, opening_tag_size);

        buf += opening_tag_size;

        auto [ptr, _] = std::to_chars(buf, buf + 20, value);

        std::memcpy(ptr, closing_tag, closing_tag_size);

        return original_size + (combined_size + (ptr - buf));
      });
    }
  } else if constexpr (is_cdata) {
    if constexpr (std::strcmp(format, "") == 0 &&
                  (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)) {
      result += "<![CDATA[";
      result += value;
      result += "]]>";
    } else {
      static constexpr char const* gen_format =
          std::define_static_string(std::string("{:") + format + '}');

      result += "<![CDATA[";
      std::format_to(std::back_inserter(result), std::dynamic_format(gen_format), value);
      result += "]]>";
    }
  } else {
    static constexpr char const* gen_format =
        std::define_static_string(std::string("{:") + format + '}');

    if constexpr (std::is_same_v<T, std::string> && std::strcmp(format, "") == 0) {
      buffer = std::ref(value);
    } else {
      buffer.clear();
      std::format_to(std::back_inserter(buffer), std::dynamic_format(gen_format), value);
    }

    auto padded_size = get_escape_bitmask(buffer);

    auto num_escapes = count_escapes(padded_size);

    result.resize_and_overwrite(original_size + combined_size + buffer.size() + (num_escapes * 5),
                                [&](char* buf, std::size_t) {
                                  const auto original_buf = buf;

                                  buf += original_size;

                                  std::memcpy(buf, opening_tag, opening_tag_size);

                                  buf += opening_tag_size;

                                  buf += copy_with_escapes(buf, buffer, padded_size);

                                  std::memcpy(buf, closing_tag, closing_tag_size);

                                  return (buf + closing_tag_size) - original_buf;
                                });
  }
}

template <bool is_attribute, bool is_cdata, bool is_no_iter, char const* name, char const* format>
auto handle_stl(std::string& result, std::string& buffer, const auto& value) -> bool {
  using T = std::decay_t<decltype(value)>;

  static constexpr auto m_t = std::meta::template_of(std::meta::dealias(^^T));

  if constexpr (!is_attribute) {
    if constexpr (!is_no_iter &&
                  (m_t == ^^std::vector || m_t == ^^std::array || m_t == ^^std::inplace_vector ||
                   m_t == ^^std::deque || m_t == ^^std::forward_list || m_t == ^^std::span ||
                   m_t == ^^std::valarray || m_t == ^^std::set || m_t == ^^std::unordered_set ||
                   m_t == ^^std::multiset || m_t == ^^std::unordered_multiset)) {
      static constexpr auto tags = get_tags<name>();
      static constexpr auto start = std::get<0>(tags);
      static constexpr auto end = std::get<2>(tags);
      result += start;

      static constexpr auto single_name = std::define_static_string("element");
      static constexpr auto item_m_t = std::meta::dealias(^^decltype(value[0]));

      for (const auto& item : value) {
        add_child<single_name, is_cdata, format>(result, buffer, item);
      }
      result += end;

      return true;
    }
  }

  if constexpr (m_t == ^^std::optional) {
    if (!value.has_value()) {
      return true;
    }

    if constexpr (is_attribute) {
      add_attribute<name, format>(result, buffer, value.value());
    } else {
      add_child<name, is_cdata, format>(result, buffer, value.value());
    }

    return true;
  }

  return false;
}

template <typename T>
  requires(std::is_class_v<T>)
void to_xml(const T& value, std::string& result, std::string& buffer, bool first,
            const std::string& fixed_name = "") {
  if (first) {
    result += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  }

  static constexpr auto M = ^^T;

  static constexpr auto annotations = std::define_static_array(std::meta::annotations_of(M));

  if (!fixed_name.empty()) {
    buffer = fixed_name;
  } else {
    template for (constexpr auto a : annotations) {
      if constexpr (std::meta::template_of(std::meta::type_of(a)) == ^^::serial_xml::name) {
        static constexpr auto temp_name = std::meta::extract<typename[:std::meta::type_of(a):]>(a);
        buffer = std::string(temp_name.value);
      }
    }
    if (buffer.empty()) {
      if constexpr (std::meta::has_identifier(M)) {
        static constexpr auto temp_name = std::meta::identifier_of(M);
        buffer = std::string(temp_name);
      }
    }
  }

  std::string name{buffer};
  std::format_to(std::back_inserter(result), "<{}", name);

  static constexpr auto members = get_members<M>();
  static constexpr auto attribute_annotations = members.first;
  static constexpr auto child_annotations = members.second;

  template for (constexpr auto m_a : attribute_annotations) {
    static constexpr auto is_std = is_stl_handled<std::meta::type_of(m_a.first)>();

    static constexpr auto m = m_a.first;
    static constexpr auto m_annotations = m_a.second;

    static constexpr auto custom_format = structural_tuple::get<0>(m_annotations);

    static constexpr auto m_name = structural_tuple::get<1>(m_annotations);

    static constexpr auto view_name = std::string_view(m_name);

    if constexpr (!(std::ranges::all_of(
                      view_name,
                      [](char c) { return is_alnum(c) || c == '_' || c == '-' || c == '.'; })) ||
                  view_name[0] == '_' || view_name[0] == '-' || view_name[0] == '.' ||
                  is_num(view_name[0]) ||
                  std::ranges::starts_with(view_name, std::string_view("xml"))) {
      throw std::logic_error(std::format("Invalid XML name: '{}'", view_name));
    } else {
      if constexpr (is_std) {
        handle_stl<true, false, true, m_name,
                   (custom_format) ? custom_format : std::define_static_string("")>(result, buffer,
                                                                                    value.[:m:]);
      } else if constexpr (custom_format != nullptr) {
        add_attribute<m_name, custom_format>(result, buffer, value.[:m:]);
      } else {
        add_attribute<m_name, std::define_static_string("")>(result, buffer, value.[:m:]);
      }
    }
  }

  if constexpr (child_annotations.size() == 0) {
    result += "/>";
  } else {
    result += '>';

    template for (constexpr auto m_a : child_annotations) {
      static constexpr auto is_std = is_stl_handled<std::meta::type_of(m_a.first)>();

      static constexpr auto m = m_a.first;
      static constexpr auto m_annotations = m_a.second;

      static constexpr auto is_cdata = structural_tuple::get<0>(m_annotations);
      static constexpr auto is_no_iter = structural_tuple::get<1>(m_annotations);
      static constexpr auto is_raw = structural_tuple::get<2>(m_annotations);
      static constexpr auto is_unpack = structural_tuple::get<3>(m_annotations);

      static constexpr auto custom_format = structural_tuple::get<4>(m_annotations);
      static constexpr auto iter_names = structural_tuple::get<5>(m_annotations);

      static constexpr auto m_name = structural_tuple::get<6>(m_annotations);

      static constexpr auto view_name = std::string_view(m_name);

      if constexpr (!(std::ranges::all_of(
                        view_name,
                        [](char c) { return is_alnum(c) || c == '_' || c == '-' || c == '.'; })) ||
                    view_name[0] == '_' || view_name[0] == '-' || view_name[0] == '.' ||
                    is_num(view_name[0]) ||
                    std::ranges::starts_with(view_name, std::string_view("xml"))) {
        throw std::logic_error(std::format("Invalid XML name: '{}'", view_name));
      } else {
        if constexpr (iter_names.first == nullptr && is_std && !is_no_iter) {
          handle_stl<false, is_cdata, is_no_iter, m_name,
                     (custom_format) ? custom_format : std::define_static_string("")>(
              result, buffer, value.[:m:]);
        } else {
          if constexpr (iter_names.first != nullptr &&
                        std::meta::is_class_type(std::meta::type_of(m)) &&
                        std::ranges::range<typename[:std::meta::type_of(m):]>) {
            result.push_back('<');
            result.append(iter_names.second);
            result.push_back('>');

            for (const auto& item : value.[:m:]) {
              if constexpr (is_unpack) {
                to_xml(item, result, buffer, false, iter_names.first);
              } else {
                if constexpr (custom_format != nullptr) {
                  add_child<iter_names.first, is_cdata, custom_format>(result, buffer, item);
                } else {
                  add_child<iter_names.first, is_cdata, std::define_static_string("")>(
                      result, buffer, item);
                }
              }
            }

            result.append("</");
            result.append(iter_names.second);
            result.push_back('>');
          } else if constexpr (is_unpack) {
            to_xml(value.[:m:], result, buffer, false, m_name);
          } else {
            if constexpr (is_raw) {
              buffer.clear();
              if constexpr (custom_format != nullptr) {
                static constexpr auto gen_format =
                    std::define_static_string(std::string("{:") + custom_format + '}');
                std::format_to(std::back_inserter(buffer), std::dynamic_format(gen_format),
                               value.[:m:]);
              } else {
                std::format_to(std::back_inserter(buffer), "{}", value.[:m:]);
              }

              auto padded_size = get_escape_bitmask(buffer);

              auto num_escapes = count_escapes(padded_size);

              const auto original_size = result.size();
              result.resize_and_overwrite(
                  original_size + buffer.size() + (num_escapes * 5), [&](char* buf, std::size_t) {
                    buf += original_size;

                    return original_size + copy_with_escapes(buf, buffer, padded_size);
                  });
            } else {
              if constexpr (custom_format != nullptr) {
                add_child<m_name, is_cdata, custom_format>(result, buffer, value.[:m:]);
              } else {
                add_child<m_name, is_cdata, std::define_static_string("")>(result, buffer,
                                                                           value.[:m:]);
              }
            }
          }
        }
      }
    }

    result.append("</");
    result.append(name);
    result.push_back('>');
  }
}

export template <typename T>
  requires(std::is_class_v<T>)
auto to_xml(const T& value, bool first = true, const std::string& fixed_name = "") -> std::string {
  std::string result;
  std::string buffer;

  result.reserve(4096);
  buffer.reserve(256);

  to_xml(value, result, buffer, first, fixed_name);

  escape_flags.clear();
  escape_flags.shrink_to_fit();

  return result;
}

export auto prettify(const std::string& xml) -> std::string {
  std::string result;
  result.reserve(static_cast<std::size_t>(static_cast<double>(xml.size()) * 1.5));

  int indent_level = 0;
  auto append_indent = [&](int level) {
    for (int i = 0; i < level; ++i) {
      result += "  ";
    }
  };

  for (std::size_t i = 0; i < xml.size();) {
    if (xml[i] == '<') {
      const std::size_t tag_end = xml.find('>', i);
      if (tag_end == std::string::npos) {
        if (!result.empty() && result.back() != '\n') {
          result += '\n';
        }
        append_indent(indent_level);
        result += xml.substr(i);
        break;
      }

      const std::string_view tag(xml.data() + i, tag_end - i + 1);
      const bool is_closing_tag = tag.size() > 1 && tag[1] == '/';
      const bool is_declaration = tag.size() > 1 && tag[1] == '?';
      const bool is_comment = tag.size() > 3 && tag.substr(1, 3) == "!--";
      const bool is_cdata = tag.size() > 8 && tag.substr(1, 8) == "![CDATA[";
      const bool is_self_closing = !is_closing_tag && !is_declaration && !is_comment && !is_cdata &&
                                   tag.size() > 2 && tag[tag.size() - 2] == '/';

      if (is_closing_tag) {
        indent_level = std::max(0, indent_level - 1);
      }

      if (!result.empty() && result.back() != '\n') {
        result += '\n';
      }

      if (!is_declaration && !is_comment && !is_cdata) {
        append_indent(indent_level);
      }

      result.append(tag);

      if (!is_closing_tag && !is_declaration && !is_comment && !is_cdata && !is_self_closing) {
        ++indent_level;
      }

      result += '\n';
      i = tag_end + 1;
      continue;
    }

    const std::size_t text_end = xml.find('<', i);
    const std::size_t length = text_end == std::string::npos ? xml.size() - i : text_end - i;
    const std::string_view text(xml.data() + i, length);
    const bool has_content =
        std::ranges::any_of(text, [](unsigned char ch) { return !std::isspace(ch); });

    if (has_content) {
      if (!result.empty() && result.back() != '\n') {
        result += '\n';
      }
      append_indent(indent_level);
      result.append(text);
      result += '\n';
    }

    if (text_end == std::string::npos) {
      break;
    }
    i = text_end;
  }

  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  return result;
}
}  // namespace serial_xml