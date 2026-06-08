#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace base64 {

inline constexpr std::string_view chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string encode(std::span<const std::byte> data) {
    std::string result;
    result.reserve(((data.size() + 2) / 3) * 4);
    std::size_t i = 0;
    while (i + 3 <= data.size()) {
        auto b0 = std::to_integer<std::uint32_t>(data[i]);
        auto b1 = std::to_integer<std::uint32_t>(data[i + 1]);
        auto b2 = std::to_integer<std::uint32_t>(data[i + 2]);
        auto n = (b0 << 16) | (b1 << 8) | b2;
        result += chars[(n >> 18) & 0x3f];
        result += chars[(n >> 12) & 0x3f];
        result += chars[(n >> 6) & 0x3f];
        result += chars[n & 0x3f];
        i += 3;
    }
    auto remain = data.size() - i;
    if (remain == 1) {
        auto b0 = std::to_integer<std::uint32_t>(data[i]);
        result += chars[(b0 >> 2) & 0x3f];
        result += chars[(b0 << 4) & 0x3f];
        result += "==";
    } else if (remain == 2) {
        auto b0 = std::to_integer<std::uint32_t>(data[i]);
        auto b1 = std::to_integer<std::uint32_t>(data[i + 1]);
        auto n = (b0 << 8) | b1;
        result += chars[(n >> 10) & 0x3f];
        result += chars[(n >> 4) & 0x3f];
        result += chars[(n << 2) & 0x3f];
        result += "=";
    }
    return result;
}

inline std::string encode(std::string_view str) {
    return encode(std::as_bytes(std::span(str)));
}

inline std::optional<std::vector<std::byte>> decode(std::string_view encoded) {
    auto pos = encoded.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=");
    if (pos != std::string_view::npos) return std::nullopt;
    if (encoded.empty()) return std::vector<std::byte>{};
    if (encoded.size() % 4 != 0) return std::nullopt;

    auto decode_char = [](char c) -> std::optional<std::uint8_t> {
        if (c >= 'A' && c <= 'Z') return static_cast<std::uint8_t>(c - 'A');
        if (c >= 'a' && c <= 'z') return static_cast<std::uint8_t>(c - 'a' + 26);
        if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(c - '0' + 52);
        if (c == '+') return 62;
        if (c == '/') return 63;
        return std::nullopt;
    };

    auto padding = (encoded[encoded.size() - 1] == '=') +
                   (encoded.size() > 1 && encoded[encoded.size() - 2] == '=');
    std::vector<std::byte> result;
    result.reserve((encoded.size() / 4) * 3 - padding);

    for (std::size_t i = 0; i < encoded.size() - padding * 2; i += 4) {
        auto c0 = decode_char(encoded[i]);
        auto c1 = decode_char(encoded[i + 1]);
        auto c2 = (i + 2 < encoded.size() - padding * 2) ? decode_char(encoded[i + 2]) : std::optional<std::uint8_t>(0);
        auto c3 = (i + 3 < encoded.size() - padding * 2) ? decode_char(encoded[i + 3]) : std::optional<std::uint8_t>(0);
        if (!c0 || !c1) return std::nullopt;
        auto n = (static_cast<std::uint32_t>(*c0) << 18) |
                 (static_cast<std::uint32_t>(*c1) << 12) |
                 (static_cast<std::uint32_t>(c2.value_or(0)) << 6) |
                 static_cast<std::uint32_t>(c3.value_or(0));
        result.push_back(std::byte((n >> 16) & 0xff));
        if (c2) result.push_back(std::byte((n >> 8) & 0xff));
        if (c3) result.push_back(std::byte(n & 0xff));
    }
    return result;
}

inline std::optional<std::string> decode_to_string(std::string_view encoded) {
    auto bytes = decode(encoded);
    if (!bytes) return std::nullopt;
    return std::string(reinterpret_cast<const char*>(bytes->data()), bytes->size());
}

} // namespace base64
