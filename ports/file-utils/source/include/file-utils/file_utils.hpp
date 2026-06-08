#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace file {

namespace fs = std::filesystem;

inline std::optional<std::string> read_all(std::string_view path) {
    std::ifstream file(fs::path(path), std::ios::binary | std::ios::ate);
    if (!file) return std::nullopt;
    auto size = file.tellg();
    file.seekg(0);
    std::string content(static_cast<std::size_t>(size), '\0');
    file.read(content.data(), size);
    return content;
}

inline bool write_all(std::string_view path, std::string_view content) {
    std::ofstream file(fs::path(path), std::ios::binary);
    if (!file) return false;
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return file.good();
}

inline bool append_all(std::string_view path, std::string_view content) {
    std::ofstream file(fs::path(path), std::ios::binary | std::ios::app);
    if (!file) return false;
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return file.good();
}

inline std::string dirname(std::string_view path) {
    return fs::path(path).parent_path().string();
}

inline std::string filename(std::string_view path) {
    return fs::path(path).filename().string();
}

inline std::string stem(std::string_view path) {
    return fs::path(path).stem().string();
}

inline std::string extension(std::string_view path) {
    return fs::path(path).extension().string();
}

inline std::string join(std::string_view base, std::string_view name) {
    return (fs::path(base) / fs::path(name)).string();
}

inline bool exists(std::string_view path) {
    return fs::exists(fs::path(path));
}

inline bool is_file(std::string_view path) {
    return fs::is_regular_file(fs::path(path));
}

inline bool is_directory(std::string_view path) {
    return fs::is_directory(fs::path(path));
}

inline std::uintmax_t file_size(std::string_view path) {
    std::error_code ec;
    auto size = fs::file_size(fs::path(path), ec);
    return ec ? 0 : size;
}

inline bool create_directories(std::string_view path) {
    std::error_code ec;
    return fs::create_directories(fs::path(path), ec);
}

inline bool copy_file(std::string_view from, std::string_view to, bool overwrite = false) {
    std::error_code ec;
    auto options = overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none;
    fs::copy_file(fs::path(from), fs::path(to), options, ec);
    return !ec;
}

inline bool remove(std::string_view path) {
    std::error_code ec;
    return fs::remove(fs::path(path), ec);
}

inline bool remove_all(std::string_view path) {
    std::error_code ec;
    return fs::remove_all(fs::path(path), ec) > 0;
}

inline bool rename(std::string_view from, std::string_view to) {
    std::error_code ec;
    fs::rename(fs::path(from), fs::path(to), ec);
    return !ec;
}

inline std::vector<std::string> list_files(std::string_view dir) {
    std::vector<std::string> result;
    if (!is_directory(dir)) return result;
    for (auto& entry : fs::directory_iterator(fs::path(dir))) {
        if (entry.is_regular_file()) {
            result.push_back(entry.path().string());
        }
    }
    return result;
}

inline std::vector<std::string> list_directories(std::string_view dir) {
    std::vector<std::string> result;
    if (!is_directory(dir)) return result;
    for (auto& entry : fs::directory_iterator(fs::path(dir))) {
        if (entry.is_directory()) {
            result.push_back(entry.path().string());
        }
    }
    return result;
}

inline void walk(std::string_view dir, std::function<void(const std::string&)> callback, bool recursive = true) {
    if (!is_directory(dir)) return;
    auto iter = recursive ? fs::recursive_directory_iterator(fs::path(dir))
                          : fs::directory_iterator(fs::path(dir));
    for (auto& entry : iter) {
        callback(entry.path().string());
    }
}

} // namespace file
