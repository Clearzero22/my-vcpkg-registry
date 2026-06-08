#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif

namespace net {

#ifdef _WIN32
using socket_t = SOCKET;
constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
using socket_t = int;
constexpr socket_t invalid_socket = -1;
#endif

class tcp_client {
public:
    tcp_client() {
#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    }

    ~tcp_client() {
        disconnect();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    tcp_client(const tcp_client&) = delete;
    tcp_client& operator=(const tcp_client&) = delete;

    tcp_client(tcp_client&& other) noexcept
        : sock_(other.sock_), connected_(other.connected_.load()) {
        other.sock_ = invalid_socket;
        other.connected_.store(false);
    }

    tcp_client& operator=(tcp_client&& other) noexcept {
        if (this != &other) {
            disconnect();
            sock_ = other.sock_;
            connected_.store(other.connected_.load());
            other.sock_ = invalid_socket;
            other.connected_.store(false);
        }
        return *this;
    }

    bool connect(std::string_view host, std::uint16_t port, int timeout_ms = 5000) {
        if (connected_.load()) disconnect();

        struct addrinfo hints{};
        struct addrinfo* result = nullptr;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        auto port_str = std::to_string(port);
        auto addr_ret = getaddrinfo(host.data(), port_str.c_str(), &hints, &result);
        if (addr_ret != 0 || result == nullptr) return false;

        for (auto* rp = result; rp != nullptr; rp = rp->ai_next) {
            sock_ = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sock_ == invalid_socket) continue;

#ifdef _WIN32
            u_long mode = 1;
            ioctlsocket(sock_, FIONBIO, &mode);
#else
            auto flags = fcntl(sock_, F_GETFL, 0);
            fcntl(sock_, F_SETFL, flags | O_NONBLOCK);
#endif

            ::connect(sock_, rp->ai_addr, static_cast<int>(rp->ai_addrlen));

            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(sock_, &fdset);
            struct timeval tv{};
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            auto ret = select(static_cast<int>(sock_) + 1, nullptr, &fdset, nullptr, &tv);
            if (ret > 0) {
                int so_error = 0;
                socklen_t len = sizeof(so_error);
                getsockopt(sock_, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &len);
                if (so_error == 0) {
                    set_blocking(true);
                    connected_.store(true);
                    freeaddrinfo(result);
                    return true;
                }
            }

#ifdef _WIN32
            closesocket(sock_);
#else
            ::close(sock_);
#endif
            sock_ = invalid_socket;
        }

        freeaddrinfo(result);
        return false;
    }

    void disconnect() {
        if (sock_ != invalid_socket) {
#ifdef _WIN32
            shutdown(sock_, SD_BOTH);
            closesocket(sock_);
#else
            ::shutdown(sock_, SHUT_RDWR);
            ::close(sock_);
#endif
            sock_ = invalid_socket;
        }
        connected_.store(false);
    }

    bool is_connected() const {
        return connected_.load();
    }

    int send(std::span<const std::byte> data) {
        if (!connected_.load()) return -1;
        auto ret = ::send(sock_, reinterpret_cast<const char*>(data.data()),
                          static_cast<int>(data.size()), 0);
        if (ret == SOCKET_ERROR_NEG) {
            disconnect();
        }
        return static_cast<int>(ret);
    }

    int send(std::string_view data) {
        return send(std::as_bytes(std::span(data)));
    }

    int receive(std::span<std::byte> buffer) {
        if (!connected_.load()) return -1;
        auto ret = ::recv(sock_, reinterpret_cast<char*>(buffer.data()),
                          static_cast<int>(buffer.size()), 0);
        if (ret <= 0) {
            disconnect();
            return -1;
        }
        return static_cast<int>(ret);
    }

    std::optional<std::string> receive_until(std::string_view delimiter) {
        std::string buffer;
        std::array<std::byte, 1024> chunk{};
        while (true) {
            auto ret = recv_raw(chunk);
            if (ret <= 0) return std::nullopt;
            buffer.append(reinterpret_cast<const char*>(chunk.data()), static_cast<std::size_t>(ret));
            auto pos = buffer.find(delimiter);
            if (pos != std::string::npos) {
                buffer.resize(pos + delimiter.size());
                return buffer;
            }
        }
    }

    void set_timeout(int ms) {
        if (sock_ == invalid_socket) return;
        auto tv = ms;
#ifdef _WIN32
        setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
        setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
#else
        struct timeval timeout{};
        timeout.tv_sec = ms / 1000;
        timeout.tv_usec = (ms % 1000) * 1000;
        setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif
    }

    void set_nodelay(bool enable) {
        if (sock_ == invalid_socket) return;
        int opt = enable ? 1 : 0;
        setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));
    }

    socket_t native_handle() const {
        return sock_;
    }

private:
#ifdef _WIN32
    static constexpr int SOCKET_ERROR_NEG = SOCKET_ERROR;
#else
    static constexpr int SOCKET_ERROR_NEG = -1;
#endif

    int recv_raw(std::span<std::byte> buffer) {
        if (!connected_.load()) return -1;
        auto ret = ::recv(sock_, reinterpret_cast<char*>(buffer.data()),
                          static_cast<int>(buffer.size()), 0);
        if (ret <= 0) {
            disconnect();
            return -1;
        }
        return static_cast<int>(ret);
    }

    void set_blocking(bool blocking) {
        if (sock_ == invalid_socket) return;
#ifdef _WIN32
        u_long mode = blocking ? 0 : 1;
        ioctlsocket(sock_, FIONBIO, &mode);
#else
        auto flags = fcntl(sock_, F_GETFL, 0);
        if (blocking) {
            fcntl(sock_, F_SETFL, flags & ~O_NONBLOCK);
        } else {
            fcntl(sock_, F_SETFL, flags | O_NONBLOCK);
        }
#endif
    }

    socket_t sock_ = invalid_socket;
    std::atomic<bool> connected_{false};
};

} // namespace net
