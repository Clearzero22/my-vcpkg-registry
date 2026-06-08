#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <string_view>

namespace uuid {

struct uuid {
    std::array<std::uint8_t, 16> bytes{};

    uuid() = default;

    static uuid v4() {
        static thread_local std::mt19937_64 rng(std::random_device{}());
        std::array<std::uint64_t, 2> rnd{};
        rnd[0] = rng();
        rnd[1] = rng();
        uuid result;
        std::memcpy(result.bytes.data(), rnd.data(), 16);
        result.bytes[6] = (result.bytes[6] & 0x0f) | 0x40;
        result.bytes[8] = (result.bytes[8] & 0x3f) | 0x80;
        return result;
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (int i = 0; i < 16; ++i) {
            if (i == 4 || i == 6 || i == 8 || i == 10) oss << '-';
            oss << std::setw(2) << static_cast<int>(bytes[i]);
        }
        return oss.str();
    }

    std::string to_string_compact() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (auto b : bytes) {
            oss << std::setw(2) << static_cast<int>(b);
        }
        return oss.str();
    }

    static std::optional<uuid> parse(std::string_view str) {
        std::string cleaned;
        for (auto c : str) {
            if (c != '-' && c != '{' && c != '}') {
                cleaned += c;
            }
        }
        if (cleaned.size() != 32) return std::nullopt;
        uuid result;
        for (int i = 0; i < 16; ++i) {
            auto hex_byte = cleaned.substr(i * 2, 2);
            result.bytes[i] = static_cast<std::uint8_t>(std::stoul(hex_byte, nullptr, 16));
        }
        return result;
    }

    bool operator==(const uuid& other) const = default;
    bool operator!=(const uuid& other) const = default;
    bool operator<(const uuid& other) const { return bytes < other.bytes; }
};

inline std::ostream& operator<<(std::ostream& os, const uuid& u) {
    os << u.to_string();
    return os;
}

} // namespace uuid

namespace std {
template<> struct hash<uuid::uuid> {
    std::size_t operator()(const uuid::uuid& u) const noexcept {
        std::size_t h = 0;
        for (auto b : u.bytes) {
            h = (h * 31) + static_cast<std::size_t>(b);
        }
        return h;
    }
};
} // namespace std
