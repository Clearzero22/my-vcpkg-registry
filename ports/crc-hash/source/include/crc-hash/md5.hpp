#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

namespace hash {

struct md5_digest {
    std::uint8_t bytes[16]{};

    std::string to_string() const {
        std::ostringstream oss;
        for (auto b : bytes) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        }
        return oss.str();
    }

    bool operator==(const md5_digest& other) const {
        return std::memcmp(bytes, other.bytes, 16) == 0;
    }

    bool operator!=(const md5_digest& other) const {
        return !(*this == other);
    }
};

namespace detail {

inline md5_digest md5_transform(std::span<const std::byte> data) {
    auto left_rotate = [](std::uint32_t x, std::uint32_t c) {
        return (x << c) | (x >> (32 - c));
    };

    std::uint32_t a0 = 0x67452301, b0 = 0xefcdab89;
    std::uint32_t c0 = 0x98badcfe, d0 = 0x10325476;

    std::uint64_t bit_len = static_cast<std::uint64_t>(data.size()) * 8;

    std::vector<std::byte> msg(data.begin(), data.end());
    msg.push_back(std::byte(0x80));
    while ((msg.size() % 64) != 56) {
        msg.push_back(std::byte(0));
    }
    for (int i = 0; i < 8; ++i) {
        msg.push_back(std::byte((bit_len >> (i * 8)) & 0xff));
    }

    static const std::array<std::uint32_t, 64> k = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    static const std::array<int, 64> r = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    for (std::size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        std::array<std::uint32_t, 16> m{};
        for (int i = 0; i < 16; ++i) {
            m[i] = static_cast<std::uint32_t>(
                (std::to_integer<std::uint32_t>(msg[chunk + i * 4])) |
                (std::to_integer<std::uint32_t>(msg[chunk + i * 4 + 1]) << 8) |
                (std::to_integer<std::uint32_t>(msg[chunk + i * 4 + 2]) << 16) |
                (std::to_integer<std::uint32_t>(msg[chunk + i * 4 + 3]) << 24)
            );
        }

        std::uint32_t a = a0, b = b0, c = c0, d = d0;
        for (int i = 0; i < 64; ++i) {
            std::uint32_t f, g;
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = i;
            } else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3 * i + 5) % 16;
            } else {
                f = c ^ (b | (~d));
                g = (7 * i) % 16;
            }
            f = f + a + k[i] + m[g];
            a = d;
            d = c;
            c = b;
            b = b + left_rotate(f, r[i]);
        }
        a0 += a; b0 += b; c0 += c; d0 += d;
    }

    md5_digest result;
    for (int i = 0; i < 4; ++i) {
        result.bytes[i] = static_cast<std::uint8_t>((a0 >> (i * 8)) & 0xff);
        result.bytes[4 + i] = static_cast<std::uint8_t>((b0 >> (i * 8)) & 0xff);
        result.bytes[8 + i] = static_cast<std::uint8_t>((c0 >> (i * 8)) & 0xff);
        result.bytes[12 + i] = static_cast<std::uint8_t>((d0 >> (i * 8)) & 0xff);
    }
    return result;
}

} // namespace detail

inline md5_digest md5(std::span<const std::byte> data) {
    return detail::md5_transform(data);
}

inline md5_digest md5(std::string_view str) {
    return detail::md5_transform(std::as_bytes(std::span(str)));
}

inline std::optional<md5_digest> md5_file(std::string_view path) {
    std::ifstream file(std::filesystem::path(path), std::ios::binary);
    if (!file) return std::nullopt;
    file.seekg(0, std::ios::end);
    auto size = static_cast<std::size_t>(file.tellg());
    file.seekg(0);
    std::vector<std::byte> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size));
    if (!file) return std::nullopt;
    return detail::md5_transform(buffer);
}

} // namespace hash
