#include "io.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <cerrno>
#endif

#include <error.h>

int readFdError(int fd)
{
#ifdef MSG_ERRQUEUE
    char rcvbuf[128]; /* Buffer for normal data (not expected here...) */
    char cbuf[512];   /* Buffer for ancillary data (errors) */
    struct iovec iov;
    struct msghdr msg;

    iov.iov_base = &rcvbuf;
    iov.iov_len = sizeof(rcvbuf);

    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cbuf;
    msg.msg_controllen = sizeof(cbuf);

    int recv_bytes = recvmsg(fd, &msg, MSG_ERRQUEUE | MSG_DONTWAIT);
    if (recv_bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        return errno;
    }

    /* Receive auxiliary data in msgh */
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
    {
        fprintf(stderr, "cmsg_len=%zu, cmsg_level=%u, cmsg_type=%u\n", cmsg->cmsg_len, cmsg->cmsg_level, cmsg->cmsg_type);

        if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR)
        {
            return ((struct sock_extended_err *)CMSG_DATA(cmsg))->ee_errno;
        }
    }
#else
    (void)fd;
#endif

    // Default to EIO as a generic error path
    return EIO;
}

#ifdef _WIN32
void* attachSharedBuffer(HANDLE fileHandle, size_t& size)
{
    SIZE_T fileSize;
    if (!GetFileSizeEx(fileHandle, reinterpret_cast<PLARGE_INTEGER>(&fileSize)))
    {
        perror("invalid shared buffer file handle");
        //Bye();
    }
    size = fileSize;

    HANDLE mappingHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (mappingHandle == nullptr)
    {
        perror("CreateFileMapping");
        //Bye();
    }

    void* ret = MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(mappingHandle);

    if (ret == nullptr)
    {
        perror("MapViewOfFile");
        //Bye();
    }

    return ret;
}

void detachSharedBuffer(void* ptr)
{
    if (!UnmapViewOfFile(ptr))
    {
        perror("shared buffer UnmapViewOfFile");
        //Bye();
    }
}

#else
void *attachSharedBuffer(int fd, size_t &size)
{
    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        perror("invalid shared buffer fd");
        //Bye();
    }
    size = sb.st_size;
    void *ret = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);

    if (ret == MAP_FAILED)
    {
        perror("mmap");
        //Bye();
    }

    return ret;
}

void detachSharedBuffer(int fd, void *ptr, size_t size)
{
    (void)fd;
    if (munmap(ptr, size) == -1)
    {
        perror("shared buffer munmap");
        //Bye();
    }
}
#endif