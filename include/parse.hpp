#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {

// здесь ваш код
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename U>
concept string_type = 
    std::same_as<U, std::string_view> ||
    std::same_as<U, std::string>;

template<arithmetic T>
std::expected<T, scan_error> parse_value(std::string_view input){
    T out;
    auto [ptr, ec] = std::from_chars(input.begin(), input.end(), out);
            
    if (ec == std::errc::invalid_argument)
        return std::unexpected(scan_error{"This is not a number."});
    if (ec == std::errc::result_out_of_range)
        return std::unexpected(scan_error{"This number is larger than requsted type."});
    
    return out;
}

template<string_type T>
std::expected<T, scan_error> parse_value(std::string_view input){
    return T{input};
}

//{%d} {%f} {%u} {%s}
// Функция для парсинга значения с учетом спецификатора формата
template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    if (fmt.empty())
        return parse_value<T>(input);

    if (fmt.size() != 2 || fmt[0] != '%') {
        return std::unexpected(
            scan_error{"Invalid format specifier \"" +
                       std::string(fmt) +
                       "\": expected \"%<char>\""});
    }

    char spec = fmt[1];
    switch (spec) {
        case 'd':
            if constexpr (!(std::is_integral_v<T> && std::is_signed_v<T>)) {
                return std::unexpected(
                    scan_error{"Conversion specifier %d requires a signed integer type"});
            }
            break;

        case 'u':
            if constexpr (!(std::is_integral_v<T> && std::is_unsigned_v<T>)) {
                return std::unexpected(
                    scan_error{"Conversion specifier %u requires an unsigned integer type"});
            }
            break;

        case 'f':
            if constexpr (!std::is_floating_point_v<T>) {
                return std::unexpected(
                    scan_error{"Conversion specifier %f requires a floating-point type"});
            }
            break;

        case 's':
            if constexpr (!string_type<T>) {
                return std::unexpected(
                    scan_error{"Conversion specifier %s requires a string-like type"});
            }
            break;

        default:
            return std::unexpected(
                scan_error{"Unknown conversion specifier '%" +
                           std::string(1, spec) + "'"});
    }

    return parse_value<T>(input);
    
}

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
template <typename... Ts>
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{format_parts, input_parts};
}

} // namespace stdx::details