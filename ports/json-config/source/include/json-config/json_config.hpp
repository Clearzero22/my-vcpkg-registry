#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace json {

enum class value_type { null, boolean, integer, real, string, array, object };

class value {
public:
    value() : type_(value_type::null) { integer_ = 0; }
    value(nullptr_t) : type_(value_type::null) { integer_ = 0; }
    value(bool b) : type_(value_type::boolean) { boolean_ = b; }
    value(std::int64_t i) : type_(value_type::integer) { integer_ = i; }
    value(double d) : type_(value_type::real) { real_ = d; }
    value(const char* s) : type_(value_type::string) { string_ = new std::string(s); }
    value(std::string s) : type_(value_type::string) { string_ = new std::string(std::move(s)); }
    value(std::string_view s) : type_(value_type::string) { string_ = new std::string(s); }
    value(std::initializer_list<value> list) : type_(value_type::array) { array_ = new std::vector<value>(list); }

    value(const value& other)
        : type_(other.type_), integer_(other.integer_) {
        copy_from(other);
    }
    value(value&& other) noexcept
        : type_(other.type_), integer_(other.integer_) {
        move_from(std::move(other));
    }
    value& operator=(const value& other) {
        if (this != &other) {
            destroy();
            type_ = other.type_;
            integer_ = other.integer_;
            copy_from(other);
        }
        return *this;
    }
    value& operator=(value&& other) noexcept {
        if (this != &other) {
            destroy();
            type_ = other.type_;
            integer_ = other.integer_;
            move_from(std::move(other));
        }
        return *this;
    }
    ~value() { destroy(); }

    value_type type() const { return type_; }
    bool is_null() const { return type_ == value_type::null; }
    bool is_bool() const { return type_ == value_type::boolean; }
    bool is_integer() const { return type_ == value_type::integer; }
    bool is_real() const { return type_ == value_type::real; }
    bool is_string() const { return type_ == value_type::string; }
    bool is_array() const { return type_ == value_type::array; }
    bool is_object() const { return type_ == value_type::object; }

    bool as_bool() const {
        if (type_ == value_type::boolean) return boolean_;
        if (type_ == value_type::integer) return integer_ != 0;
        return false;
    }
    std::int64_t as_integer() const {
        if (type_ == value_type::integer) return integer_;
        if (type_ == value_type::real) return static_cast<std::int64_t>(real_);
        return 0;
    }
    double as_real() const {
        if (type_ == value_type::real) return real_;
        if (type_ == value_type::integer) return static_cast<double>(integer_);
        return 0.0;
    }
    const std::string& as_string() const {
        static std::string empty;
        return string_ ? *string_ : empty;
    }

    std::size_t size() const {
        if (array_) return array_->size();
        if (object_) return object_->size();
        return 0;
    }

    value& operator[](std::size_t index) {
        if (!array_) {
            array_ = new std::vector<value>();
            type_ = value_type::array;
        }
        if (index >= array_->size()) array_->resize(index + 1);
        return (*array_)[index];
    }

    value& operator[](std::string_view key) {
        if (!object_) {
            object_ = new std::map<std::string, value>();
            type_ = value_type::object;
        }
        return (*object_)[std::string(key)];
    }

    const value& operator[](std::string_view key) const {
        static value null_val;
        if (!object_) return null_val;
        auto it = object_->find(std::string(key));
        if (it == object_->end()) return null_val;
        return it->second;
    }

    std::string dump(int indent = 0) const {
        std::string result;
        dump_to(result, indent, 0);
        return result;
    }

    static value parse_file(std::string_view path) {
        auto fpath = std::string(path);
        std::ifstream file(fpath);
        if (!file) return value{};
        std::stringstream ss;
        ss << file.rdbuf();
        return parse(ss.str());
    }

    static value parse(std::string_view json) {
        std::size_t pos = 0;
        skip_whitespace(json, pos);
        if (pos >= json.size()) return value{};
        auto result = parse_value(json, pos);
        return result;
    }

private:
    void destroy() {
        if (type_ == value_type::string) delete string_;
        if (type_ == value_type::array) delete array_;
        if (type_ == value_type::object) delete object_;
        string_ = nullptr;
        array_ = nullptr;
        object_ = nullptr;
    }

    void copy_from(const value& other) {
        if (other.type_ == value_type::string) string_ = new std::string(*other.string_);
        else if (other.type_ == value_type::array) array_ = new std::vector<value>(*other.array_);
        else if (other.type_ == value_type::object) object_ = new std::map<std::string, value>(*other.object_);
    }

    void move_from(value&& other) {
        string_ = other.string_;
        array_ = other.array_;
        object_ = other.object_;
        other.string_ = nullptr;
        other.array_ = nullptr;
        other.object_ = nullptr;
        other.type_ = value_type::null;
    }

    void dump_to(std::string& result, int indent, int depth) const {
        std::string ind(depth * indent, ' ');
        std::string child_ind((depth + 1) * indent, ' ');

        switch (type_) {
        case value_type::null:
            result += "null"; break;
        case value_type::boolean:
            result += boolean_ ? "true" : "false"; break;
        case value_type::integer:
            result += std::to_string(integer_); break;
        case value_type::real: {
            auto s = std::to_string(real_);
            if (s.find('.') == std::string::npos && s.find('e') == std::string::npos) s += ".0";
            result += s;
            break;
        }
        case value_type::string:
            result += "\"" + escape(*string_) + "\""; break;
        case value_type::array:
            if (array_->empty()) { result += "[]"; break; }
            result += "[\n";
            for (std::size_t i = 0; i < array_->size(); ++i) {
                result += child_ind;
                (*array_)[i].dump_to(result, indent, depth + 1);
                if (i + 1 < array_->size()) result += ",";
                result += "\n";
            }
            result += ind + "]";
            break;
        case value_type::object:
            if (object_->empty()) { result += "{}"; break; }
            result += "{\n";
            bool first = true;
            for (auto& [k, v] : *object_) {
                if (!first) result += ",\n";
                result += child_ind + "\"" + escape(k) + "\": ";
                v.dump_to(result, indent, depth + 1);
                first = false;
            }
            result += "\n" + ind + "}";
            break;
        }
    }

    static void skip_whitespace(std::string_view s, std::size_t& pos) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
            ++pos;
    }

    static value parse_value(std::string_view s, std::size_t& pos) {
        skip_whitespace(s, pos);
        if (pos >= s.size()) return value{};
        if (s[pos] == '"') return parse_string(s, pos);
        if (s[pos] == '{') return parse_object(s, pos);
        if (s[pos] == '[') return parse_array(s, pos);
        if (s[pos] == 't' || s[pos] == 'f') return parse_bool(s, pos);
        if (s[pos] == 'n') return parse_null(s, pos);
        return parse_number(s, pos);
    }

    static value parse_string(std::string_view s, std::size_t& pos) {
        if (s[pos] != '"') return value{};
        ++pos;
        std::string result;
        while (pos < s.size() && s[pos] != '"') {
            if (s[pos] == '\\') {
                ++pos;
                if (pos >= s.size()) break;
                switch (s[pos]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u': {
                    if (pos + 4 < s.size()) {
                        auto hex = s.substr(pos + 1, 4);
                        auto code = static_cast<char>(std::stoul(std::string(hex), nullptr, 16));
                        result += code;
                        pos += 4;
                    }
                    break;
                }
                default: result += s[pos]; break;
                }
            } else {
                result += s[pos];
            }
            ++pos;
        }
        if (pos < s.size()) ++pos;
        return value(result);
    }

    static value parse_number(std::string_view s, std::size_t& pos) {
        std::size_t start = pos;
        bool is_real = false;
        if (pos < s.size() && s[pos] == '-') ++pos;
        while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos;
        if (pos < s.size() && s[pos] == '.') { is_real = true; ++pos; while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos; }
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) { is_real = true; ++pos; if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos; while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') ++pos; }
        auto num_str = s.substr(start, pos - start);
        if (is_real) return value(std::stod(std::string(num_str)));
        return value(static_cast<std::int64_t>(std::stoll(std::string(num_str))));
    }

    static value parse_bool(std::string_view s, std::size_t& pos) {
        if (s.substr(pos, 4) == "true") { pos += 4; return value(true); }
        if (s.substr(pos, 5) == "false") { pos += 5; return value(false); }
        return value{};
    }

    static value parse_null(std::string_view s, std::size_t& pos) {
        if (s.substr(pos, 4) == "null") { pos += 4; return value{}; }
        return value{};
    }

    static value parse_array(std::string_view s, std::size_t& pos) {
        if (s[pos] != '[') return value{};
        ++pos;
        value arr;
        arr.type_ = value_type::array;
        arr.array_ = new std::vector<value>();
        skip_whitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') { ++pos; return arr; }
        while (pos < s.size()) {
            arr.array_->push_back(parse_value(s, pos));
            skip_whitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; skip_whitespace(s, pos); }
            else if (pos < s.size() && s[pos] == ']') { ++pos; break; }
        }
        return arr;
    }

    static value parse_object(std::string_view s, std::size_t& pos) {
        if (s[pos] != '{') return value{};
        ++pos;
        value obj;
        obj.type_ = value_type::object;
        obj.object_ = new std::map<std::string, value>();
        skip_whitespace(s, pos);
        if (pos < s.size() && s[pos] == '}') { ++pos; return obj; }
        while (pos < s.size()) {
            skip_whitespace(s, pos);
            auto key = parse_string(s, pos);
            skip_whitespace(s, pos);
            if (pos < s.size() && s[pos] == ':') ++pos;
            auto val = parse_value(s, pos);
            (*obj.object_)[key.as_string()] = std::move(val);
            skip_whitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') { ++pos; }
            else if (pos < s.size() && s[pos] == '}') { ++pos; break; }
        }
        return obj;
    }

    static std::string escape(std::string_view s) {
        std::string result;
        for (auto c : s) {
            switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
            }
        }
        return result;
    }

    value_type type_;
    union {
        bool boolean_;
        std::int64_t integer_;
        double real_;
        std::string* string_;
        std::vector<value>* array_;
        std::map<std::string, value>* object_;
    };
};

} // namespace json
