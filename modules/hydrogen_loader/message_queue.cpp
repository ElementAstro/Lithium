#include "message_queue.hpp"

#ifdef _WIN32
#include <WinSock2.h> // 包含 WinSock2 库
#include <MSWSock.h>  // 包含 MSWSock 库
#include <ws2def.h>
#include <windows.h>
#else
#include <sys/socket.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#include "io.hpp"
#include "hydrogen_server.hpp"

#include "atom/log/loguru.hpp"

MsgQueue::MsgQueue(bool useSharedBuffer) : useSharedBuffer(useSharedBuffer)
{
    lp = newLilXML();
#ifdef USE_LIBUV
    uv_poll_init(uv_default_loop(), &rio, rFd);
    uv_poll_init(uv_default_loop(), &wio, wFd);
    rio.data = this;
    wio.data = this;
#else
    rio.set<MsgQueue, &MsgQueue::ioCb>(this);
    wio.set<MsgQueue, &MsgQueue::ioCb>(this);
#endif
    rFd = -1;
    wFd = -1;
}

MsgQueue::~MsgQueue()
{
#ifdef USE_LIBUV
    uv_poll_stop(&rio);
    uv_poll_stop(&wio);
#else
    rio.stop();
    wio.stop();
#endif

    clearMsgQueue();
    delLilXML(lp);
    lp = nullptr;

    setFds(-1, -1);

    /* unreference messages queue for this client */
    auto msgqcp = msgq;
    msgq.clear();
    for (auto mp : msgqcp)
    {
        mp->release(this);
    }
}

void MsgQueue::closeWritePart()
{
    if (wFd == -1)
    {
        // Already closed
        return;
    }

    int oldWFd = wFd;

    wFd = -1;
    // Clear the queue and stop the io slot
    clearMsgQueue();

    if (oldWFd == rFd)
    {
#ifdef _WIN32
        if (shutdown(static_cast<SOCKET>(oldWFd), SD_SEND) == SOCKET_ERROR)
#else
        if (shutdown(oldWFd, SHUT_WR) == -1)
#endif
        {
            if (errno != ENOTCONN)
            {
                LOG_F(ERROR, "socket shutdown failed: %s\n", strerror(errno));
                close();
            }
        }
    }
    else
    {
        if (::close(oldWFd) == -1)
        {
            // log(fmt("socket close failed: %s\n", strerror(errno)));
            close();
        }
    }
}

#ifdef USE_LIBUV
void MsgQueue::setFds(int rFd, int wFd)
{
    if (this->rFd != -1)
    {
        uv_poll_stop(&rio);
        uv_poll_stop(&wio);
        ::close(this->rFd);
        if (this->rFd != this->wFd)
        {
            ::close(this->wFd);
        }
    }
    else if (this->wFd != -1)
    {
        uv_poll_stop(&wio);
        ::close(this->wFd);
    }

    this->rFd = rFd;
    this->wFd = wFd;
    this->nsent.reset();

    if (rFd != -1)
    {
#ifdef _WIN32
        int fd = _fileno(reinterpret_cast<FILE *>(_get_osfhandle(rFd)));
        _setmode(fd, _O_BINARY);
#else
        fcntl(rFd, F_SETFL, fcntl(rFd, F_GETFL, 0) | O_NONBLOCK);
#endif

        if (wFd != rFd)
        {
#ifdef _WIN32
            int fd = _fileno(reinterpret_cast<FILE *>(_get_osfhandle(wFd)));
            _setmode(fd, _O_BINARY);
#else
            fcntl(wFd, F_SETFL, fcntl(wFd, F_GETFL, 0) | O_NONBLOCK);
#endif
        }

        uv_poll_init(uv_default_loop(), &rio, rFd);
        uv_poll_init(uv_default_loop(), &wio, wFd);
        rio.data = this;
        wio.data = this;
        updateIos();
    }
}
#else
void MsgQueue::setFds(int rFd, int wFd)
{
    if (this->rFd != -1)
    {
        rio.stop();
        wio.stop();
        ::close(this->rFd);
        if (this->rFd != this->wFd)
        {
            ::close(this->wFd);
        }
    }
    else if (this->wFd != -1)
    {
        wio.stop();
        ::close(this->wFd);
    }

    this->rFd = rFd;
    this->wFd = wFd;
    this->nsent.reset();

    if (rFd != -1)
    {
#ifdef _WIN32
        int fd = _fileno(reinterpret_cast<FILE *>(_get_osfhandle(rFd)));
        _setmode(fd, _O_BINARY);
#else
        fcntl(rFd, F_SETFL, fcntl(rFd, F_GETFL, 0) | O_NONBLOCK);
#endif

        if (wFd != rFd)
        {
#ifdef _WIN32
            int fd = _fileno(reinterpret_cast<FILE *>(_get_osfhandle(wFd)));
            _setmode(fd, _O_BINARY);
#else
            fcntl(wFd, F_SETFL, fcntl(wFd, F_GETFL, 0) | O_NONBLOCK);
#endif
        }

        rio.set(rFd, ev::READ);
        wio.set(wFd, ev::WRITE);
        updateIos();
    }
}
#endif

SerializedMsg *MsgQueue::headMsg() const
{
    if (msgq.empty())
        return nullptr;
    return *(msgq.begin());
}

void MsgQueue::consumeHeadMsg()
{
    auto msg = headMsg();
    msgq.pop_front();
    msg->release(this);
    nsent.reset();

    updateIos();
}

void MsgQueue::pushMsg(Msg *mp)
{
    // Don't write messages to client that have been disconnected
    if (wFd == -1)
    {
        return;
    }

    auto serialized = mp->serialize(this);

    msgq.push_back(serialized);
    serialized->addAwaiter(this);

    // Register for client write
    updateIos();
}

#ifdef USE_LIBUV
void MsgQueue::updateIos()
{
    if (wFd != -1)
    {
        if (msgq.empty() || !msgq.front()->requestContent(nsent))
        {
            uv_poll_stop(&wio);
        }
        else
        {
            uv_poll_start(&wio, UV_WRITABLE, IOCallback);
        }
    }
    if (rFd != -1)
    {
        uv_poll_start(&rio, UV_READABLE, IOCallback);
    }
}

// IO事件回调函数
void MsgQueue::IOCallback(uv_poll_t *handle, int status, int events)
{
    MsgQueue *msgQueue = static_cast<MsgQueue *>(handle->data);
    if (status < 0)
    {
        // 处理错误情况
        return;
    }

    if (events & UV_READABLE)
    {
        // 处理可读事件
    }

    if (events & UV_WRITABLE)
    {
        // 处理可写事件
    }
}

#else
void MsgQueue::updateIos()
{
    if (wFd != -1)
    {
        if (msgq.empty() || !msgq.front()->requestContent(nsent))
        {
            wio.stop();
        }
        else
        {
            wio.start();
        }
    }
    if (rFd != -1)
    {
        rio.start();
    }
}
#endif

void MsgQueue::messageMayHaveProgressed(const SerializedMsg *msg)
{
    if ((!msgq.empty()) && (msgq.front() == msg))
    {
        updateIos();
    }
}

void MsgQueue::clearMsgQueue()
{
    nsent.reset();

    auto queueCopy = msgq;
    for (auto mp : queueCopy)
    {
        mp->release(this);
    }
    msgq.clear();

    // Cancel io write events
    updateIos();
#ifdef USE_LIBUV
    uv_poll_stop(&wio);
#else
    wio.stop();
#endif
}

unsigned long MsgQueue::msgQSize() const
{
    unsigned long l = 0;

    for (auto mp : msgq)
    {
        l += sizeof(Msg);
        l += mp->queueSize();
    }

    return (l);
}

#ifdef USE_LIBUV
void MsgQueue::ioCb(uv_poll_t *handle, int status, int revents)
{
    if (status < 0)
    {
        int sockErrno = readFdError(this->rFd);
        if ((!sockErrno) && this->wFd != this->rFd)
        {
            sockErrno = readFdError(this->wFd);
        }

        if (sockErrno)
        {
            // log(fmt("Communication error: %s\n", strerror(sockErrno)));
            close();
            return;
        }
    }

    if (UV_READABLE & revents)
        readFromFd();

    if (UV_WRITABLE & revents)
        writeToFd();
}

void MsgQueue::setReadWriteCallback()
{
    uv_poll_init(uv_default_loop(), &rio, rFd);
    uv_poll_init(uv_default_loop(), &wio, wFd);
    rio.data = this;
    wio.data = this;
    uv_poll_start(&rio, UV_READABLE, IOCallback);
    uv_poll_start(&wio, UV_WRITABLE, IOCallback);
}
#else
void MsgQueue::ioCb(ev::io &, int revents)
{
    if (EV_ERROR & revents)
    {
        int sockErrno = readFdError(this->rFd);
        if ((!sockErrno) && this->wFd != this->rFd)
        {
            sockErrno = readFdError(this->wFd);
        }

        if (sockErrno)
        {
            // log(fmt("Communication error: %s\n", strerror(sockErrno)));
            close();
            return;
        }
    }

    if (revents & EV_READ)
        readFromFd();

    if (revents & EV_WRITE)
        writeToFd();
}
#endif

#ifdef _WIN32
size_t MsgQueue::doRead(char *buf, size_t nr)
{
    if (!useSharedBuffer)
    {
        // 非共享缓冲区方式
        DWORD bytesToRead = static_cast<DWORD>(nr);
        DWORD bytesRead = 0;
        if (!ReadFile(reinterpret_cast<HANDLE>(rFd), buf, bytesToRead, &bytesRead, NULL))
        {
            // 错误处理
            return -1;
        }
        return bytesRead;
    }
    else
    {
        // 使用共享缓冲区方式
        WSABUF wsaBuf{nr, buf};
        DWORD bytesReceived = 0;
        DWORD flags = 0;
        WSAMSG wsaMsg;
        memset(&wsaMsg, 0x0, sizeof(wsaMsg));
        wsaMsg.name = nullptr;
        wsaMsg.namelen = 0;
        wsaMsg.lpBuffers = &wsaBuf;
        wsaMsg.dwBufferCount = 1;

        std::vector<char> controlBuffer(sizeof(WSACMSGHDR) + WSA_CMSG_SPACE(MAXFD_PER_MESSAGE * sizeof(HANDLE)));
        wsaMsg.Control.len = static_cast<ULONG>(controlBuffer.size());
        wsaMsg.Control.buf = controlBuffer.data();

        LPFN_WSARECVMSG pfnWSARecvMsg = nullptr;
        GUID guidWSARecvMsg = WSAID_WSARECVMSG;
        DWORD dwBytes = 0;
        SOCKET sock;
        if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidWSARecvMsg, sizeof(guidWSARecvMsg),
                     &pfnWSARecvMsg, sizeof(pfnWSARecvMsg), &dwBytes, nullptr, nullptr) != 0)
        {
            // 错误处理
            return -1;
        }

        int result = pfnWSARecvMsg(sock, &wsaMsg, &bytesReceived, nullptr, nullptr);
        if (result == SOCKET_ERROR)
        {
            // 错误处理
            return -1;
        }

        for (char *pData = (wsaMsg.Control.buf); pData < (wsaMsg.Control.buf + wsaMsg.Control.len);)
        {
            WSACMSGHDR *pHdr = reinterpret_cast<WSACMSGHDR *>(pData);
            if (pHdr->cmsg_level == SOL_SOCKET)
            {
                DWORD fdCount = 0;
                while (WSA_CMSG_NXTHDR(&wsaMsg, pHdr) != nullptr)
                {
                    pHdr = WSA_CMSG_NXTHDR(&wsaMsg, pHdr);
                    if (pHdr->cmsg_level == SOL_SOCKET)
                    {
                        fdCount++;
                    }
                }

                int *pfds = reinterpret_cast<int *>(WSA_CMSG_DATA(pHdr));
                for (int i = 0; i < fdCount; ++i)
                {
                    incomingSharedBuffers.push_back(reinterpret_cast<HANDLE>(static_cast<INT_PTR>(pfds[i])));
                }
            }
            pData += pHdr->cmsg_len;
        }
        return bytesReceived;
    }
}
#else
size_t MsgQueue::doRead(char *buf, size_t nr)
{
    if (!useSharedBuffer)
    {
        /* read client - works for all kinds of fds incl pipe*/
        return read(rFd, buf, sizeof(buf));
    }
    else
    {
        // Use recvmsg for ancillary data
        struct msghdr msgh;
        struct iovec iov;

        union
        {
            struct cmsghdr cmsgh;
            /* Space large enough to hold an 'int' */
            char control[CMSG_SPACE(MAXFD_PER_MESSAGE * sizeof(int))];
        } control_un;

        iov.iov_base = buf;
        iov.iov_len = nr;

        msgh.msg_name = NULL;
        msgh.msg_namelen = 0;
        msgh.msg_iov = &iov;
        msgh.msg_iovlen = 1;
        msgh.msg_flags = 0;
        msgh.msg_control = control_un.control;
        msgh.msg_controllen = sizeof(control_un.control);

        int recvflag;
#ifdef __linux__
        recvflag = MSG_CMSG_CLOEXEC;
#else
        recvflag = 0;
#endif
        int size = recvmsg(rFd, &msgh, recvflag);
        if (size == -1)
        {
            return -1;
        }

        for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL; cmsg = CMSG_NXTHDR(&msgh, cmsg))
        {
            if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
            {
                int fdCount = 0;
                while (cmsg->cmsg_len >= CMSG_LEN((fdCount + 1) * sizeof(int)))
                {
                    fdCount++;
                }
                // // log(fmt("Received %d fds\n", fdCount));
                int *fds = (int *)CMSG_DATA(cmsg);
                for (int i = 0; i < fdCount; ++i)
                {
#ifndef __linux__
                    fcntl(fds[i], F_SETFD, FD_CLOEXEC);
#endif
                    incomingSharedBuffers.push_back(fds[i]);
                }
            }
            else
            {
                // log(fmt("Ignoring ancillary data level %d, type %d\n", cmsg->cmsg_level, cmsg->cmsg_type));
            }
        }
        return size;
    }
}
#endif

void MsgQueue::readFromFd()
{
    char buf[MAXRBUF];
    ssize_t nr;

    /* read client */
    nr = doRead(buf, sizeof(buf));
    if (nr <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        if (nr < 0)
            LOG_F(ERROR, "read: %s\n", strerror(errno));
        else if (verbose > 0)
            LOG_F(ERROR, "read EOF\n");
        close();
        return;
    }

    /* process XML chunk */
    char err[1024];
    XMLEle **nodes = parseXMLChunk(lp, buf, nr, err);
    if (!nodes)
    {
        // log(fmt("XML error: %s\n", err));
        // log(fmt("XML read: %.*s\n", (int)nr, buf));
        close();
        return;
    }

    int inode = 0;

    XMLEle *root = nodes[inode];
    // Stop processing message in case of deletion...
    auto hb = heartBeat();
    while (root)
    {
        if (hb.alive())
        {
            if (verbose > 2)
                traceMsg("read ", root);
            else if (verbose > 1)
            {
                DLOG_F(INFO, "read <%s device='%s' name='%s'>\n", tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"));
            }

            onMessage(root, incomingSharedBuffers);
        }
        else
        {
            // Otherwise, client got killed. Just release pending messages
            delXMLEle(root);
        }
        inode++;
        root = nodes[inode];
    }

    free(nodes);
}

#ifdef _WIN32
void MsgQueue::writeToFd()
{
    ssize_t nw;
    void *data;
    ssize_t nsend;
    std::vector<int> sharedBuffers;

    // 获取当前消息
    auto mp = headMsg();
    if (mp == nullptr)
    {
        LOG_F(ERROR, "Unexpected write notification");
        return;
    }

    do
    {
        if (!mp->getContent(nsent, data, nsend, sharedBuffers))
        {
            wio.stop();
            return;
        }

        if (nsend == 0)
        {
            consumeHeadMsg();
            mp = headMsg();
            if (mp == nullptr)
            {
                return;
            }
        }
    } while (nsend == 0);

    // 发送下一个块，不超过MAXWSIZ以减少阻塞
    if (nsend > MAXWSIZ)
        nsend = MAXWSIZ;

    if (!useSharedBuffer)
    {
        nw = write(static_cast<SOCKET>(wFd), reinterpret_cast<const char *>(data), static_cast<int>(nsend));
    }
    else
    {
        WSAMSG msgh{};
        WSABUF iov[1]{};
        int cmsghdrlength;
        std::vector<char> controlBuffer;

        int fdCount = sharedBuffers.size();
        if (fdCount > 0)
        {
            if (fdCount > MAXFD_PER_MESSAGE)
            {
                LOG_F(ERROR, "attempt to send too many FD\n");
                close();
                return;
            }

            cmsghdrlength = WSA_CMSG_SPACE(fdCount * sizeof(int));
            controlBuffer.resize(cmsghdrlength);

            // 填充控制信息
            msgh.Control.buf = controlBuffer.data();
            msgh.Control.len = cmsghdrlength;
            LPWSACMSGHDR pCmsgHdr = WSA_CMSG_FIRSTHDR(&msgh);
            pCmsgHdr->cmsg_level = SOL_SOCKET;
            pCmsgHdr->cmsg_len = cmsghdrlength;

            int *fdPtr = reinterpret_cast<int *>(WSA_CMSG_DATA(pCmsgHdr));
            for (int i = 0; i < fdCount; ++i)
            {
                fdPtr[i] = sharedBuffers[i];
            }
        }
        else
        {
            cmsghdrlength = 0;
            msgh.Control.buf = nullptr;
            msgh.Control.len = cmsghdrlength;
        }

        iov[0].buf = reinterpret_cast<char *>(data);
        iov[0].len = static_cast<ULONG>(nsend);

        msgh.dwFlags = 0;
        msgh.name = nullptr;
        msgh.namelen = 0;
        msgh.lpBuffers = iov;
        msgh.dwBufferCount = 1;

        nw = WSASendMsg(static_cast<SOCKET>(wFd), &msgh, 0, nullptr, nullptr, nullptr);

        controlBuffer.clear();
    }

    // 发生错误时关闭写入部分
    if (nw <= 0)
    {
        if (nw == 0)
            DLOG_F(INFO, "write returned 0\n");
        else
            LOG_F(ERROR, "write: %s\n", strerror(errno));

        // 保持读取部分打开
        closeWritePart();
        return;
    }

    // 更新发送量，完成后释放消息并从队列中弹出
    mp->advance(nsent, nw);
    if (nsent.done())
        consumeHeadMsg();
}

#else
void MsgQueue::writeToFd()
{
    ssize_t nw;
    void *data;
    ssize_t nsend;
    std::vector<int> sharedBuffers;

    /* get current message */
    auto mp = headMsg();
    if (mp == nullptr)
    {
        LOG_F(ERROR, "Unexpected write notification");
        return;
    }

    do
    {
        if (!mp->getContent(nsent, data, nsend, sharedBuffers))
        {
#ifdef USE_LIBUV
            uv_poll_stop(&wio);
#else
            wio.stop();
#endif
            return;
        }

        if (nsend == 0)
        {
            consumeHeadMsg();
            mp = headMsg();
            if (mp == nullptr)
            {
                return;
            }
        }
    } while (nsend == 0);

    /* send next chunk, never more than MAXWSIZ to reduce blocking */
    if (nsend > MAXWSIZ)
        nsend = MAXWSIZ;

    if (!useSharedBuffer)
    {
        nw = write(wFd, data, nsend);
    }
    else
    {
        struct msghdr msgh;
        struct iovec iov[1];
        int cmsghdrlength;
        struct cmsghdr *cmsgh;

        int fdCount = sharedBuffers.size();
        if (fdCount > 0)
        {
            if (fdCount > MAXFD_PER_MESSAGE)
            {
                LOG_F(ERROR, "attempt to send too many FD\n");
                close();
                return;
            }

            cmsghdrlength = CMSG_SPACE((fdCount * sizeof(int)));
            // FIXME: abort on alloc error here
            cmsgh = (struct cmsghdr *)malloc(cmsghdrlength);
            memset(cmsgh, 0, cmsghdrlength);

            /* Write the fd as ancillary data */
            cmsgh->cmsg_len = CMSG_LEN(fdCount * sizeof(int));
            cmsgh->cmsg_level = SOL_SOCKET;
            cmsgh->cmsg_type = SCM_RIGHTS;
            msgh.msg_control = cmsgh;
            msgh.msg_controllen = cmsghdrlength;
            for (int i = 0; i < fdCount; ++i)
            {
                ((int *)CMSG_DATA(CMSG_FIRSTHDR(&msgh)))[i] = sharedBuffers[i];
            }
        }
        else
        {
            cmsgh = NULL;
            cmsghdrlength = 0;
            msgh.msg_control = cmsgh;
            msgh.msg_controllen = cmsghdrlength;
        }

        iov[0].iov_base = data;
        iov[0].iov_len = nsend;

        msgh.msg_flags = 0;
        msgh.msg_name = NULL;
        msgh.msg_namelen = 0;
        msgh.msg_iov = iov;
        msgh.msg_iovlen = 1;

        nw = sendmsg(wFd, &msgh, MSG_NOSIGNAL);

        free(cmsgh);
    }

    /* shut down if trouble */
    if (nw <= 0)
    {
        if (nw == 0)
            DLOG_F(INFO, "write returned 0\n");
        else
            LOG_F(ERROR, "write: %s\n", strerror(errno));

        // Keep the read part open
        closeWritePart();
        return;
    }

    /* update amount sent. when complete: free message if we are the last
     * to use it and pop from our queue.
     */
    mp->advance(nsent, nw);
    if (nsent.done())
        consumeHeadMsg();
}
#endif

void MsgQueue::crackBLOB(const char *enableBLOB, BLOBHandling *bp)
{
    if (!strcmp(enableBLOB, "Also"))
        *bp = B_ALSO;
    else if (!strcmp(enableBLOB, "Only"))
        *bp = B_ONLY;
    else if (!strcmp(enableBLOB, "Never"))
        *bp = B_NEVER;
}

void MsgQueue::traceMsg(const std::string &logMsg, XMLEle *root)
{
    DLOG_F(INFO, "{}", logMsg);

    static const char *prtags[] =
        {
            "defNumber",
            "oneNumber",
            "defText",
            "oneText",
            "defSwitch",
            "oneSwitch",
            "defLight",
            "oneLight",
        };
    XMLEle *e;
    const char *msg, *perm, *pcd;
    unsigned int i;

    /* print tag header */
    // fprintf(stderr, "%s %s %s %s", tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"),
    //         findXMLAttValu(root, "state"));
    LOG_F(ERROR, "{} {} {} {}", tagXMLEle(root), findXMLAttValu(root, "device"), findXMLAttValu(root, "name"),
          findXMLAttValu(root, "state"));
    pcd = pcdataXMLEle(root);
    if (pcd[0])
        // fprintf(stderr, " %s", pcd);
        LOG_F(ERROR, "{}", pcd);
    perm = findXMLAttValu(root, "perm");
    if (perm[0])
        // fprintf(stderr, " %s", perm);
        LOG_F(ERROR, "{}", perm);
    msg = findXMLAttValu(root, "message");
    if (msg[0])
        // fprintf(stderr, " '%s'", msg);
        LOG_F(ERROR, "{}", msg);

    /* print each array value */
    for (e = nextXMLEle(root, 1); e; e = nextXMLEle(root, 0))
        for (i = 0; i < sizeof(prtags) / sizeof(prtags[0]); i++)
            if (strcmp(prtags[i], tagXMLEle(e)) == 0)
                // fprintf(stderr, "\n %10s='%s'", findXMLAttValu(e, "name"), pcdataXMLEle(e));
                LOG_F(ERROR, "{:<10}='{}'", findXMLAttValu(e, "name"), pcdataXMLEle(e));
}