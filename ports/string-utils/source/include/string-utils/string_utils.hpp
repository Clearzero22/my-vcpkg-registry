#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <functional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace str {

inline std::string trim_left(std::string_view s) {
    auto it = std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return std::string(it, s.end());
}

inline std::string trim_right(std::string_view s) {
    auto it = std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return std::string(s.begin(), it.base());
}

inline std::string trim(std::string_view s) {
    return trim_left(trim_right(s));
}

inline std::vector<std::string> split(std::string_view s, std::string_view delimiter) {
    std::vector<std::string> result;
    if (delimiter.empty()) {
        for (auto ch : s) {
            result.emplace_back(1, ch);
        }
        return result;
    }
    std::size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string_view::npos) {
        result.emplace_back(s.substr(start, end - start));
        start = end + delimiter.size();
    }
    result.emplace_back(s.substr(start));
    return result;
}

inline std::string join(std::span<const std::string> parts, std::string_view delimiter) {
    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) result += delimiter;
        result += parts[i];
    }
    return result;
}

inline std::string to_lower(std::string_view s) {
    std::string result(s);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return result;
}

inline std::string to_upper(std::string_view s) {
    std::string result(s);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return result;
}

inline std::string to_hex(std::span<const std::byte> data) {
    static constexpr char hex[] = "0123456789abcdef";
    std::string result(data.size() * 2, '\0');
    for (std::size_t i = 0; i < data.size(); ++i) {
        result[i * 2] = hex[(std::to_integer<int>(data[i]) >> 4) & 0x0f];
        result[i * 2 + 1] = hex[std::to_integer<int>(data[i]) & 0x0f];
    }
    return result;
}

inline std::vector<std::byte> from_hex(std::string_view hex) {
    std::vector<std::byte> result;
    result.reserve(hex.size() / 2);
    auto hex_val = [](char c) -> unsigned char {
        if (c >= '0' && c <= '9') return static_cast<unsigned char>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<unsigned char>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<unsigned char>(c - 'A' + 10);
        return 0;
    };
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        auto val = static_cast<unsigned char>((hex_val(hex[i]) << 4) | hex_val(hex[i + 1]));
        result.push_back(std::byte(val));
    }
    return result;
}

inline std::string replace(std::string_view s, std::string_view from, std::string_view to) {
    std::string result;
    std::size_t start = 0, pos;
    while ((pos = s.find(from, start)) != std::string_view::npos) {
        result += s.substr(start, pos - start);
        result += to;
        start = pos + from.size();
    }
    result += s.substr(start);
    return result;
}

inline bool replace_in_place(std::string& s, std::string_view from, std::string_view to) {
    auto pos = s.find(from);
    if (pos == std::string::npos) return false;
    s.replace(pos, from.size(), to);
    return true;
}

inline bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

inline bool ends_with(std::string_view s, std::string_view suffix) {
    return s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix;
}

inline bool contains(std::string_view s, std::string_view sub) {
    return s.find(sub) != std::string_view::npos;
}

} // namespace str
