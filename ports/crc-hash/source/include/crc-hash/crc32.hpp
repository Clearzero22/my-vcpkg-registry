#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <string_view>

namespace hash {

namespace detail {

inline const std::array<std::uint32_t, 256>& crc32_table() {
    static const auto table = []() {
        std::array<std::uint32_t, 256> t{};
        for (std::uint32_t i = 0; i < 256; ++i) {
            std::uint32_t crc = i;
            for (int j = 0; j < 8; ++j) {
                if (crc & 1)
                    crc = (crc >> 1) ^ 0xedb88320;
                else
                    crc >>= 1;
            }
            t[i] = crc;
        }
        return t;
    }();
    return table;
}

} // namespace detail

inline std::uint32_t crc32(std::span<const std::byte> data) {
    std::uint32_t crc = 0xffffffff;
    auto& table = detail::crc32_table();
    for (auto b : data) {
        auto idx = static_cast<std::uint8_t>((crc ^ std::to_integer<std::uint32_t>(b)) & 0xff);
        crc = (crc >> 8) ^ table[idx];
    }
    return crc ^ 0xffffffff;
}

inline std::uint32_t crc32(std::string_view str) {
    return crc32(std::as_bytes(std::span(str)));
}

inline std::optional<std::uint32_t> crc32_file(std::string_view path) {
    std::ifstream file(std::filesystem::path(path), std::ios::binary);
    if (!file) return std::nullopt;
    std::uint32_t crc = 0xffffffff;
    auto& table = detail::crc32_table();
    std::array<char, 8192> buffer{};
    while (file) {
        file.read(buffer.data(), buffer.size());
        auto count = static_cast<std::size_t>(file.gcount());
        for (std::size_t i = 0; i < count; ++i) {
            auto idx = static_cast<std::uint8_t>((crc ^ static_cast<std::uint8_t>(buffer[i])) & 0xff);
            crc = (crc >> 8) ^ table[idx];
        }
    }
    return crc ^ 0xffffffff;
}

} // namespace hash
