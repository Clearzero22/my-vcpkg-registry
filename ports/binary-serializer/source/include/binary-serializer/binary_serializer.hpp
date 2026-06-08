#pragma once

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace bin {

class writer {
public:
    void write_raw(const void* data, std::size_t size) {
        auto ptr = static_cast<const std::byte*>(data);
        buffer_.insert(buffer_.end(), ptr, ptr + size);
    }

    template<typename T>
    void write(T value) {
        if constexpr (std::is_arithmetic_v<T>) {
            if constexpr (std::endian::native == std::endian::big) {
                value = std::byteswap(value);
            }
            auto* ptr = reinterpret_cast<const std::byte*>(&value);
            buffer_.insert(buffer_.end(), ptr, ptr + sizeof(T));
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>) {
            auto size = static_cast<std::uint64_t>(value.size());
            write(size);
            auto* ptr = reinterpret_cast<const std::byte*>(value.data());
            buffer_.insert(buffer_.end(), ptr, ptr + static_cast<std::size_t>(size));
        } else {
            static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>,
                          "Unsupported type for binary writer");
        }
    }

    template<typename T>
    void write(const std::vector<T>& vec) {
        write(static_cast<std::uint64_t>(vec.size()));
        for (const auto& v : vec) {
            write(v);
        }
    }

    template<typename K, typename V>
    void write(const std::map<K, V>& map) {
        write(static_cast<std::uint64_t>(map.size()));
        for (const auto& [k, v] : map) {
            write(k);
            write(v);
        }
    }

    template<typename... Args>
    void write(const std::tuple<Args...>& t) {
        std::apply([this](const auto&... args) {
            (this->write(args), ...);
        }, t);
    }

    std::span<const std::byte> data() const {
        return buffer_;
    }

    const std::vector<std::byte>& bytes() const {
        return buffer_;
    }

    void clear() {
        buffer_.clear();
    }

private:
    std::vector<std::byte> buffer_;
};

class reader {
public:
    explicit reader(std::span<const std::byte> data) : data_(data), pos_(0) {}

    bool read_raw(void* data, std::size_t size) {
        if (pos_ + size > data_.size()) return false;
        std::memcpy(data, data_.data() + pos_, size);
        pos_ += size;
        return true;
    }

    template<typename T>
    std::optional<T> read() {
        T value;
        if constexpr (std::is_arithmetic_v<T>) {
            if (!read_raw(&value, sizeof(T))) return std::nullopt;
            if constexpr (std::endian::native == std::endian::big) {
                value = std::byteswap(value);
            }
            return value;
        } else if constexpr (std::is_same_v<T, std::string>) {
            auto size_opt = read<std::uint64_t>();
            if (!size_opt) return std::nullopt;
            auto size = static_cast<std::size_t>(*size_opt);
            if (pos_ + size > data_.size()) return std::nullopt;
            std::string result(reinterpret_cast<const char*>(data_.data() + pos_), size);
            pos_ += size;
            return result;
        } else {
            static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>,
                          "Unsupported type for binary reader");
            return std::nullopt;
        }
    }

    template<typename T>
    std::optional<std::vector<T>> read_vector() {
        auto size_opt = read<std::uint64_t>();
        if (!size_opt) return std::nullopt;
        auto count = static_cast<std::size_t>(*size_opt);
        std::vector<T> result;
        result.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            auto elem = read<T>();
            if (!elem) return std::nullopt;
            result.push_back(std::move(*elem));
        }
        return result;
    }

    bool eof() const {
        return pos_ >= data_.size();
    }

    std::size_t remain() const {
        return data_.size() - pos_;
    }

    std::size_t position() const {
        return pos_;
    }

private:
    std::span<const std::byte> data_;
    std::size_t pos_;
};

} // namespace bin
