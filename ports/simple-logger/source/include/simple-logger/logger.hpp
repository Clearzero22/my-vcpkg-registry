#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace simple {

enum class log_level {
    debug,
    info,
    warn,
    error,
    fatal
};

inline std::string_view level_to_string(log_level lv) {
    switch (lv) {
        case log_level::debug: return "DEBUG";
        case log_level::info:  return "INFO";
        case log_level::warn:  return "WARN";
        case log_level::error: return "ERROR";
        case log_level::fatal: return "FATAL";
    }
    return "UNKNOWN";
}

class sink {
public:
    virtual ~sink() = default;
    virtual void write(std::string_view message) = 0;
};

class console_sink : public sink {
public:
    void write(std::string_view message) override {
        std::fwrite(message.data(), 1, message.size(), stdout);
        std::fputc('\n', stdout);
        std::fflush(stdout);
    }
};

class file_sink : public sink {
public:
    file_sink(std::string_view path, std::size_t max_size, int max_files)
        : base_path_(path), max_size_(max_size), max_files_(max_files) {
        rotate();
    }

    void write(std::string_view message) override {
        if (!file_) return;
        file_ << message << "\n";
        file_.flush();
        if (file_.tellp() > static_cast<std::streamoff>(max_size_)) {
            file_.close();
            rotate();
        }
    }

private:
    void rotate() {
        file_.close();
        for (int i = max_files_ - 1; i > 0; --i) {
            auto old_name = rotate_name(i);
            auto new_name = rotate_name(i + 1);
            std::filesystem::rename(old_name, new_name);
        }
        if (std::filesystem::exists(base_path_)) {
            std::filesystem::rename(base_path_, rotate_name(1));
        }
        file_.open(base_path_, std::ios::binary | std::ios::out);
    }

    std::string rotate_name(int index) const {
        if (index == 0) return std::string(base_path_);
        auto path = std::filesystem::path(base_path_);
        auto stem = path.stem().string();
        auto ext = path.extension().string();
        return stem + "." + std::to_string(index) + ext;
    }

    std::ofstream file_;
    std::string base_path_;
    std::size_t max_size_;
    int max_files_;
};

class logger {
public:
    static logger& instance() {
        static logger inst;
        return inst;
    }

    void set_level(log_level lv) {
        std::lock_guard<std::mutex> lock(mutex_);
        level_ = lv;
    }

    void add_console_sink() {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks_.push_back(std::make_unique<console_sink>());
    }

    void add_file_sink(std::string_view path, std::size_t max_size = 5 * 1024 * 1024, int max_files = 3) {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks_.push_back(std::make_unique<file_sink>(path, max_size, max_files));
    }

    void log(log_level lv, std::string_view tag, std::string_view msg) {
        if (lv < level_) return;

        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        auto tm = *std::gmtime(&tt);
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
            << "." << std::setw(3) << std::setfill('0') << ms.count()
            << " [" << level_to_string(lv) << "]"
            << " [" << tag << "] "
            << msg;

        auto formatted = oss.str();
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& s : sinks_) {
            s->write(formatted);
        }
    }

private:
    logger() = default;
    ~logger() = default;
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

    log_level level_ = log_level::debug;
    std::vector<std::unique_ptr<sink>> sinks_;
    std::mutex mutex_;
};

} // namespace simple

#define LOG_DEBUG(tag, msg)   simple::logger::instance().log(simple::log_level::debug, tag, msg)
#define LOG_INFO(tag, msg)    simple::logger::instance().log(simple::log_level::info,  tag, msg)
#define LOG_WARN(tag, msg)    simple::logger::instance().log(simple::log_level::warn,  tag, msg)
#define LOG_ERROR(tag, msg)   simple::logger::instance().log(simple::log_level::error, tag, msg)
#define LOG_FATAL(tag, msg)   simple::logger::instance().log(simple::log_level::fatal, tag, msg)
