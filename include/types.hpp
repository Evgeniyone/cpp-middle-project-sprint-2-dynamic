#pragma once
#include <tuple>
#include <string>

namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования
template <typename... Ts>
struct scan_result {
    std::tuple<Ts...> data;

    scan_result() = default;
    explicit scan_result(Ts... vs)
        : data(std::make_tuple(std::move(vs)...)) {}

    const std::tuple<Ts...>& values() const & noexcept { return data; }
    std::tuple<Ts...>&       values() & noexcept       { return data; }
    std::tuple<Ts...>        values() && noexcept      { return std::move(data); }
};

} // namespace stdx::details
