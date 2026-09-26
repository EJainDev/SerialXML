module;

#include <cstdint>

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

// Decode UTF-8 strictly so malformed input cannot produce malformed XML.
constexpr bool next_code_point(std::string_view text, std::size_t& offset, char32_t& value) {
  if (offset == text.size()) return false;
  const auto first = static_cast<unsigned char>(text[offset++]);
  if (first < 0x80) {
    value = first;
    return true;
  }
  int extra = 0;
  char32_t code = 0;
  char32_t minimum = 0;
  if (first >= 0xC2 && first <= 0xDF) {
    extra = 1;
    code = first & 0x1F;
    minimum = 0x80;
  } else if (first >= 0xE0 && first <= 0xEF) {
    extra = 2;
    code = first & 0x0F;
    minimum = 0x800;
  } else if (first >= 0xF0 && first <= 0xF4) {
    extra = 3;
    code = first & 0x07;
    minimum = 0x10000;
  } else {
    return false;
  }
  if (text.size() - offset < static_cast<std::size_t>(extra)) return false;
  for (int i = 0; i < extra; ++i) {
    const auto next = static_cast<unsigned char>(text[offset++]);
    if ((next & 0xC0) != 0x80) return false;
    code = (code << 6) | (next & 0x3F);
  }
  if (code < minimum || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF)) return false;
  value = code;
  return true;
}

constexpr bool is_xml_character(char32_t c) {
  return c == 0x9 || c == 0xA || c == 0xD || (c >= 0x20 && c <= 0xD7FF) ||
         (c >= 0xE000 && c <= 0xFFFD) || (c >= 0x10000 && c <= 0x10FFFF);
}

constexpr bool is_name_start(char32_t c) {
  return c == ':' || c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
         (c >= 0xC0 && c <= 0xD6) || (c >= 0xD8 && c <= 0xF6) || (c >= 0xF8 && c <= 0x2FF) ||
         (c >= 0x370 && c <= 0x37D) || (c >= 0x37F && c <= 0x1FFF) ||
         (c >= 0x200C && c <= 0x200D) || (c >= 0x2070 && c <= 0x218F) ||
         (c >= 0x2C00 && c <= 0x2FEF) || (c >= 0x3001 && c <= 0xD7FF) ||
         (c >= 0xF900 && c <= 0xFDCF) || (c >= 0xFDF0 && c <= 0xFFFD) ||
         (c >= 0x10000 && c <= 0xEFFFF);
}

constexpr bool is_name_character(char32_t c) {
  return is_name_start(c) || c == '-' || c == '.' || (c >= '0' && c <= '9') || c == 0xB7 ||
         (c >= 0x300 && c <= 0x36F) || (c >= 0x203F && c <= 0x2040);
}

constexpr bool is_valid_xml_name(std::string_view text) {
  if (text.empty()) return false;
  std::size_t offset = 0;
  char32_t code = 0;
  if (!next_code_point(text, offset, code) || !is_name_start(code)) return false;
  while (offset < text.size()) {
    if (!next_code_point(text, offset, code) || !is_name_character(code)) return false;
  }
  // XML reserves names beginning with any case spelling of "xml".
  if (text.size() >= 3) {
    auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; };
    if (lower(text[0]) == 'x' && lower(text[1]) == 'm' && lower(text[2]) == 'l') return false;
  }
  return true;
}

void validate_xml_characters(std::string_view text) {
  std::size_t offset = 0;
  char32_t code = 0;
  while (offset < text.size()) {
    if (!next_code_point(text, offset, code) || !is_xml_character(code)) {
      throw std::invalid_argument("Invalid XML character or UTF-8 sequence");
    }
  }
}

void append_escaped(std::string& result, std::string_view text, bool attribute_value = false) {
  validate_xml_characters(text);
  for (char ch : text) {
    switch (ch) {
      case '<':
        result += "&lt;";
        break;
      case '>':
        result += "&gt;";
        break;
      case '&':
        result += "&amp;";
        break;
      case '\'':
        result += "&apos;";
        break;
      case '"':
        result += "&quot;";
        break;
      case '\t':
        if (attribute_value) {
          result += "&#x9;";
          break;
        }
        result += ch;
        break;
      case '\n':
        if (attribute_value) {
          result += "&#xA;";
          break;
        }
        result += ch;
        break;
      case '\r':
        result += "&#xD;";
        break;
      default:
        result += ch;
    }
  }
}

void append_cdata(std::string& result, std::string_view text) {
  validate_xml_characters(text);
  result += "<![CDATA[";
  std::size_t start = 0;
  while (true) {
    const auto terminator = text.find("]]>", start);
    const auto carriage_return = text.find('\r', start);
    if (terminator == std::string_view::npos && carriage_return == std::string_view::npos) break;
    if (carriage_return < terminator) {
      result.append(text.substr(start, carriage_return - start));
      result += "]]>&#xD;<![CDATA[";
      start = carriage_return + 1;
    } else {
      result.append(text.substr(start, terminator + 2 - start));
      result += "]]><![CDATA[>";
      start = terminator + 3;
    }
  }
  result.append(text.substr(start));
  result += "]]>";
}

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

template <std::meta::info m>
struct value_type {
  using t = std::remove_cvref_t<typename[:std::meta::type_of(m):]>;
  static constexpr auto m_t = std::meta::dealias(std::meta::type_of(m));
};

template <std::meta::info m>
  requires(std::meta::is_function(m))
struct value_type<m> {
  using t = std::remove_cvref_t<typename[:std::meta::return_type_of(m):]>;
  static constexpr auto m_t = std::meta::dealias(std::meta::return_type_of(m));
};

template <std::meta::info m>
using value_t = typename value_type<m>::t;

template <std::meta::info m>
constexpr auto value_m_t = value_type<m>::m_t;

template <std::meta::info m>
consteval bool is_stl_handled() {
  if constexpr (get_namespace<m>() == ^^std) {
    static constexpr auto m_t = std::meta::template_of(std::meta::dealias(m));

    if constexpr (m_t == ^^std::vector || m_t == ^^std::array || m_t == ^^std::inplace_vector ||
                  m_t == ^^std::deque || m_t == ^^std::forward_list || m_t == ^^std::span ||
                  m_t == ^^std::valarray || m_t == ^^std::optional) {
      return true;
    }
  }

  return false;
}

template <std::meta::info m>
consteval auto get_annotations()
    -> structural_tuple::tuple<bool, bool, bool, bool, bool, bool,
                               std::pair<char const*, char const*>, char const*, bool> {
  static constexpr auto annotations = std::define_static_array(std::meta::annotations_of(m));

  bool is_attribute = false;
  bool is_cdata = false;
  bool is_no_iter = !is_stl_handled<value_m_t<m>>();
  bool is_raw = false;
  bool is_skip = false;
  bool is_unpack = std::meta::is_class_type(value_m_t<m>) && !std::formattable<value_t<m>, char> &&
                   !is_stl_handled<value_m_t<m>>();

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
        if constexpr (!std::ranges::range<value_t<m>>) {
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

  if constexpr (std::ranges::range<value_t<m>>) {
    if (iter_names.has_value()) {
      using m_t = std::ranges::range_value_t<value_t<m>>;
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

template <std::meta::info m>
consteval auto is_invalid_function() {
  if constexpr (std::meta::is_function(m)) {
    if constexpr (!(std::meta::is_constructor(m) || std::meta::is_destructor(m))) {
      if constexpr (std::meta::return_type_of(m) == ^^void) {
        return true;
      }
    } else {
      return true;
    }

    if constexpr (!std::meta::is_const(m)) {
      return true;
    }

    static constexpr auto params = std::define_static_array(std::meta::parameters_of(m));
    template for (constexpr auto p : params) {
      if constexpr (!std::meta::has_default_argument(p)) {
        return true;
      }
    }
  } else if constexpr (std::meta::is_function_template(m)) {
    return true;
  }

  return false;
}

template <std::meta::info container>
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
    static constexpr auto invalid_function = is_invalid_function<m>();

    if constexpr (!invalid_function) {
      static constexpr auto m_annotations = get_annotations<m>();

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

        auto [ptr, _] = std::to_chars(buf, buf + 20, value);

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

    result.append(prefix, prefix_size);
    append_escaped(result, buffer, true);
    result += '"';
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

                                    return original_size + combined_size + (ptr - buf);
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
    if constexpr (formatter.is_empty() &&
                  (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)) {
      append_cdata(result, value);
    } else {
      append_cdata(result, format_value<formatter>(value));
    }
  } else {
    if constexpr (formatter.is_empty() && std::is_same_v<T, std::string>) {
      buffer = std::ref(value);
    } else {
      buffer = format_value<formatter>(value);
    }

    result.append(opening_tag, opening_tag_size);
    append_escaped(result, buffer);
    result.append(closing_tag, closing_tag_size);
  }
}

template <typename T>
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
      if (value.size() > 0) {
        if constexpr (!is_raw) {
          result += start;
        }

        static constexpr auto single_name = std::define_static_string("element");
        static constexpr auto item_m_t = std::meta::dealias(^^decltype(value[0]));

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

template <typename T>
  requires(std::is_class_v<T>)
void to_xml(const T& value, std::string& result, std::string& buffer, bool first,
            const std::string& fixed_name) {
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
  if (!is_valid_xml_name(name)) {
    throw std::invalid_argument("Invalid XML name: '" + name + "'");
  }
  std::format_to(std::back_inserter(result), "<{}", name);

  static constexpr auto members = get_members<M>();
  static constexpr auto attribute_annotations = members.first;
  static constexpr auto child_annotations = members.second;

  template for (constexpr auto m_a : attribute_annotations) {
    static constexpr auto is_std = is_stl_handled<std::meta::type_of(m_a.first)>();

    static constexpr auto m = m_a.first;
    static constexpr auto m_annotations = m_a.second;

    static constexpr auto formatter = get_format<m>();
    static constexpr auto m_name = structural_tuple::get<0>(m_annotations);

    static constexpr auto view_name = std::string_view(m_name);

    static_assert(is_valid_xml_name(view_name),
                  std::string("Invalid XML name: '") + std::string(view_name) + "'");

    if constexpr (is_std) {
      handle_stl<true, false, true, false, false, false, m_name, formatter>(result, buffer,
                                                                            get_value<m>(value));
    } else {
      add_attribute<m_name, formatter>(result, buffer, get_value<m>(value));
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

      static constexpr auto formatter = get_format<m>();
      static constexpr auto iter_names = structural_tuple::get<4>(m_annotations);

      static constexpr auto m_name = structural_tuple::get<5>(m_annotations);

      static constexpr auto is_exclude_on_empty = structural_tuple::get<6>(m_annotations);

      static constexpr auto view_name = std::string_view(m_name);

      static_assert(is_valid_xml_name(view_name),
                    std::string("Invalid XML name: '") + std::string(view_name) + "'");

      if constexpr (iter_names.first != nullptr) {
        static_assert(is_valid_xml_name(iter_names.first), "Invalid XML iterator item name");
        static_assert(is_valid_xml_name(iter_names.second), "Invalid XML iterator container name");
      }

      if constexpr (iter_names.first == nullptr && is_std && !is_no_iter) {
        handle_stl<false, is_cdata, is_no_iter, is_raw, is_exclude_on_empty, is_unpack, m_name,
                   formatter>(result, buffer, get_value<m>(value));
      } else {
        if constexpr (iter_names.first != nullptr && std::meta::is_class_type(value_m_t<m>) &&
                      std::ranges::range<value_t<m>>) {
          if constexpr (!is_raw) {
            result.push_back('<');
            result.append(iter_names.second);
            result.push_back('>');
          }

          for (const auto& item : get_value<m>(value)) {
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
          to_xml(get_value<m>(value), result, buffer, false, m_name);
        } else {
          if constexpr (is_raw) {
            buffer = format_value<formatter>(get_value<m>(value));

            append_escaped(result, buffer);
          } else {
            add_child<m_name, is_cdata, formatter>(result, buffer, get_value<m>(value));
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

export template <typename T>
  requires(std::is_class_v<T>)
auto to_xml(const T& value, bool first = true, const std::string& fixed_name = "") -> std::string {
  std::string result;
  std::string buffer;

  result.reserve(4096);
  buffer.reserve(256);

  to_xml(value, result, buffer, first, fixed_name);

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
