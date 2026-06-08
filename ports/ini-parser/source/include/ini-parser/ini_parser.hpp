#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace ini {

class parser {
public:
    bool load(std::string_view path) {
        auto filepath = std::filesystem::path(path);
        std::ifstream file(filepath);
        if (!file) return false;
        std::stringstream ss;
        ss << file.rdbuf();
        return load_from_string(ss.str());
    }

    bool load_from_string(std::string_view content) {
        data_.clear();
        std::string current_section = "";
        std::string content_str(content);
        std::istringstream stream(content_str);
        std::string line;
        int line_num = 0;
        while (std::getline(stream, line)) {
            ++line_num;
            auto trimmed = trim(line);
            if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') continue;

            if (trimmed[0] == '[') {
                auto end = trimmed.find(']');
                if (end == std::string::npos) return false;
                current_section = trim(trimmed.substr(1, end - 1));
                if (data_.find(current_section) == data_.end()) {
                    data_[current_section] = {};
                }
                continue;
            }

            auto eq_pos = trimmed.find('=');
            if (eq_pos == std::string::npos) return false;
            auto key = trim(trimmed.substr(0, eq_pos));
            auto value = trim(trimmed.substr(eq_pos + 1));

            if (!value.empty()) {
                if ((value.front() == '"' && value.back() == '"') ||
                    (value.front() == '\'' && value.back() == '\'')) {
                    if (value.size() >= 2) {
                        value = value.substr(1, value.size() - 2);
                    }
                }
            }
            data_[current_section][key] = value;
        }
        loaded_ = true;
        return true;
    }

    bool save(std::string_view path) {
        auto filepath = std::string(path);
        std::ofstream file(filepath);
        if (!file) return false;
        for (auto& [section, kv] : data_) {
            if (section.empty()) {
                for (auto& [k, v] : kv) {
                    file << k << " = " << v << "\n";
                }
            } else {
                file << "[" << section << "]\n";
                for (auto& [k, v] : kv) {
                    file << k << " = " << v << "\n";
                }
            }
        }
        return true;
    }

    std::optional<std::string> get(std::string_view section, std::string_view key) const {
        auto sit = data_.find(std::string(section));
        if (sit == data_.end()) return std::nullopt;
        auto kit = sit->second.find(std::string(key));
        if (kit == sit->second.end()) return std::nullopt;
        return kit->second;
    }

    template<typename T>
    std::optional<T> get_as(std::string_view section, std::string_view key) const {
        auto val = get(section, key);
        if (!val) return std::nullopt;
        std::istringstream iss(*val);
        T result;
        iss >> result;
        if (iss.fail()) return std::nullopt;
        return result;
    }

    void set(std::string_view section, std::string_view key, std::string_view value) {
        data_[std::string(section)][std::string(key)] = std::string(value);
    }

    bool has_section(std::string_view section) const {
        return data_.contains(std::string(section));
    }

    bool has_key(std::string_view section, std::string_view key) const {
        auto it = data_.find(std::string(section));
        if (it == data_.end()) return false;
        return it->second.contains(std::string(key));
    }

    std::vector<std::string> sections() const {
        std::vector<std::string> result;
        for (auto& [sec, _] : data_) {
            result.push_back(sec);
        }
        return result;
    }

    std::vector<std::string> keys(std::string_view section) const {
        auto it = data_.find(std::string(section));
        if (it == data_.end()) return {};
        std::vector<std::string> result;
        for (auto& [k, _] : it->second) {
            result.push_back(k);
        }
        return result;
    }

private:
    static std::string trim(std::string_view s) {
        auto start = std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        auto end = std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        return std::string(start, end.base());
    }

    bool loaded_ = false;
    std::map<std::string, std::map<std::string, std::string>> data_;
};

} // namespace ini
