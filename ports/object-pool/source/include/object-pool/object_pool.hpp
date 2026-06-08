#pragma once

#include <atomic>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>

namespace pool {

template<typename T>
class object_pool {
public:
    explicit object_pool(std::size_t initial_size = 16, bool thread_safe = true)
        : thread_safe_(thread_safe) {
        reserve(initial_size);
    }

    template<typename... Args>
    std::shared_ptr<T> acquire(Args&&... args) {
        T* ptr = nullptr;

        if (thread_safe_) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!pool_.empty()) {
                ptr = pool_.back();
                pool_.pop_back();
            }
        } else {
            if (!pool_.empty()) {
                ptr = pool_.back();
                pool_.pop_back();
            }
        }

        if (ptr) {
            if constexpr (std::is_default_constructible_v<T>) {
                if constexpr (requires { ptr->reset(); }) {
                    ptr->reset();
                } else {
                    *ptr = T(std::forward<Args>(args)...);
                }
            }
        } else {
            ptr = new T(std::forward<Args>(args)...);
        }

        return std::shared_ptr<T>(ptr, [this](T* obj) {
            this->release(obj);
        });
    }

    void release(T* obj) {
        if (thread_safe_) {
            std::lock_guard<std::mutex> lock(mutex_);
            pool_.push_back(obj);
        } else {
            pool_.push_back(obj);
        }
    }

    std::size_t available() const {
        if (thread_safe_) {
            std::lock_guard<std::mutex> lock(mutex_);
            return pool_.size();
        }
        return pool_.size();
    }

    std::size_t total() const {
        if (thread_safe_) {
            std::lock_guard<std::mutex> lock(mutex_);
            return pool_.size() + allocated_;
        }
        return pool_.size() + allocated_;
    }

    void reserve(std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            auto* ptr = new T();
            if (thread_safe_) {
                std::lock_guard<std::mutex> lock(mutex_);
                pool_.push_back(ptr);
                ++allocated_;
            } else {
                pool_.push_back(ptr);
                ++allocated_;
            }
        }
    }

    void clear() {
        if (thread_safe_) {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto* ptr : pool_) {
                delete ptr;
            }
            pool_.clear();
            allocated_ = 0;
        } else {
            for (auto* ptr : pool_) {
                delete ptr;
            }
            pool_.clear();
            allocated_ = 0;
        }
    }

    ~object_pool() {
        clear();
    }

    object_pool(const object_pool&) = delete;
    object_pool& operator=(const object_pool&) = delete;
    object_pool(object_pool&&) = delete;
    object_pool& operator=(object_pool&&) = delete;

private:
    mutable std::mutex mutex_;
    std::vector<T*> pool_;
    std::atomic<std::size_t> allocated_{0};
    bool thread_safe_;
};

} // namespace pool
