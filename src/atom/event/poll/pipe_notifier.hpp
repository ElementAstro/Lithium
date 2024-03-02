/*
 * pipe_notifier.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: PipeNotifier for poll

**************************************************/

#ifndef __PipeNotifier_HPP
#define __PipeNotifier_HPP

#include "Notifier.h"
#include "utils/utils.h"


ATOM_NS_BEGIN

class PipeNotifier : public Notifier {
public:
    enum { READ_FD = 0, WRITE_FD };
    ~PipeNotifier() { cleanup(); }
    bool init() override {
        cleanup();
        if (::pipe(fds_) != 0) {
            cleanup();
            return false;
        } else {
            ::fcntl(fds_[READ_FD], F_SETFL, O_RDONLY | O_NONBLOCK);
            ::fcntl(fds_[WRITE_FD], F_SETFL, O_WRONLY | O_NONBLOCK);
#ifdef FD_CLOEXEC
            ::fcntl(fds_[READ_FD], F_SETFD,
                    ::fcntl(fds_[READ_FD], F_GETFD) | FD_CLOEXEC);
            ::fcntl(fds_[WRITE_FD], F_SETFD,
                    ::fcntl(fds_[WRITE_FD], F_GETFD) | FD_CLOEXEC);
#endif
            return true;
        }
    }
    bool ready() override {
        return fds_[READ_FD] != INVALID_FD && fds_[WRITE_FD] != INVALID_FD;
    }
    void notify() override {
        if (fds_[WRITE_FD] != INVALID_FD) {
            ssize_t ret = 0;
            do {
                char c = 1;
                ret = ::write(fds_[WRITE_FD], &c, sizeof(c));
            } while (ret < 0 && errno == EINTR);
        }
    }

    SOCKET_FD getReadFD() override { return fds_[READ_FD]; }

    Result onEvent(KMEvent ev) override {
        char buf[1024];
        ssize_t ret = 0;
        do {
            ret = ::read(fds_[READ_FD], buf, sizeof(buf));
        } while (ret == sizeof(buf) || (ret < 0 && errno == EINTR));
        return Result::OK;
    }

private:
    void cleanup() {
        if (fds_[READ_FD] != INVALID_FD) {
            ::close(fds_[READ_FD]);
            fds_[READ_FD] = INVALID_FD;
        }
        if (fds_[WRITE_FD] != INVALID_FD) {
            ::close(fds_[WRITE_FD]);
            fds_[WRITE_FD] = INVALID_FD;
        }
    }

    SOCKET_FD fds_[2]{INVALID_FD, INVALID_FD};
};

ATOM_NS_END

#endif
