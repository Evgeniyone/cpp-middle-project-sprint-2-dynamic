#pragma once

#include "parse.hpp"
#include "types.hpp"
#include <expected>
#include <iostream>
#include <ostream>

namespace stdx {

template <typename... Ts, size_t... I>
std::expected<details::scan_result<Ts...>, details::scan_error>
scan_impl(std::index_sequence<I...>,
          const std::vector<std::string_view>& fmt_parts,
          const std::vector<std::string_view>& inp_parts) {
    std::tuple<Ts...> values;
    details::scan_error err{""};

    bool all_ok = ( ... && ([&]() -> bool {
        auto r = details::parse_value_with_format<Ts>(inp_parts[I], fmt_parts[I]);
        if (!r) {
            err = r.error();
            return false;
        }
        std::get<I>(values) = *r;
        return true;
    }() ) );

    if (!all_ok) {
        return std::unexpected(err);
    }

    return details::scan_result<Ts...>{ std::move(std::get<I>(values))... };
}

template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input, std::string_view format) {
    auto res = details::parse_sources<Ts...>(input, format);
    if (!res.has_value())
        return std::unexpected(res.error());

    auto [fmt_parts, inp_parts] = *res;
    constexpr size_t N = sizeof...(Ts);
    if (fmt_parts.size() != N || inp_parts.size() != N)
        return std::unexpected(details::scan_error{"Placeholder/Type count mismatch"});

    return scan_impl<Ts...>(std::make_index_sequence<N>{}, fmt_parts, inp_parts);

}

} // namespace stdx
