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

struct setter_ {};
export constexpr setter_ setter;

struct optional_ {};
export constexpr optional_ optional;

struct skip_ {};
export constexpr skip_ skip;

struct attribute_ {};
export constexpr attribute_ attribute;

struct no_unpack_ {};
export constexpr no_unpack_ no_unpack;

struct unpack_ {};
export constexpr unpack_ unpack;

struct no_iter_ {};
export constexpr no_iter_ no_iter;

struct raw_ {};
export constexpr raw_ raw;

struct cdata_ {};
export constexpr cdata_ cdata;

struct exclude_on_empty_ {};
export constexpr exclude_on_empty_ exclude_on_empty;

constexpr bool is_alpha(const char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

constexpr bool is_num(const char c) { return (c >= '0' && c <= '9'); }

constexpr bool is_alnum(const char c) { return is_alpha(c) || is_num(c); }

export template <std::size_t N = 1, typename F = std::nullptr_t>
struct format {
  char value[N]{};
  F function{};

  constexpr format(const char (&str)[N])
    requires std::same_as<F, std::nullptr_t>
  {
    for (std::size_t i = 0; i < N; ++i) value[i] = str[i];
  }

  constexpr format(F formatter)
    requires(!std::same_as<F, std::nullptr_t>)
      : function(formatter) {}
};

template <std::size_t N>
format(const char (&)[N]) -> format<N>;

template <typename F>
format(F) -> format<1, F>;

template <std::size_t N, typename F>
struct format_config {
  serial_xml::format<N, F> value;

  constexpr bool is_empty() const {
    if constexpr (std::same_as<F, std::nullptr_t>) {
      return std::strcmp(value.value, "") == 0;
    }
    return false;
  }
};

template <auto formatter>
std::string format_value(const auto& input) {
  if constexpr (std::same_as<decltype(formatter.value.function), std::nullptr_t>) {
    std::string result;
    if constexpr (formatter.is_empty()) {
      std::format_to(std::back_inserter(result), "{}", input);
    } else {
      static constexpr auto format_string =
          std::define_static_string(std::string("{:") + formatter.value.value + '}');
      std::format_to(std::back_inserter(result), std::dynamic_format(format_string), input);
    }
    return result;
  } else {
    return std::string(std::invoke(formatter.value.function, input));
  }
}

template <std::meta::info m>
consteval bool has_format() {
  static constexpr auto annotations = std::define_static_array(std::meta::annotations_of(m));

  template for (constexpr auto a : annotations) {
    static constexpr auto a_t = std::meta::type_of(a);
    if constexpr (std::meta::has_template_arguments(a_t) &&
                  std::meta::template_of(a_t) == ^^::serial_xml::format) {
      return true;
    }
  }
  return false;
}

template <std::meta::info m>
consteval auto get_defined_format() {
  static constexpr auto annotations = std::define_static_array(std::meta::annotations_of(m));

  template for (constexpr auto a : annotations) {
    static constexpr auto a_t = std::meta::type_of(a);
    if constexpr (std::meta::has_template_arguments(a_t) &&
                  std::meta::template_of(a_t) == ^^::serial_xml::format) {
      static constexpr auto value = std::meta::extract<typename[:a_t:]>(a);
      return format_config{value};
    }
  }

  std::unreachable();
}

template <std::meta::info m>
consteval auto get_format() {
  if constexpr (has_format<m>()) {
    return get_defined_format<m>();
  } else {
    return format_config{serial_xml::format{""}};
  }
}

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
inline decltype(auto) get_value(auto&& s) {
  return (s.[:m:]);
}

template <std::meta::info m>
  requires(std::meta::is_function(m))
inline decltype(auto) get_value(auto&& s) {
  return s.[:m:]();
}

consteval bool is_annotated_setter(std::meta::info member) {
  if (!std::meta::is_function(member) && !std::meta::is_nonstatic_data_member(member) &&
      !std::meta::is_variable(member))
    return false;
  for (auto annotation : std::meta::annotations_of(member)) {
    if (std::meta::type_of(annotation) == ^^decltype(serial_xml::setter)) return true;
  }
  return false;
}

template <std::meta::info m>
struct value_type {
  using t = std::remove_cvref_t<typename[:std::meta::type_of(m):]>;
  static constexpr auto m_t = std::meta::dealias(std::meta::type_of(m));
};

template <std::meta::info m>
  requires(std::meta::is_function(m))
struct value_type<m> {
  static consteval std::meta::info reflected_type() {
    if constexpr ((is_annotated_setter(m) || !std::meta::is_const(m)) &&
                  std::meta::parameters_of(m).size() == 1) {
      return std::meta::type_of(std::meta::parameters_of(m)[0]);
    } else {
      return std::meta::return_type_of(m);
    }
  }
  using t = std::remove_cvref_t<typename[:reflected_type():]>;
  static constexpr auto m_t = std::meta::dealias(std::meta::remove_cvref(reflected_type()));
};

template <std::meta::info m>
using value_t = typename value_type<m>::t;

template <std::meta::info m>
constexpr auto value_m_t = value_type<m>::m_t;

template <std::meta::info m>
consteval bool is_stl_handled() {
  if constexpr (get_namespace<m>() ==
                ^^std&& std::meta::has_template_arguments(std::meta::dealias(m))) {
    static constexpr auto m_t = std::meta::template_of(std::meta::dealias(m));

    if constexpr (m_t == ^^std::vector || m_t == ^^std::array || m_t == ^^std::inplace_vector ||
                  m_t == ^^std::deque || m_t == ^^std::forward_list || m_t == ^^std::span ||
                  m_t == ^^std::valarray || m_t == ^^std::optional) {
      return true;
    }
  }

  return false;
}

template <std::meta::info m, std::meta::info source = m>
consteval auto get_annotations()
    -> structural_tuple::tuple<bool, bool, bool, bool, bool, bool,
                               std::pair<char const*, char const*>, char const*, bool> {
  static constexpr auto annotations = std::define_static_array(std::meta::annotations_of(m));

  bool is_attribute = false;
  bool is_cdata = false;
  bool is_no_iter = !is_stl_handled<value_m_t<source>>();
  bool is_raw = false;
  bool is_skip = false;
  bool is_unpack = std::meta::is_class_type(value_m_t<source>) &&
                   !std::formattable<value_t<source>, char> && !is_stl_handled<value_m_t<source>>();

  bool has_custom_format_function = false;
  std::optional<std::pair<std::string, std::string>> iter_names;

  std::string name;

  bool is_exclude_on_empty = false;

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
    } else if constexpr (a_t == ^^decltype(::serial_xml::exclude_on_empty)) {
      is_exclude_on_empty = true;
    } else if constexpr (std::meta::has_template_arguments(a_t)) {
      if constexpr (std::meta::template_of(a_t) == ^^::serial_xml::format) {
        static constexpr auto format_value = std::meta::extract<typename[:a_t:]>(a);
        if constexpr (!std::same_as<decltype(format_value.function), std::nullptr_t>) {
          has_custom_format_function = true;
        }
      } else if constexpr (std::meta::template_of(a_t) == ^^::serial_xml::iter) {
        if constexpr (!std::ranges::range<value_t<source>>) {
          throw std::logic_error(
              "serial_xml::iter annotation can only be applied to range types. Member name: " +
              std::string(std::meta::identifier_of(m)));
        }

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
  }

  if constexpr (std::ranges::range<value_t<source>>) {
    if (iter_names.has_value()) {
      using m_t = std::ranges::range_value_t<value_t<source>>;
      is_unpack = std::is_class_v<m_t> && !std::formattable<m_t, char>;
    }
  }

  if (has_custom_format_function) {
    is_unpack = false;
  }

  if (name.empty()) {
    if constexpr (std::meta::has_identifier(m)) {
      static constexpr auto temp_name = std::meta::identifier_of(m);
      name = std::string(temp_name);
    } else {
      name = "field";
    }
  }

  return structural_tuple::tuple{is_attribute,
                                 is_cdata,
                                 is_no_iter,
                                 is_raw,
                                 is_skip,
                                 is_unpack,
                                 (iter_names.has_value())
                                     ? std::make_pair(std::define_static_string(iter_names->first),
                                                      std::define_static_string(iter_names->second))
                                     : std::make_pair<char const*, char const*>(nullptr, nullptr),
                                 std::define_static_string(name),
                                 is_exclude_on_empty};
}

consteval bool is_skipped_member(std::meta::info member) {
  for (auto annotation : std::meta::annotations_of(member)) {
    if (std::meta::type_of(annotation) == ^^decltype(::serial_xml::skip)) {
      return true;
    }
  }
  return false;
}

// A persistent, compile-time hash table. Build once per concrete target type, then
// resolve each mock member without expanding a target-member template loop.
struct member_map_entry {
  char const* identifier = nullptr;
  std::meta::info member{};
  bool ambiguous = false;
};

consteval std::size_t identifier_hash(std::string_view identifier) {
  std::size_t hash = 14695981039346656037ULL;
  for (unsigned char c : identifier) {
    hash = (hash ^ c) * 1099511628211ULL;
  }
  return hash;
}

consteval bool is_serializable_member(std::meta::info member) {
  if (is_annotated_setter(member)) return false;
  if (std::meta::is_nonstatic_data_member(member) || std::meta::is_variable(member)) {
    return true;
  }
  if (!std::meta::is_function(member) || std::meta::is_constructor(member) ||
      std::meta::is_destructor(member) || std::meta::is_static_member(member) ||
      !std::meta::is_const(member) || std::meta::is_rvalue_reference_qualified(member) ||
      std::meta::return_type_of(member) == ^^void) {
    return false;
  }
  for (auto parameter : std::meta::parameters_of(member)) {
    if (!std::meta::has_default_argument(parameter)) {
      return false;
    }
  }
  return true;
}

template <typename T>
consteval auto make_string_to_field_map() {
  // Includes functions as well as fields. Inaccessible members and members that
  // cannot be read from a const object do not participate in serialization.
  auto members = std::meta::members_of(^^T, std::meta::access_context::current());
  std::size_t count = 0;
  for (auto member : members) {
    if (is_serializable_member(member)) {
      ++count;
    }
  }
  std::size_t capacity = 1;
  while (capacity < count * 2) {
    capacity *= 2;
  }
  std::vector<member_map_entry> entries(capacity);
  for (auto member : members) {
    if (!is_serializable_member(member)) {
      continue;
    }
    auto identifier = std::meta::identifier_of(member);
    auto index = identifier_hash(identifier) & (capacity - 1);
    while (entries[index].identifier != nullptr &&
           std::string_view(entries[index].identifier) != identifier) {
      index = (index + 1) & (capacity - 1);
    }
    if (entries[index].identifier == nullptr) {
      entries[index] = {std::define_static_string(identifier), member, false};
    } else {
      entries[index].ambiguous = true;
    }
  }
  return std::define_static_array(entries);
}

template <typename T>
constexpr auto string_to_field_map = make_string_to_field_map<T>();

template <typename T>
consteval member_map_entry find_member(std::string_view identifier) {
  constexpr auto entries = string_to_field_map<T>;
  auto index = identifier_hash(identifier) & (entries.size() - 1);
  while (entries[index].identifier != nullptr) {
    if (std::string_view(entries[index].identifier) == identifier) {
      return entries[index];
    }
    index = (index + 1) & (entries.size() - 1);
  }
  return {};
}

template <std::meta::info member, typename T>
consteval std::meta::info resolve_member() {
  if constexpr (std::meta::parent_of(member) == std::meta::dealias(^^T)) {
    return member;
  } else {
    constexpr auto identifier = std::meta::identifier_of(member);
    constexpr auto entry = find_member<T>(identifier);
    static_assert(
        entry.identifier != nullptr,
        "No accessible field or const getter for mock member: " + std::string(identifier));
    static_assert(!entry.ambiguous, "Ambiguous mock member: " + std::string(identifier));
    if constexpr (entry.identifier != nullptr) {
      static_assert(std::meta::is_function(member) == std::meta::is_function(entry.member),
                    "Mock member must match a field or a const getter: " + std::string(identifier));
    }
    return entry.member;
  }
}

template <std::meta::info container, typename T>
consteval auto get_members() {
  static constexpr auto members = std::define_static_array(
      std::meta::members_of(container, std::meta::access_context::current()));

  std::vector<
      std::pair<std::meta::info,
                structural_tuple::tuple<bool, bool, bool, bool, std::pair<char const*, char const*>,
                                        char const*, bool>>>
      child_annotations;
  std::vector<std::pair<std::meta::info, structural_tuple::tuple<char const*>>>
      attribute_annotations;

  template for (constexpr auto m : members) {
    // A skipped placeholder need not exist on the target.
    if constexpr (is_serializable_member(m) && !is_skipped_member(m)) {
      static constexpr auto source = resolve_member<m, T>();
      static constexpr auto m_annotations = get_annotations<m, source>();

      // Invalid attribute combinations
      static_assert(
          !(structural_tuple::get<0>(m_annotations) && structural_tuple::get<5>(m_annotations)),
          "Cannot have both serial_xml::attribute and serial_xml::unpack annotations on the "
          "same member. If you did not add the unpack annotation, add the "
          "serial_xml::no_unpack annotation to the member. Member name: " +
              std::string(std::meta::identifier_of(m)));

      static_assert(
          !(structural_tuple::get<0>(m_annotations) && structural_tuple::get<1>(m_annotations)),
          "Cannot have both serial_xml::attribute and serial_xml::cdata annotations on the "
          "same member. Member name: " +
              std::string(std::meta::identifier_of(m)));

      static_assert(
          !(structural_tuple::get<0>(m_annotations) && structural_tuple::get<3>(m_annotations)),
          "Cannot have both serial_xml::attribute and serial_xml::raw annotations on the "
          "same member. Member name: " +
              std::string(std::meta::identifier_of(m)));

      // Invalid child combinations
      static_assert(
          !(structural_tuple::get<1>(m_annotations) && structural_tuple::get<3>(m_annotations)),
          "Cannot have both serial_xml::cdata and serial_xml::raw annotations on the "
          "same member. Member name: " +
              std::string(std::meta::identifier_of(m)));
      static_assert(
          !(structural_tuple::get<1>(m_annotations) && structural_tuple::get<5>(m_annotations)),
          "Cannot have both serial_xml::cdata and serial_xml::unpack annotations on the "
          "same member. Member name: " +
              std::string(std::meta::identifier_of(m)));
      static_assert(
          !(structural_tuple::get<1>(m_annotations) && !structural_tuple::get<2>(m_annotations)),
          "Cannot have both the serial_xml::cdata annotation and iteration on the "
          "same member. Member name: " +
              std::string(std::meta::identifier_of(m)));

      if constexpr (structural_tuple::get<4>(m_annotations)) {
        continue;
      }

      if constexpr (structural_tuple::get<0>(m_annotations)) {
        attribute_annotations.push_back(
            std::make_pair(m, structural_tuple::tuple{structural_tuple::get<7>(m_annotations)}));
      } else {
        child_annotations.push_back(std::make_pair(
            m, structural_tuple::tuple{
                   structural_tuple::get<1>(m_annotations), structural_tuple::get<2>(m_annotations),
                   structural_tuple::get<3>(m_annotations), structural_tuple::get<5>(m_annotations),
                   structural_tuple::get<6>(m_annotations), structural_tuple::get<7>(m_annotations),
                   structural_tuple::get<8>(m_annotations)}));
      }
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

thread_local std::vector<std::uint64_t> escape_flags;

auto get_escape_bitmask(std::string_view input) -> std::size_t {
  using simd_t = std::simd::vec<char>;
  const auto blocks = (input.size() + 63) / 64;
  escape_flags.assign(blocks, 0);
  static constexpr auto indices = std::make_index_sequence<simd_t::size()>{};
  for (std::size_t offset = 0; offset < input.size(); offset += simd_t::size()) {
    const auto count = std::min(static_cast<std::size_t>(simd_t::size()), input.size() - offset);
    auto value = std::simd::partial_load<simd_t>(input.data() + offset, count);
    auto mask = (value == '<') | (value == '>') | (value == '&') | (value == '"') | (value == '\'');
    template for (constexpr auto i : indices) {
      if (i < count && mask[static_cast<int>(i)]) {
        const auto index = offset + i;
        escape_flags[index / 64] |= std::uint64_t{1} << (index % 64);
      }
    }
  }
  return blocks;
}

auto count_escapes(std::size_t blocks) -> std::size_t {
  std::size_t count = 0;
  for (std::size_t i = 0; i < blocks; ++i) count += std::popcount(escape_flags[i]);
  return count;
}

auto copy_with_escapes(char* buffer, std::string_view input, std::size_t blocks) -> std::size_t {
  char* original = buffer;
  for (std::size_t block = 0; block < blocks; ++block) {
    const auto offset = block * 64;
    const auto count = std::min(64uz, input.size() - offset);
    auto mask = escape_flags[block];
    std::size_t previous = 0;
    while (mask) {
      const auto index = static_cast<std::size_t>(std::countr_zero(mask));
      std::memcpy(buffer, input.data() + offset + previous, index - previous);
      buffer += index - previous;
      std::string_view entity;
      switch (input[offset + index]) {
        case '<':
          entity = "&lt;";
          break;
        case '>':
          entity = "&gt;";
          break;
        case '&':
          entity = "&amp;";
          break;
        case '"':
          entity = "&quot;";
          break;
        case '\'':
          entity = "&apos;";
          break;
        default:
          std::unreachable();
      }
      std::memcpy(buffer, entity.data(), entity.size());
      buffer += entity.size();
      previous = index + 1;
      mask &= mask - 1;
    }
    std::memcpy(buffer, input.data() + offset + previous, count - previous);
    buffer += count - previous;
  }
  return static_cast<std::size_t>(buffer - original);
}

template <char const* name, auto formatter>
void add_attribute(std::string& result, std::string& buffer, const auto& value) {
  using T = std::decay_t<decltype(value)>;
  static constexpr auto m_t = ^^std::decay_t<decltype(value)>;

  static constexpr auto prefix_result = get_attribute_prefix<name>();
  static constexpr auto prefix = std::get<0>(prefix_result);
  static constexpr auto prefix_size = std::get<1>(prefix_result);
  if constexpr (formatter.is_empty() && std::is_arithmetic_v<T> && sizeof(T) <= 64) {
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

        auto [ptr, _] = std::to_chars(buf, buf + 20, +value);

        *ptr = '"';

        return original_size + prefix_size + ((ptr + 1) - buf);
      });
    }
  } else {
    if constexpr (formatter.is_empty() && std::is_same_v<T, std::string>) {
      buffer = std::ref(value);
    } else {
      buffer = format_value<formatter>(value);
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

template <char const* name, bool is_cdata, auto formatter>
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

  if constexpr (formatter.is_empty() && std::is_arithmetic_v<T> && sizeof(T) <= 64) {
    if constexpr (std::is_floating_point_v<T>) {
      static constexpr auto format_resize = 311 + combined_size;
      result.resize_and_overwrite(original_size + format_resize,
                                  [&](char* buf, std::size_t max_size) {
                                    buf += original_size;

                                    std::memcpy(buf, opening_tag, opening_tag_size);

                                    buf += opening_tag_size;

                                    auto [ptr, _] = std::to_chars(buf, buf + 311, value);

                                    std::memcpy(ptr, closing_tag, closing_tag_size);

                                    return original_size + (combined_size + (ptr - buf));
                                  });
    } else if constexpr (std::is_integral_v<T>) {
      static constexpr auto format_resize = 20 + combined_size;

      result.resize_and_overwrite(original_size + format_resize, [&](char* buf, std::size_t) {
        buf += original_size;

        std::memcpy(buf, opening_tag, opening_tag_size);

        buf += opening_tag_size;

        auto [ptr, _] = std::to_chars(buf, buf + 20, +value);

        std::memcpy(ptr, closing_tag, closing_tag_size);

        return original_size + (combined_size + (ptr - buf));
      });
    }
  } else if constexpr (is_cdata) {
    if constexpr (formatter.is_empty() &&
                  (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)) {
      result += "<![CDATA[";
      result += value;
      result += "]]>";
    } else {
      result += "<![CDATA[";
      result += format_value<formatter>(value);
      result += "]]>";
    }
  } else {
    if constexpr (formatter.is_empty() && std::is_same_v<T, std::string>) {
      buffer = std::ref(value);
    } else {
      buffer = format_value<formatter>(value);
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

template <typename Schema = void, typename T>
  requires(std::is_class_v<T>)
void to_xml(const T& value, std::string& result, std::string& buffer, bool first,
            const std::string& fixed_name = "");

template <bool is_attribute, bool is_cdata, bool is_no_iter, bool is_raw, bool is_exclude_on_empty,
          bool is_unpack, char const* name, auto formatter>
auto handle_stl(std::string& result, std::string& buffer, const auto& value) -> bool {
  using T = std::decay_t<decltype(value)>;

  static constexpr auto m_t = std::meta::template_of(std::meta::dealias(^^T));

  if constexpr (!is_attribute) {
    if constexpr (!is_no_iter &&
                  (m_t == ^^std::vector || m_t == ^^std::array || m_t == ^^std::inplace_vector ||
                   m_t == ^^std::deque || m_t == ^^std::forward_list || m_t == ^^std::span ||
                   m_t == ^^std::valarray)) {
      static constexpr auto tags = get_tags<name>();
      static constexpr auto start = std::get<0>(tags);
      static constexpr auto start_size = std::get<1>(tags);
      static constexpr auto end = std::get<2>(tags);
      if (!std::ranges::empty(value)) {
        if constexpr (!is_raw) {
          result += start;
        }

        static constexpr auto single_name = std::define_static_string("element");
        static constexpr auto item_m_t = std::meta::dealias(^^std::ranges::range_value_t<T>);

        for (const auto& item : value) {
          if constexpr (is_unpack ||
                        (!std::formattable<typename[:item_m_t:], char> && formatter.is_empty())) {
            to_xml(item, result, buffer, false, std::string(single_name));
          } else {
            add_child<single_name, is_cdata, formatter>(result, buffer, item);
          }
        }

        if constexpr (!is_raw) {
          result += end;
        }
      } else {
        if constexpr (!is_exclude_on_empty) {
          result.append(std::string_view(start).substr(0, start_size - 1)).append("/>");
        }
      }

      return true;
    }
  }

  if constexpr (m_t == ^^std::optional) {
    if (!value.has_value()) {
      return true;
    }

    if constexpr (is_attribute) {
      add_attribute<name, formatter>(result, buffer, value.value());
    } else {
      if constexpr (is_unpack) {
        to_xml(value.value(), result, buffer, false, std::string(name));
      } else {
        add_child<name, is_cdata, formatter>(result, buffer, value.value());
      }
    }

    return true;
  }

  return false;
}

template <typename Schema, typename T>
  requires(std::is_class_v<T>)
void to_xml(const T& value, std::string& result, std::string& buffer, bool first,
            const std::string& fixed_name) {
  if (first) {
    result += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  }

  using schema_type = std::conditional_t<std::is_void_v<Schema>, T, Schema>;
  static constexpr auto M = std::meta::dealias(^^schema_type);

  static constexpr auto annotations = std::define_static_array(std::meta::annotations_of(M));

  if (!fixed_name.empty()) {
    buffer = fixed_name;
  } else {
    template for (constexpr auto a : annotations) {
      if constexpr (std::meta::has_template_arguments(std::meta::type_of(a)) &&
                    std::meta::template_of(std::meta::type_of(a)) == ^^::serial_xml::name) {
        static constexpr auto temp_name = std::meta::extract<typename[:std::meta::type_of(a):]>(a);
        buffer = std::string(temp_name.value);
      }
    }
    if (buffer.empty()) {
      if constexpr (std::meta::has_identifier(std::meta::dealias(^^T))) {
        static constexpr auto temp_name = std::meta::identifier_of(std::meta::dealias(^^T));
        buffer = std::string(temp_name);
      } else if constexpr (std::meta::has_template_arguments(std::meta::dealias(^^T))) {
        static constexpr auto type_template = std::meta::template_of(std::meta::dealias(^^T));
        if constexpr (std::meta::has_identifier(type_template)) {
          buffer = std::string(std::meta::identifier_of(type_template));
        }
      }
    }
  }

  std::string name{buffer};
  std::format_to(std::back_inserter(result), "<{}", name);

  static constexpr auto members = get_members<M, T>();
  static constexpr auto attribute_annotations = members.first;
  static constexpr auto child_annotations = members.second;

  template for (constexpr auto m_a : attribute_annotations) {
    static constexpr auto m = m_a.first;
    static constexpr auto source = resolve_member<m, T>();
    static constexpr auto is_std = is_stl_handled<value_m_t<source>>();
    static constexpr auto m_annotations = m_a.second;

    static constexpr auto formatter = get_format<m>();
    static constexpr auto m_name = structural_tuple::get<0>(m_annotations);

    static constexpr auto view_name = std::string_view(m_name);

    static_assert(
        std::ranges::all_of(
            view_name, [](char c) { return is_alnum(c) || c == '_' || c == '-' || c == '.'; }) ||
            !(view_name[0] == '_' || view_name[0] == '-' || view_name[0] == '.' ||
              is_num(view_name[0]) || std::ranges::starts_with(view_name, std::string_view("xml"))),
        std::string("Invalid XML name: '") + std::string(view_name) + "'");

    if constexpr (is_std && std::meta::template_of(value_m_t<source>) == ^^std::optional) {
      handle_stl<true, false, true, false, false, false, m_name, formatter>(
          result, buffer, get_value<source>(value));
    } else {
      add_attribute<m_name, formatter>(result, buffer, get_value<source>(value));
    }
  }

  if constexpr (child_annotations.size() == 0) {
    result += "/>";
  } else {
    result += '>';

    template for (constexpr auto m_a : child_annotations) {
      static constexpr auto m = m_a.first;
      static constexpr auto source = resolve_member<m, T>();
      static constexpr auto is_std = is_stl_handled<value_m_t<source>>();
      static constexpr auto m_annotations = m_a.second;

      static constexpr auto is_cdata = structural_tuple::get<0>(m_annotations);
      static constexpr auto is_no_iter = structural_tuple::get<1>(m_annotations);
      static constexpr auto is_raw = structural_tuple::get<2>(m_annotations);
      static constexpr auto is_unpack = structural_tuple::get<3>(m_annotations);

      static constexpr auto formatter = get_format<m>();
      static constexpr auto iter_names = structural_tuple::get<4>(m_annotations);

      static constexpr auto m_name = structural_tuple::get<5>(m_annotations);

      static constexpr auto is_exclude_on_empty = structural_tuple::get<6>(m_annotations);

      static constexpr auto view_name = std::string_view(m_name);

      static_assert(
          std::ranges::all_of(
              view_name, [](char c) { return is_alnum(c) || c == '_' || c == '-' || c == '.'; }) ||
              !(view_name[0] == '_' || view_name[0] == '-' || view_name[0] == '.' ||
                is_num(view_name[0]) ||
                std::ranges::starts_with(view_name, std::string_view("xml"))),
          std::string("Invalid XML name: '") + std::string(view_name) + "'");

      if constexpr (iter_names.first == nullptr && is_std && !is_no_iter) {
        handle_stl<false, is_cdata, is_no_iter, is_raw, is_exclude_on_empty, is_unpack, m_name,
                   formatter>(result, buffer, get_value<source>(value));
      } else {
        if constexpr (iter_names.first != nullptr && std::meta::is_class_type(value_m_t<source>) &&
                      std::ranges::range<value_t<source>>) {
          if constexpr (!is_raw) {
            result.push_back('<');
            result.append(iter_names.second);
            result.push_back('>');
          }

          for (const auto& item : get_value<source>(value)) {
            if constexpr (is_unpack) {
              to_xml(item, result, buffer, false, iter_names.first);
            } else {
              add_child<iter_names.first, is_cdata, formatter>(result, buffer, item);
            }
          }

          if constexpr (!is_raw) {
            result.append("</");
            result.append(iter_names.second);
            result.push_back('>');
          }
        } else if constexpr (is_unpack) {
          to_xml(get_value<source>(value), result, buffer, false, m_name);
        } else {
          if constexpr (is_raw) {
            buffer = format_value<formatter>(get_value<source>(value));

            auto padded_size = get_escape_bitmask(buffer);

            auto num_escapes = count_escapes(padded_size);

            const auto original_size = result.size();
            result.resize_and_overwrite(
                original_size + buffer.size() + (num_escapes * 5), [&](char* buf, std::size_t) {
                  buf += original_size;

                  return original_size + copy_with_escapes(buf, buffer, padded_size);
                });
          } else {
            add_child<m_name, is_cdata, formatter>(result, buffer, get_value<source>(value));
          }
        }
      }
    }

    if (std::string_view(result).substr(result.size() - name.size() - 2, name.size() + 2) ==
        std::string("<") + name + ">") {
      result.pop_back();
      result.append("/>");
    } else {
      result.append("</");
      result.append(name);
      result.push_back('>');
    }
  }
}

export template <typename Schema = void, typename T>
  requires(std::is_class_v<T> && (std::is_void_v<Schema> || std::is_class_v<Schema>))
auto to_xml(const T& value, bool first = true, const std::string& fixed_name = "") -> std::string {
  std::string result;
  std::string buffer;

  result.reserve(4096);
  buffer.reserve(256);

  to_xml<Schema>(value, result, buffer, first, fixed_name);

  escape_flags.clear();
  escape_flags.shrink_to_fit();

  return result;
}

// String conversion deliberately does not reflect user-defined classes. Specialize
// from_string<T> for leaf types whose representation is controlled by the caller.
export class deserialization_error : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

namespace detail {
template <typename>
inline constexpr bool dependent_false = false;

template <typename T>
struct optional_type : std::false_type {};
template <typename T>
struct optional_type<std::optional<T>> : std::true_type {
  using value_type = T;
};

template <typename T>
struct scalar_type {
  using type = T;
};
template <typename T>
struct scalar_type<std::optional<T>> {
  using type = T;
};

template <typename T>
struct array_type : std::false_type {};
template <typename T, std::size_t N>
struct array_type<std::array<T, N>> : std::true_type {};

template <typename T>
struct valarray_type : std::false_type {};
template <typename T>
struct valarray_type<std::valarray<T>> : std::true_type {};

inline std::string_view trim(std::string_view text) {
  auto whitespace = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
  while (!text.empty() && whitespace(text.front())) text.remove_prefix(1);
  while (!text.empty() && whitespace(text.back())) text.remove_suffix(1);
  return text;
}

// std::format's range and tuple syntax, with nested brackets and quoted strings.
std::vector<std::string_view> split_values(std::string_view text, char open, char close) {
  text = trim(text);
  if (text.size() < 2 || text.front() != open || text.back() != close) {
    throw deserialization_error("Expected a bracketed string representation");
  }
  text.remove_prefix(1);
  text.remove_suffix(1);
  std::vector<std::string_view> values;
  if (trim(text).empty()) return values;
  std::vector<char> brackets;
  char quote = 0;
  bool escaped = false;
  std::size_t start = 0;
  for (std::size_t i = 0; i < text.size(); ++i) {
    char c = text[i];
    if (quote) {
      if (escaped)
        escaped = false;
      else if (c == '\\')
        escaped = true;
      else if (c == quote)
        quote = 0;
    } else if (c == '"' || c == '\'') {
      quote = c;
    } else if (c == '[' || c == '(' || c == '{') {
      brackets.push_back(c == '[' ? ']' : c == '(' ? ')' : '}');
    } else if (c == ']' || c == ')' || c == '}') {
      if (brackets.empty() || brackets.back() != c) {
        throw deserialization_error("Unbalanced string representation");
      }
      brackets.pop_back();
    } else if (c == ',' && brackets.empty()) {
      values.push_back(trim(text.substr(start, i - start)));
      start = i + 1;
    }
  }
  if (quote || !brackets.empty()) throw deserialization_error("Unbalanced string representation");
  values.push_back(trim(text.substr(start)));
  for (auto value : values) {
    if (value.empty()) throw deserialization_error("Empty item in string representation");
  }
  return values;
}

void append_codepoint(std::string& output, std::uint32_t c) {
  if (c > 0x10FFFF || (c >= 0xD800 && c <= 0xDFFF))
    throw deserialization_error("Invalid Unicode escape in string representation");
  if (c < 0x80)
    output += static_cast<char>(c);
  else if (c < 0x800) {
    output += static_cast<char>(0xC0 | (c >> 6));
    output += static_cast<char>(0x80 | (c & 0x3F));
  } else if (c < 0x10000) {
    output += static_cast<char>(0xE0 | (c >> 12));
    output += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
    output += static_cast<char>(0x80 | (c & 0x3F));
  } else {
    output += static_cast<char>(0xF0 | (c >> 18));
    output += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
    output += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
    output += static_cast<char>(0x80 | (c & 0x3F));
  }
}

std::string unquote(std::string_view text) {
  if (text.size() < 2 || text.front() != '"' || text.back() != '"') return std::string(text);
  std::string result;
  for (std::size_t i = 1; i + 1 < text.size(); ++i) {
    if (text[i] != '\\') {
      if (text[i] == '"') throw deserialization_error("Unescaped quote in string representation");
      result += text[i];
      continue;
    }
    if (++i + 1 >= text.size()) throw deserialization_error("Invalid quoted string escape");
    switch (text[i]) {
      case '\\':
        result += '\\';
        break;
      case '"':
        result += '"';
        break;
      case '\'':
        result += '\'';
        break;
      case 'n':
        result += '\n';
        break;
      case 'r':
        result += '\r';
        break;
      case 't':
        result += '\t';
        break;
      case 'u': {
        if (i + 1 >= text.size() || text[++i] != '{')
          throw deserialization_error("Expected braced Unicode escape");
        auto end = text.find('}', i + 1);
        if (end == std::string_view::npos || end + 1 >= text.size())
          throw deserialization_error("Unterminated Unicode escape");
        auto digits = text.substr(i + 1, end - i - 1);
        std::uint32_t codepoint{};
        auto [ptr, error] =
            std::from_chars(digits.data(), digits.data() + digits.size(), codepoint, 16);
        if (digits.empty() || error != std::errc{} || ptr != digits.data() + digits.size())
          throw deserialization_error("Invalid Unicode escape");
        append_codepoint(result, codepoint);
        i = end;
        break;
      }
      default:
        throw deserialization_error("Unsupported quoted string escape");
    }
  }
  return result;
}

template <typename T, typename E>
void append(T& container, E&& item) {
  if constexpr (requires { container.push_back(std::forward<E>(item)); }) {
    container.push_back(std::forward<E>(item));
  } else if constexpr (requires { container.insert(std::forward<E>(item)); }) {
    container.insert(std::forward<E>(item));
  } else {
    static_assert(dependent_false<T>, "Deserialization requires an owning, writable container");
  }
}

template <typename T, typename Parse>
T make_range(std::size_t count, Parse parse) {
  T result{};
  if constexpr (requires { T::capacity(); }) {
    if (count > T::capacity())
      throw deserialization_error("Too many elements for fixed-capacity container");
  }
  if constexpr (requires { result.reserve(count); }) result.reserve(count);
  if constexpr (array_type<T>::value) {
    if (count != result.size()) throw deserialization_error("Wrong number of array elements");
    for (std::size_t i = 0; i < count; ++i) result[i] = parse(i);
  } else if constexpr (valarray_type<T>::value) {
    result.resize(count);
    for (std::size_t i = 0; i < count; ++i) result[i] = parse(i);
  } else if constexpr (requires { result.before_begin(); }) {
    auto tail = result.before_begin();
    for (std::size_t i = 0; i < count; ++i) tail = result.insert_after(tail, parse(i));
  } else {
    for (std::size_t i = 0; i < count; ++i) append(result, parse(i));
  }
  return result;
}
}  // namespace detail

export template <typename T>
T from_string(std::string_view text);

namespace detail {
template <typename T>
T range_item(std::string_view text) {
  if constexpr (std::same_as<T, std::string>) {
    return unquote(text);
  } else if constexpr (std::same_as<T, char>) {
    if (text.size() >= 2 && text.front() == '\'' && text.back() == '\'') {
      auto value = unquote(std::string("\"") + std::string(text.substr(1, text.size() - 2)) + "\"");
      if (value.size() != 1) throw deserialization_error("Expected one character");
      return value.front();
    }
    return from_string<T>(text);
  } else {
    return from_string<T>(text);
  }
}

template <typename T, std::size_t... I>
T make_tuple(const std::vector<std::string_view>& values, std::index_sequence<I...>) {
  return T{range_item<std::remove_cvref_t<std::tuple_element_t<I, T>>>(values[I])...};
}
}  // namespace detail

export template <typename T>
T from_string(std::string_view text) {
  if constexpr (std::same_as<T, std::string>) {
    return std::string(text);
  } else if constexpr (std::same_as<T, bool>) {
    text = detail::trim(text);
    if (text == "true" || text == "1") return true;
    if (text == "false" || text == "0") return false;
    throw deserialization_error("Invalid boolean value");
  } else if constexpr (std::is_arithmetic_v<T>) {
    text = detail::trim(text);
    if (text.empty()) throw deserialization_error("Empty numeric value");
    T value{};
    auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size() || text.empty()) {
      throw deserialization_error("Invalid or out-of-range numeric value: " + std::string(text));
    }
    return value;
  } else if constexpr (detail::optional_type<T>::value) {
    return T{from_string<typename T::value_type>(text)};
  } else if constexpr (requires { typename std::tuple_size<T>::type; } &&
                       !detail::array_type<T>::value) {
    auto values = detail::split_values(text, '(', ')');
    constexpr auto count = std::tuple_size_v<T>;
    if (values.size() != count) throw deserialization_error("Wrong number of tuple elements");
    return detail::make_tuple<T>(values, std::make_index_sequence<count>{});
  } else if constexpr (requires { typename T::value_type; } &&
                       (std::ranges::range<T> || detail::valarray_type<T>::value)) {
    // Associative maps use {key: value}; sequences use [value, ...].
    if constexpr (requires { typename T::mapped_type; }) {
      auto values = detail::split_values(text, '{', '}');
      T result;
      for (auto entry : values) {
        // Locate the separator outside quoted strings and nested values.
        std::size_t separator = std::string_view::npos;
        char quote = 0;
        int depth = 0;
        bool escape = false;
        for (std::size_t i = 0; i < entry.size(); ++i) {
          char c = entry[i];
          if (quote) {
            if (escape)
              escape = false;
            else if (c == '\\')
              escape = true;
            else if (c == quote)
              quote = 0;
          } else if (c == '"' || c == '\'')
            quote = c;
          else if (c == '[' || c == '(' || c == '{')
            ++depth;
          else if (c == ']' || c == ')' || c == '}')
            --depth;
          else if (c == ':' && depth == 0) {
            separator = i;
            break;
          }
        }
        if (separator == std::string_view::npos)
          throw deserialization_error("Missing map separator");
        result.emplace(
            detail::range_item<typename T::key_type>(detail::trim(entry.substr(0, separator))),
            detail::range_item<typename T::mapped_type>(detail::trim(entry.substr(separator + 1))));
      }
      return result;
    } else {
      constexpr bool is_set = requires { typename T::key_type; };
      auto values = detail::split_values(text, is_set ? '{' : '[', is_set ? '}' : ']');
      return detail::make_range<T>(values.size(), [&](std::size_t i) {
        return detail::range_item<typename T::value_type>(values[i]);
      });
    }
  } else {
    static_assert(detail::dependent_false<T>,
                  "Implement serial_xml::from_string<T>(std::string_view) for this leaf type");
  }
}

namespace detail {
// Text and names borrow the input or the reader's arena, which outlives reconstruction.
struct xml_text {
  std::string_view value;
  bool cdata = false;
};
// A direct bump allocator avoids virtual allocation calls for the parser's small records.
class xml_arena final : public std::pmr::memory_resource {
  alignas(std::max_align_t) std::array<std::byte, 8192> storage_;
  std::size_t used_ = 0;
  std::pmr::monotonic_buffer_resource overflow_;

  void* do_allocate(std::size_t size, std::size_t alignment) override {
    return allocate_record(size, alignment);
  }
  void do_deallocate(void*, std::size_t, std::size_t) override {}
  bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
    return this == &other;
  }

 public:
  void* allocate_record(std::size_t size, std::size_t alignment) {
    if (alignment <= alignof(std::max_align_t)) {
      auto offset = (used_ + alignment - 1) & ~(alignment - 1);
      if (offset <= storage_.size() && size <= storage_.size() - offset) {
        used_ = offset + size;
        return storage_.data() + offset;
      }
    }
    return overflow_.allocate(size, alignment);
  }
};

// Append-only arena storage keeps parsed nodes stable without reallocating or moving siblings.
// Its elements contain only borrowed views and other arena lists; the reader owns all storage.
template <typename T>
class xml_list {
  struct link {
    link* next = nullptr;
    T value;
    template <typename... Args>
    explicit link(Args&&... args) : value(std::forward<Args>(args)...) {}
  };
  link* first_ = nullptr;
  link* last_ = nullptr;

 public:
  struct iterator {
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    link* current = nullptr;
    const T& operator*() const { return current->value; }
    const T* operator->() const { return &current->value; }
    iterator& operator++() {
      current = current->next;
      return *this;
    }
    iterator operator++(int) {
      auto old = *this;
      ++*this;
      return old;
    }
    bool operator==(const iterator&) const = default;
  };
  iterator begin() const { return {first_}; }
  iterator end() const { return {}; }
  bool empty() const { return first_ == nullptr; }
  std::size_t size() const { return std::distance(begin(), end()); }
  const T& front() const { return first_->value; }
  template <typename... Args>
  T& emplace_back(xml_arena* resource, Args&&... args) {
    static_assert(std::is_trivially_destructible_v<T>);
    auto* item = std::construct_at(
        static_cast<link*>(resource->allocate_record(sizeof(link), alignof(link))),
        std::forward<Args>(args)...);
    if (last_)
      last_->next = item;
    else
      first_ = item;
    last_ = item;
    return item->value;
  }
  void push_back(xml_arena* resource, const T& value) { emplace_back(resource, value); }
};
struct xml_node {
  std::string_view name;
  xml_arena* resource;
  xml_list<std::pair<std::string_view, std::string_view>> attributes;
  xml_list<xml_node> children;
  xml_list<xml_text> text;

  explicit xml_node(xml_arena* arena) : resource(arena) {}
};

// An XML 1.0 reader without DTD validation. External entities are unsupported;
// namespace names (including prefixes) are matched literally.
class xml_reader {
 public:
  explicit xml_reader(std::string_view input) : input_(input) {}

  inline xml_node read() {
    if (input_.starts_with("\xEF\xBB\xBF")) position_ = 3;
    whitespace();
    if (starts("<?xml") && input_.size() > position_ + 5 &&
        trim(input_.substr(position_ + 5, 1)).empty())
      declaration();
    miscellaneous();
    xml_node root(&resource_);
    element(root, 0);
    miscellaneous();
    if (position_ != input_.size()) fail("Trailing content after root element");
    return root;
  }

 private:
  // Small documents allocate their temporary tree entirely on the stack.
  // Large documents transparently spill into the monotonic resource's upstream allocator.
  xml_arena resource_;
  std::string_view input_;
  std::size_t position_ = 0;

  [[noreturn]] void fail(std::string_view message) const {
    throw deserialization_error(std::string(message) + " at byte " + std::to_string(position_));
  }
  bool starts(std::string_view value) const { return input_.substr(position_).starts_with(value); }
  [[gnu::always_inline]] inline void whitespace() {
    while (position_ < input_.size() && (input_[position_] == ' ' || input_[position_] == '\t' ||
                                         input_[position_] == '\r' || input_[position_] == '\n'))
      ++position_;
  }
  void expect(std::string_view value) {
    if (!starts(value)) fail("Expected " + std::string(value));
    position_ += value.size();
  }
  static bool valid_character(std::uint32_t c) {
    return c == 9 || c == 10 || c == 13 || (c >= 0x20 && c <= 0xD7FF) ||
           (c >= 0xE000 && c <= 0xFFFD) || (c >= 0x10000 && c <= 0x10FFFF);
  }
  static bool plain_ascii(std::string_view value, bool entities) {
    constexpr std::uint64_t high_bits = 0x8080808080808080ULL;
    constexpr std::uint64_t spaces = 0x2020202020202020ULL;
    constexpr std::uint64_t ampersands = 0x2626262626262626ULL;
    constexpr std::uint64_t ones = 0x0101010101010101ULL;
    std::size_t offset = 0;
    for (; value.size() - offset >= sizeof(std::uint64_t); offset += sizeof(std::uint64_t)) {
      std::uint64_t bytes;
      std::memcpy(&bytes, value.data() + offset, sizeof(bytes));
      // Any non-ASCII byte or character below space needs the full validator.
      if ((bytes & high_bits) || ((bytes - spaces) & ~bytes & high_bits)) return false;
      if (entities) {
        auto difference = bytes ^ ampersands;
        if ((difference - ones) & ~difference & high_bits) return false;
      }
    }
    for (; offset < value.size(); ++offset) {
      auto c = static_cast<unsigned char>(value[offset]);
      if (c < 0x20 || c >= 0x80 || (entities && c == '&')) return false;
    }
    return true;
  }

  std::string_view decode(std::string_view value, bool entities = true, bool attribute = false) {
    if (plain_ascii(value, entities)) return value;
    std::pmr::string output(&resource_);
    bool changed = false;
    auto begin_output = [&](std::size_t index) {
      if (!changed) {
        output.assign(value.substr(0, index));
        changed = true;
      }
    };
    for (std::size_t i = 0; i < value.size(); ++i) {
      unsigned char c = value[i];
      if (c == '&' && entities) {
        begin_output(i);
        auto end = value.find(';', i + 1);
        if (end == std::string_view::npos) fail("Unterminated entity reference");
        auto entity = value.substr(i + 1, end - i - 1);
        if (entity == "amp")
          output += '&';
        else if (entity == "lt")
          output += '<';
        else if (entity == "gt")
          output += '>';
        else if (entity == "quot")
          output += '\"';
        else if (entity == "apos")
          output += '\'';
        else if (entity.starts_with('#')) {
          entity.remove_prefix(1);
          int base = 10;
          if (entity.starts_with('x')) {
            base = 16;
            entity.remove_prefix(1);
          }
          std::uint32_t codepoint{};
          auto [ptr, error] =
              std::from_chars(entity.data(), entity.data() + entity.size(), codepoint, base);
          if (entity.empty() || error != std::errc{} || ptr != entity.data() + entity.size())
            fail("Invalid character reference");
          if (!valid_character(codepoint)) fail("Invalid XML character reference");
          // Encoded references need at most four UTF-8 bytes.
          if (codepoint < 0x80)
            output += static_cast<char>(codepoint);
          else {
            unsigned count = codepoint < 0x800 ? 2 : codepoint < 0x10000 ? 3 : 4;
            output += static_cast<char>((count == 2   ? 0xC0
                                         : count == 3 ? 0xE0
                                                      : 0xF0) |
                                        (codepoint >> (6 * (count - 1))));
            for (unsigned j = count - 1; j > 0; --j)
              output += static_cast<char>(0x80 | ((codepoint >> (6 * (j - 1))) & 0x3F));
          }
        } else
          fail("Unknown entity reference");
        i = end;
      } else if (c < 0x80) {
        if (!valid_character(c)) fail("Invalid XML character");
        if (c == '\r' || (attribute && (c == '\n' || c == '\t'))) {
          begin_output(i);
          output += attribute ? ' ' : '\n';
          if (c == '\r' && i + 1 < value.size() && value[i + 1] == '\n') ++i;
        } else if (changed)
          output += static_cast<char>(c);
      } else {
        unsigned length = c >= 0xC2 && c <= 0xDF   ? 2
                          : c >= 0xE0 && c <= 0xEF ? 3
                          : c >= 0xF0 && c <= 0xF4 ? 4
                                                   : 0;
        if (!length || i + length > value.size()) fail("Invalid UTF-8");
        std::uint32_t codepoint = c & (0x7F >> length);
        for (unsigned j = 1; j < length; ++j) {
          auto next = static_cast<unsigned char>(value[i + j]);
          if ((next & 0xC0) != 0x80) fail("Invalid UTF-8");
          codepoint = (codepoint << 6) | (next & 0x3F);
        }
        if ((length == 2 && codepoint < 0x80) || (length == 3 && codepoint < 0x800) ||
            (length == 4 && codepoint < 0x10000) || !valid_character(codepoint))
          fail("Invalid UTF-8");
        if (changed) output.append(value.substr(i, length));
        i += length - 1;
      }
    }
    if (!changed) return value;
    auto* data = static_cast<char*>(resource_.allocate(output.size(), alignof(char)));
    std::memcpy(data, output.data(), output.size());
    return {data, output.size()};
  }
  // Keep ASCII scanning in the recursive parser; Unicode validation is a cold call.
  [[gnu::always_inline]] inline std::string_view name() {
    auto begin = position_;
    static constexpr auto classes = [] {
      std::array<unsigned char, 256> result{};
      for (unsigned c = 0; c < result.size(); ++c) {
        bool first =
            (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == ':' || c >= 0x80;
        result[c] =
            (first ? 1 : 0) | ((first || (c >= '0' && c <= '9') || c == '-' || c == '.') ? 2 : 0);
      }
      return result;
    }();
    if (position_ == input_.size() || !(classes[static_cast<unsigned char>(input_[position_])] & 1))
      fail("Invalid XML name");
    bool unicode = static_cast<unsigned char>(input_[position_]) >= 0x80;
    ++position_;
    while (position_ < input_.size()) {
      unsigned char c = input_[position_];
      unicode = unicode || c >= 0x80;
      if (!(classes[c] & 2)) break;
      ++position_;
    }
    auto result = input_.substr(begin, position_ - begin);
    if (!unicode) return result;  // The scanning loop already validated ASCII names.
    validate_unicode_name(result);
    return result;
  }
  void validate_unicode_name(std::string_view result) {
    decode(result, false);
    // XML NameStartChar/NameChar ranges are wider than ASCII but narrower than UTF-8.
    for (std::size_t i = 0; i < result.size();) {
      auto c = static_cast<unsigned char>(result[i]);
      std::uint32_t codepoint = c;
      unsigned length = 1;
      if (c >= 0x80) {
        length = c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
        codepoint = c & (0x7F >> length);
        for (unsigned j = 1; j < length; ++j)
          codepoint = (codepoint << 6) | (static_cast<unsigned char>(result[i + j]) & 0x3F);
      }
      bool start =
          codepoint == ':' || codepoint == '_' || (codepoint >= 'A' && codepoint <= 'Z') ||
          (codepoint >= 'a' && codepoint <= 'z') || (codepoint >= 0xC0 && codepoint <= 0xD6) ||
          (codepoint >= 0xD8 && codepoint <= 0xF6) || (codepoint >= 0xF8 && codepoint <= 0x2FF) ||
          (codepoint >= 0x370 && codepoint <= 0x37D) ||
          (codepoint >= 0x37F && codepoint <= 0x1FFF) ||
          (codepoint >= 0x200C && codepoint <= 0x200D) ||
          (codepoint >= 0x2070 && codepoint <= 0x218F) ||
          (codepoint >= 0x2C00 && codepoint <= 0x2FEF) ||
          (codepoint >= 0x3001 && codepoint <= 0xD7FF) ||
          (codepoint >= 0xF900 && codepoint <= 0xFDCF) ||
          (codepoint >= 0xFDF0 && codepoint <= 0xFFFD) ||
          (codepoint >= 0x10000 && codepoint <= 0xEFFFF);
      bool continuation = start || codepoint == '-' || codepoint == '.' ||
                          (codepoint >= '0' && codepoint <= '9') || codepoint == 0xB7 ||
                          (codepoint >= 0x300 && codepoint <= 0x36F) ||
                          (codepoint >= 0x203F && codepoint <= 0x2040);
      if (!(i == 0 ? start : continuation)) fail("Invalid XML name character");
      i += length;
    }
  }
  void comment() {
    expect("<!--");
    auto end = input_.find("--", position_);
    if (end == std::string_view::npos || input_.substr(end, 3) != "-->")
      fail("Malformed XML comment");
    decode(input_.substr(position_, end - position_), false);
    position_ = end + 3;
  }
  void declaration() {
    expect("<?xml");
    std::vector<std::pair<std::string, std::string>> attributes;
    for (;;) {
      auto before = position_;
      whitespace();
      if (starts("?>")) {
        position_ += 2;
        break;
      }
      if (before == position_) fail("Missing whitespace in XML declaration");
      auto key = name();
      whitespace();
      expect("=");
      whitespace();
      if (position_ == input_.size() || (input_[position_] != '\"' && input_[position_] != '\''))
        fail("Expected quoted XML declaration value");
      char quote = input_[position_++];
      auto end = input_.find(quote, position_);
      if (end == std::string_view::npos) fail("Unterminated XML declaration");
      attributes.emplace_back(std::move(key), input_.substr(position_, end - position_));
      position_ = end + 1;
    }
    if (attributes.empty() || attributes[0].first != "version" || attributes[0].second != "1.0")
      fail("XML declaration requires version 1.0");
    std::size_t index = 1;
    if (index < attributes.size() && attributes[index].first == "encoding") {
      auto encoding = attributes[index++].second;
      for (char& c : encoding)
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
      if (encoding != "UTF-8") fail("Only UTF-8 XML is supported");
    }
    if (index < attributes.size() && attributes[index].first == "standalone") {
      auto value = attributes[index++].second;
      if (value != "yes" && value != "no") fail("Invalid standalone declaration");
    }
    if (index != attributes.size()) fail("Invalid XML declaration attributes");
  }
  void processing_instruction() {
    expect("<?");
    auto target = std::string(name());
    for (char& c : target)
      if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    if (target == "xml") fail("Reserved processing instruction target");
    auto end = input_.find("?>", position_);
    if (end == std::string_view::npos) fail("Unterminated processing instruction");
    if (position_ != end && trim(input_.substr(position_, 1)).size())
      fail("Invalid processing instruction");
    decode(input_.substr(position_, end - position_), false);
    position_ = end + 2;
  }
  void miscellaneous() {
    for (;;) {
      whitespace();
      if (starts("<!--"))
        comment();
      else if (starts("<?")) {
        processing_instruction();
      } else
        break;
    }
  }
  [[gnu::always_inline]] inline std::string_view element_text() {
    auto begin = position_;
    bool plain = true;
    while (position_ < input_.size() && input_[position_] != '<') {
      auto c = static_cast<unsigned char>(input_[position_]);
      plain = plain && c >= 0x20 && c < 0x80 && c != '&';
      if (c == ']' && input_.substr(position_).starts_with("]]>"))
        fail("Unescaped CDATA terminator");
      ++position_;
    }
    if (position_ == input_.size()) fail("Unterminated element text");
    auto value = input_.substr(begin, position_ - begin);
    return plain ? value : decode(value);
  }

  void element(xml_node& node, std::size_t depth) {
    if (depth >= 256) fail("XML nesting limit exceeded");
    expect("<");
    node.name = name();
    for (;;) {
      auto before = position_;
      whitespace();
      if (starts("/>")) {
        position_ += 2;
        return;
      }
      if (starts(">")) {
        ++position_;
        break;
      }
      if (before == position_) fail("Missing whitespace before attribute");
      auto key = name();
      for (const auto& existing : node.attributes)
        if (existing.first == key) fail("Duplicate attribute");
      whitespace();
      expect("=");
      whitespace();
      if (position_ == input_.size() || (input_[position_] != '"' && input_[position_] != '\''))
        fail("Expected quoted attribute");
      char quote = input_[position_++];
      auto end = input_.find(quote, position_);
      if (end == std::string_view::npos) fail("Unterminated attribute");
      auto value = input_.substr(position_, end - position_);
      if (value.contains('<')) fail("Unescaped '<' in attribute");
      node.attributes.emplace_back(node.resource, key, decode(value, true, true));
      position_ = end + 1;
    }
    // A plain text leaf needs no markup dispatch or second scan of its closing name.
    if (position_ < input_.size() && input_[position_] != '<') {
      node.text.push_back(node.resource, {element_text(), false});
    }
    for (;;) {
      if (position_ == input_.size()) fail("Unterminated element");
      if (input_[position_] == '<' && position_ + 1 < input_.size() &&
          input_[position_ + 1] == '/') {
        position_ += 2;
        if (!input_.substr(position_).starts_with(node.name)) fail("Mismatched closing tag");
        position_ += node.name.size();
        whitespace();
        expect(">");
        return;
      } else if (input_[position_] == '<' && position_ + 1 < input_.size() &&
                 input_[position_ + 1] == '!') {
        if (starts("<!--")) {
          comment();
          continue;
        }
        if (!starts("<![CDATA[")) fail("DTD and markup declarations are unsupported");
        position_ += 9;
        auto end = input_.find("]]>", position_);
        if (end == std::string_view::npos) fail("Unterminated CDATA");
        node.text.push_back(node.resource,
                            {decode(input_.substr(position_, end - position_), false), true});
        position_ = end + 3;
      } else if (input_[position_] == '<' && position_ + 1 < input_.size() &&
                 input_[position_ + 1] == '?') {
        processing_instruction();
      } else if (input_[position_] == '<') {
        auto& nested = node.children.emplace_back(&resource_, &resource_);
        element(nested, depth + 1);
      } else {
        node.text.push_back(node.resource, {element_text(), false});
      }
    }
  }
};

// Constant schema names let the compiler specialize these comparisons.
[[gnu::always_inline]] inline const xml_node* child(const xml_node& node, std::string_view name) {
  const xml_node* found = nullptr;
  for (const auto& candidate : node.children) {
    if (candidate.name == name) {
      if (found) throw deserialization_error("Duplicate element: " + std::string(name));
      found = &candidate;
    }
  }
  return found;
}
[[gnu::always_inline]] inline std::string_view scalar_text(const xml_node& node) {
  if (!node.children.empty())
    throw deserialization_error("Unexpected nested element in " + std::string(node.name));
  if (node.text.empty()) return {};
  if (node.text.size() == 1) return node.text.front().value;
  std::size_t size = 0;
  for (const auto& part : node.text) size += part.value.size();
  auto* data = static_cast<char*>(node.resource->allocate_record(size, alignof(char)));
  auto* output = data;
  for (const auto& part : node.text) {
    std::memcpy(output, part.value.data(), part.value.size());
    output += part.value.size();
  }
  return {data, size};
}

template <std::meta::info M, typename Annotation>
consteval bool annotated() {
  for (auto annotation : std::meta::annotations_of(M))
    if (std::meta::type_of(annotation) == ^^Annotation) return true;
  return false;
}

template <std::meta::info M>
consteval bool is_setter() {
  return is_annotated_setter(M);
}

// An explicit setter's value type is its single parameter, not its return type.
template <std::meta::info M>
consteval std::meta::info input_type() {
  if constexpr (is_setter<M>()) {
    static_assert(std::meta::is_function(M), "serial_xml::setter requires a member function");
    constexpr auto parameters = std::define_static_array(std::meta::parameters_of(M));
    static_assert(parameters.size() == 1, "serial_xml::setter requires exactly one parameter");
    static_assert(!std::meta::is_static_member(M),
                  "serial_xml::setter requires a non-static method");
    return std::meta::type_of(parameters[0]);
  } else
    return value_m_t<M>;
}

template <std::meta::info Getter, typename T>
consteval std::meta::info matching_setter() {
  auto getter_name = std::string(std::meta::identifier_of(Getter));
  std::string setter_name = "set_" + getter_name;
  if (getter_name.starts_with("get_"))
    setter_name = "set_" + getter_name.substr(4);
  else if (getter_name.starts_with("get") && getter_name.size() > 3 && getter_name[3] >= 'A' &&
           getter_name[3] <= 'Z')
    setter_name = "set" + getter_name.substr(3);
  std::meta::info found{};
  for (auto candidate : std::meta::members_of(^^T, std::meta::access_context::current())) {
    if (!std::meta::is_function(candidate) || !std::meta::has_identifier(candidate) ||
        std::meta::is_constructor(candidate) || std::meta::is_destructor(candidate) ||
        std::meta::is_static_member(candidate) || std::meta::is_const(candidate) ||
        std::meta::is_rvalue_reference_qualified(candidate))
      continue;
    auto id = std::meta::identifier_of(candidate);
    if (id != getter_name && id != setter_name) continue;
    auto parameters = std::meta::parameters_of(candidate);
    if (parameters.size() != 1) continue;
    if (std::meta::dealias(std::meta::remove_cvref(std::meta::type_of(parameters[0]))) !=
        std::meta::dealias(std::meta::remove_cvref(value_m_t<Getter>)))
      continue;
    if (found != std::meta::info{}) throw std::logic_error("Ambiguous setter for " + getter_name);
    found = candidate;
  }
  return found;
}

template <typename T, typename Schema = void>
void read_object(const xml_node& node, T& result);

template <typename T, bool Unpack>
T read_value(const xml_node& node) {
  if constexpr (Unpack) {
    T result{};
    read_object(node, result);
    return result;
  } else
    return from_string<T>(scalar_text(node));
}

template <typename T, bool Unpack>
T read_range(const xml_node& node, std::string_view item_name, bool raw = false) {
  std::size_t count = 0;
  for (const auto& item : node.children) {
    if (item.name == item_name)
      ++count;
    else if (!raw)
      throw deserialization_error("Unexpected range element: " + std::string(item.name));
  }
  if (!raw) {
    for (const auto& text : node.text)
      if (!trim(text.value).empty()) throw deserialization_error("Unexpected text in range");
  }
  auto item = node.children.begin();
  return make_range<T>(count, [&](std::size_t) {
    while (item->name != item_name) ++item;
    return read_value<typename T::value_type, Unpack>(*item++);
  });
}

template <typename T, typename Schema>
std::string root_name() {
  constexpr auto type = std::meta::dealias(^^T);
  constexpr auto schema = std::meta::dealias(^^Schema);
  template for (constexpr auto annotation :
                std::define_static_array(std::meta::annotations_of(schema))) {
    constexpr auto a_type = std::meta::type_of(annotation);
    if constexpr (std::meta::has_template_arguments(a_type) &&
                  std::meta::template_of(a_type) == ^^serial_xml::name) {
      constexpr auto a = std::meta::extract<typename[:a_type:]>(annotation);
      return a.value;
    }
  }
  if constexpr (std::meta::has_identifier(type))
    return std::string(std::meta::identifier_of(type));
  else if constexpr (std::meta::has_template_arguments(type))
    return std::string(std::meta::identifier_of(std::meta::template_of(type)));
  else
    return {};
}
}  // namespace detail

namespace detail {
template <std::meta::info M>
consteval bool has_xml_name() {
  for (auto annotation : std::meta::annotations_of(M)) {
    auto type = std::meta::type_of(annotation);
    if (std::meta::has_template_arguments(type) &&
        std::meta::template_of(type) == ^^serial_xml::name)
      return true;
  }
  return false;
}

template <std::meta::info M, std::meta::info Source>
consteval const char* input_member_name() {
  constexpr auto annotations = get_annotations<M, Source>();
  constexpr auto name = structural_tuple::get<7>(annotations);
  if constexpr (is_setter<M>() && !has_xml_name<M>()) {
    std::string field_name(name);
    if (field_name.starts_with("set_"))
      field_name.erase(0, 4);
    else if (field_name.starts_with("set") && field_name.size() > 3 && field_name[3] >= 'A' &&
             field_name[3] <= 'Z') {
      field_name.erase(0, 3);
      field_name[0] = static_cast<char>(field_name[0] - 'A' + 'a');
    }
    return std::define_static_string(field_name);
  } else
    return name;
}

template <std::meta::info M, std::meta::info Source, typename V>
std::optional<V> read_member(const xml_node& node, std::size_t& cdata_index,
                             bool& raw_scalar_seen) {
  constexpr auto a = get_annotations<M, Source>();
  constexpr bool attribute = structural_tuple::get<0>(a);
  constexpr bool cdata = structural_tuple::get<1>(a);
  constexpr bool no_iter = structural_tuple::get<2>(a);
  constexpr bool raw = structural_tuple::get<3>(a);
  constexpr bool unpack = structural_tuple::get<5>(a);
  constexpr auto formatter = get_format<M>();
  constexpr auto names = structural_tuple::get<6>(a);
  constexpr bool excluded = structural_tuple::get<8>(a);
  constexpr bool iterating =
      !attribute &&
      (names.first != nullptr || (is_stl_handled<^^V>() && !no_iter && !optional_type<V>::value));
  constexpr bool optional = annotated<M, decltype(serial_xml::optional)>();
  constexpr std::string_view field_name = input_member_name<M, Source>();
  static_assert(!(attribute && (cdata || raw || unpack)), "Invalid setter attribute annotations");
  static_assert(!(cdata && (raw || unpack || !no_iter)), "Invalid setter CDATA annotations");
  const xml_node* selected = nullptr;
  xml_node text_node(node.resource);
  if constexpr (attribute) {
    for (const auto& [key, value] : node.attributes) {
      if (key == field_name) {
        text_node.text.push_back(text_node.resource, {value, false});
        selected = &text_node;
      }
    }
  } else if constexpr (iterating) {
    constexpr bool item_unpack =
        names.first != nullptr
            ? unpack
            : unpack || (!std::formattable<typename V::value_type, char> && formatter.is_empty());
    std::string_view item_name = names.first != nullptr ? names.first : "element";
    if constexpr (raw) {
      if constexpr (optional) {
        if (std::ranges::none_of(node.children,
                                 [&](const auto& item) { return item.name == item_name; }))
          return std::nullopt;
      }
      return std::optional<V>{std::in_place, read_range<V, item_unpack>(node, item_name, true)};
    } else {
      std::string_view wrapper = names.second != nullptr ? names.second : field_name;
      selected = child(node, wrapper);
      if (selected)
        return std::optional<V>{std::in_place, read_range<V, item_unpack>(*selected, item_name)};
    }
  } else if constexpr (raw && !unpack && (!optional_type<V>::value || no_iter)) {
    if (raw_scalar_seen)
      throw deserialization_error("Multiple raw scalar members cannot be separated");
    raw_scalar_seen = true;
    for (const auto& part : node.text)
      if (!part.cdata) text_node.text.push_back(text_node.resource, part);
    if constexpr (optional) {
      if (text_node.text.empty()) return std::nullopt;
    }
    selected = &text_node;
  } else if constexpr (cdata && !(std::is_arithmetic_v<typename scalar_type<V>::type> &&
                                  formatter.is_empty())) {
    // to_xml emits CDATA directly into the parent, without a member tag.
    std::size_t current = 0;
    for (const auto& part : node.text) {
      if (part.cdata && current++ == cdata_index) {
        text_node.text.push_back(text_node.resource, part);
        selected = &text_node;
        ++cdata_index;
        break;
      }
    }
  } else
    selected = child(node, field_name);

  if (!selected) {
    if constexpr (optional)
      return std::nullopt;
    else if constexpr (optional_type<V>::value)
      return std::optional<V>{std::in_place, V{}};
    else if constexpr (excluded && iterating && names.first == nullptr && !attribute)
      return std::optional<V>{std::in_place, V{}};
    else
      throw deserialization_error("Missing required member: " + std::string(node.name) + "." +
                                  std::string(field_name));
  }
  if constexpr (iterating) {
    std::unreachable();  // A present range returned above; an absent range was handled above.
  } else if constexpr (optional_type<V>::value && unpack && !attribute) {
    using E = typename V::value_type;
    return std::optional<V>{std::in_place, read_value<E, true>(*selected)};
  } else
    return std::optional<V>{std::in_place, read_value < V, unpack && !attribute > (*selected)};
}

template <std::meta::info M, typename T>
consteval std::meta::info resolve_input_member() {
  if constexpr (!is_setter<M>())
    return resolve_member<M, T>();
  else if constexpr (std::meta::parent_of(M) == std::meta::dealias(^^T))
    return M;
  else {
    std::meta::info found{};
    for (auto candidate : std::meta::members_of(^^T, std::meta::access_context::current())) {
      if (!std::meta::is_function(candidate) || !std::meta::has_identifier(candidate) ||
          std::meta::is_constructor(candidate) || std::meta::is_destructor(candidate) ||
          std::meta::is_static_member(candidate) || std::meta::is_const(candidate) ||
          std::meta::is_rvalue_reference_qualified(candidate))
        continue;
      if (std::meta::identifier_of(candidate) == std::meta::identifier_of(M) &&
          std::meta::parameters_of(candidate).size() == 1) {
        if (found != std::meta::info{}) throw std::logic_error("Ambiguous mock setter");
        found = candidate;
      }
    }
    if (found == std::meta::info{}) throw std::logic_error("No accessible method for mock setter");
    return found;
  }
}

template <std::meta::info Setter, typename Schema>
consteval bool explicitly_selected_setter() {
  for (auto member : std::meta::members_of(^^Schema, std::meta::access_context::current())) {
    if (!std::meta::is_function(member) || !std::meta::has_identifier(member)) continue;
    for (auto annotation : std::meta::annotations_of(member)) {
      if ((std::meta::type_of(annotation) == ^^decltype(serial_xml::setter)) &&
          std::meta::identifier_of(member) == std::meta::identifier_of(Setter))
        return true;
    }
  }
  return false;
}

template <std::meta::info Setter, typename T, typename V>
void invoke_setter(T& result, V& value) {
  constexpr auto parameter = std::meta::type_of(std::meta::parameters_of(Setter)[0]);
  using P = [:parameter:];
  result.[:Setter:](std::forward<P>(value));
}

template <typename T, typename Schema>
void read_object(const xml_node& node, T& result) {
  using S = std::conditional_t<std::is_void_v<Schema>, T, Schema>;
  // Reuse serialization's annotation validation and mock field resolution.
  [[maybe_unused]] constexpr auto validated = get_members<^^S, T>();
  std::size_t cdata_index = 0;
  bool raw_scalar_seen = false;
  static constexpr auto members =
      std::define_static_array(std::meta::members_of(^^S, std::meta::access_context::current()));
  template for (constexpr auto m : members) {
    if constexpr (!is_skipped_member(m) && (is_serializable_member(m) || is_setter<m>())) {
      constexpr auto source = resolve_input_member<m, T>();
      if constexpr (is_setter<m>()) {
        [[maybe_unused]] constexpr auto checked = input_type<m>();
        using V = value_t<source>;
        if (auto value = read_member<m, source, V>(node, cdata_index, raw_scalar_seen))
          invoke_setter<source>(result, *value);
      } else if constexpr (std::meta::is_function(source)) {
        constexpr auto setter = matching_setter<source, T>();
        if constexpr (setter != std::meta::info{}) {
          if constexpr (!explicitly_selected_setter<setter, S>()) {
            using V = value_t<source>;
            if (auto value = read_member<m, source, V>(node, cdata_index, raw_scalar_seen))
              invoke_setter<setter>(result, *value);
          }
        }
        // Read-only getters are serialization-only and intentionally ignored.
      } else {
        using V = value_t<source>;
        static_assert(
            requires { result.[:source:] = std::declval<V>(); },
            "Cannot modify XML data member: " + std::string(std::meta::identifier_of(source)));
        if (auto value = read_member<m, source, V>(node, cdata_index, raw_scalar_seen))
          result.[:source:] = std::move(*value);
      }
    }
  }
}
}  // namespace detail

namespace detail {
template <typename Schema, typename T>
void read_xml(T& result, std::string_view xml, const std::string& fixed_name) {
  xml_reader reader(xml);
  auto root = reader.read();
  using S = std::conditional_t<std::is_void_v<Schema>, T, Schema>;
  auto expected = fixed_name.empty() ? root_name<T, S>() : fixed_name;
  if (root.name != expected) throw deserialization_error("Expected root element: " + expected);
  read_object<T, Schema>(root, result);
}
}  // namespace detail

// The in-place form snapshots XML so assigning to a field that owns the input
// cannot invalidate the parser's borrowed text. XML is fully parsed before
// assignments; conversion/setter failures can leave a target partially updated.
export template <typename Schema = void, typename T>
  requires(std::is_class_v<T> && !std::convertible_to<T&, std::string_view>)
void from_xml(T& result, std::string_view xml, const std::string& fixed_name = "") {
  const std::string stable_input(xml);
  detail::read_xml<Schema>(result, stable_input, fixed_name);
}

export template <typename T, typename Schema = void>
  requires(std::is_class_v<T> && std::default_initializable<T>)
T from_xml(std::string_view xml, const std::string& fixed_name = "") {
  T result{};
  detail::read_xml<Schema>(result, xml, fixed_name);
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
