#include "connection.hpp"

#include "atom/error/exception.hpp"

#ifdef _WIN32
#include <Winsock2.h>
#define POLL WSAPoll
#else
#include <poll.h>
#define POLL poll
#endif

GuiderConnection::GuiderConnection() : m_terminate_(false) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

GuiderConnection::~GuiderConnection() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

auto GuiderConnection::connect(const char *hostname,
                               unsigned short port) -> bool {
    disconnect();

    m_terminate_ = false;

    m_curl_ = curl_easy_init();
    if (m_curl_ == nullptr) {
        return false;
    }

    std::ostringstream osa;
    osa << "http://" << hostname << ':' << port;

    curl_easy_setopt(m_curl_, CURLOPT_URL, osa.str().c_str());
    curl_easy_setopt(m_curl_, CURLOPT_CONNECT_ONLY, 1L);

    try {
        CURLcode res = curl_easy_perform(m_curl_);
        if (res != CURLE_OK) {
            THROW_CURL_RUNTIME_ERROR("curl_easy_perform() failed: ",
                                     static_cast<int>(res));
        }

        res = curl_easy_getinfo(m_curl_, CURLINFO_ACTIVESOCKET, &m_sockfd_);
        if (res != CURLE_OK) {
            THROW_CURL_RUNTIME_ERROR("curl_easy_getinfo() failed: ",
                                     static_cast<int>(res));
        }

        return true;
    } catch (CURLcode) {
        curl_easy_cleanup(m_curl_);
        m_curl_ = nullptr;
        return false;
    }
}

void GuiderConnection::disconnect() {
    if (m_curl_ != nullptr) {
        curl_easy_cleanup(m_curl_);
        m_curl_ = nullptr;
    }
}

auto GuiderConnection::waitReadable(std::stop_token st) const -> bool {
    struct pollfd pfd;
    pfd.fd = m_sockfd_;
    pfd.events = POLLIN;

    while (!st.stop_requested()) {
        int ret = POLL(&pfd, 1, 500);
        if (ret == 1) {
            return true;
        }
    }
    return false;
}

std::optional<std::string> GuiderConnection::readLine() {
    while (m_dq_.empty()) {
        char buf[1024];
        size_t nbuf;

        while (true) {
            CURLcode res = curl_easy_recv(m_curl_, buf, sizeof(buf), &nbuf);
            if (res == CURLE_OK) {
                break;
            }
            if (res == CURLE_AGAIN) {
                if (!waitReadable(m_thread_.get_stop_token())) {
                    return std::nullopt;
                }
            } else {
                return std::nullopt;
            }
        }

        const char *p0 = buf;
        const char *p = p0;
        while (p < buf + nbuf) {
            if (*p == '\r' || *p == '\n') {
                m_os_.write(p0, p - p0);
                if (m_os_.tellp() > 0) {
                    m_dq_.push_back(m_os_.str());
                    m_os_.str("");
                }
                p0 = ++p;
            } else {
                ++p;
            }
        }
        m_os_.write(p0, p - p0);
    }

    std::string sret = std::move(m_dq_.front());
    m_dq_.pop_front();

    return sret;
}

auto GuiderConnection::waitWritable(std::stop_token st) const -> bool {
    struct pollfd pfd;
    pfd.fd = m_sockfd_;
    pfd.events = POLLOUT;

    while (!st.stop_requested()) {
        int ret = POLL(&pfd, 1, -1);
        if (ret == 1) {
            return true;
        }
    }
    return false;
}

auto GuiderConnection::writeLine(const std::string &s) -> bool {
    size_t rem = s.size();
    const char *pos = s.c_str();

    while (rem > 0) {
        size_t nwr;
        CURLcode res = curl_easy_send(m_curl_, pos, rem, &nwr);
        if (res == CURLE_AGAIN) {
            if (!waitWritable(m_thread_.get_stop_token())) {
                return false;
            }
            continue;
        }
        if (res != CURLE_OK) {
            return false;
        }
        pos += nwr;
        rem -= nwr;
    }

    return true;
}

void GuiderConnection::terminate() {
    m_terminate_ = true;
    if (m_thread_.joinable()) {
        m_thread_.request_stop();
        m_thread_.join();
    }
}