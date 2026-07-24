#pragma once
// clang-format off
// Reflection-based auto-discovery of ams2_telemetry fields via C++26 <meta>.
// Requires GCC with -freflection. Only include from C++ translation units.

#include "ams2_telemetry.h"

#include <array>
#include <cstdint>
#include <meta>
#include <string>
#include <string_view>
// clang-format on

namespace meta = std::meta;

static constexpr size_t MAX_ARRAY_EXTENT = 4;
static constexpr size_t MAX_COLUMNS = 256;

struct column_def {
  const char *name;
  carquet_physical_type_t type;
  size_t elem_size;
  size_t tele_offset;
};

consteval char to_upper_char(char c) { return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c; }

consteval std::string capitalize_first(std::string_view s) {
  if (s.empty()) {
    return {};
  }
  std::string result(s);
  result[0] = to_upper_char(result[0]);
  return result;
}

consteval std::string array_suffix(size_t index, size_t extent) {
  if (extent == 3) {
    switch (index) {
    case 0:
      return "X";
    case 1:
      return "Y";
    case 2:
      return "Z";
    }
  }
  if (extent == 4) {
    switch (index) {
    case 0:
      return "Fl";
    case 1:
      return "Fr";
    case 2:
      return "Rl";
    case 3:
      return "Rr";
    }
  }
  return std::string(1, static_cast<char>('0' + index));
}

struct column_array {
  std::array<column_def, MAX_COLUMNS> cols{};
  size_t count = 0;
};

consteval column_array build_columns() {
  column_array result;
  column_def *p = result.cols.data();

  constexpr auto ac = meta::access_context::unchecked();
  auto members = meta::nonstatic_data_members_of(^^ams2_telemetry, ac);

  for (auto m : members) {
    auto t = meta::type_of(m);

    if (meta::is_volatile_type(t)) {
      continue;
    }

    auto tc = meta::remove_cv(t);

    if (meta::is_floating_point_type(tc)) {
      auto name = capitalize_first(meta::identifier_of(m));
      p[result.count++] = {std::define_static_string(name), CARQUET_PHYSICAL_FLOAT, sizeof(float),
                           static_cast<size_t>(meta::offset_of(m).bytes)};
    } else if (meta::is_array_type(tc)) {
      auto elem = meta::remove_cv(meta::remove_extent(tc));
      if (meta::is_floating_point_type(elem)) {
        size_t n = meta::extent(tc, 0);
        if (n <= MAX_ARRAY_EXTENT) {
          std::string base = capitalize_first(meta::identifier_of(m));
          size_t base_off = static_cast<size_t>(meta::offset_of(m).bytes);
          for (size_t i = 0; i < n; i++) {
            auto col_name = base + array_suffix(i, n);
            p[result.count++] = {std::define_static_string(col_name), CARQUET_PHYSICAL_FLOAT, sizeof(float),
                                 base_off + i * sizeof(float)};
          }
        }
      }
    } else if (meta::is_enum_type(tc)) {
      auto underlying = meta::underlying_type(tc);
      if (meta::is_integral_type(underlying) && meta::is_signed_type(underlying)) {
        auto name = capitalize_first(meta::identifier_of(m));
        p[result.count++] = {std::define_static_string(name), CARQUET_PHYSICAL_INT32, sizeof(int32_t),
                             static_cast<size_t>(meta::offset_of(m).bytes)};
      }
    } else if (meta::is_integral_type(tc) && meta::is_signed_type(tc)) {
      auto name = capitalize_first(meta::identifier_of(m));
      p[result.count++] = {std::define_static_string(name), CARQUET_PHYSICAL_INT32, sizeof(int32_t),
                           static_cast<size_t>(meta::offset_of(m).bytes)};
    }
  }

  return result;
}

constexpr auto all_columns = build_columns();
