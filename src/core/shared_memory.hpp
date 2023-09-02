#include <type_traits>
#include <cstring>

#include <loguru/loguru.hpp>

#if defined(_WIN32) || defined(_WIN64) // Windows
#include <windows.h>
#else // Unix-like
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class SharedMemory
{
public:
    template <typename T>
    SharedMemory(const std::string &name)
        : name_(name), buffer_(nullptr), mutex_{}
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be a trivially copyable type.");
        static_assert(std::is_standard_layout<T>::value, "T must be a standard layout type.");

#if defined(_WIN32) || defined(_WIN64) // Windows
        handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(T) + sizeof(bool), name.c_str());
        if (handle_ == nullptr)
        {
            LOG_F(ERROR, "Failed to create file mapping.");
            throw std::runtime_error("Failed to create file mapping.");
        }

        buffer_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(T) + sizeof(bool));
        if (buffer_ == nullptr)
        {
            CloseHandle(handle_);
            LOG_F(ERROR, "Failed to map view of file.");
            throw std::runtime_error("Failed to map view of file.");
        }
#else // Unix-like
        fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd_ == -1)
        {
            LOG_F(ERROR, "Failed to create shared memory.");
            throw std::runtime_error("Failed to create shared memory.");
        }

        ftruncate(fd_, sizeof(T) + sizeof(bool));
        buffer_ = mmap(nullptr, sizeof(T) + sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        close(fd_);

        if (buffer_ == MAP_FAILED)
        {
            shm_unlink(name.c_str());
            LOG_F(ERROR, "Failed to map shared memory.");
            throw std::runtime_error("Failed to map shared memory.");
        }
#endif
        mutex_ = reinterpret_cast<std::atomic_flag *>(buffer_);
        new (mutex_) std::atomic_flag();
    }

    ~SharedMemory()
    {
#if defined(_WIN32) || defined(_WIN64) // Windows
        UnmapViewOfFile(buffer_);
        CloseHandle(handle_);
#else // Unix-like
        munmap(buffer_, sizeof(T) + sizeof(bool));
        shm_unlink(name_.c_str());
#endif
    }

    template <typename T>
    void Write(const T &data)
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be a trivially copyable type.");
        static_assert(std::is_standard_layout<T>::value, "T must be a standard layout type.");

        std::unique_lock<std::mutex> lock(mutex_);

        std::memcpy(static_cast<char *>(buffer_) + sizeof(bool), &data, sizeof(T));

        LOG_F(INFO, "Data written to shared memory.");
        *reinterpret_cast<bool *>(buffer_) = true;
    }

    template <typename T>
    T Read() const
    {
        static_assert(std::is_trivially_copyable<T>::value, "T must be a trivially copyable type.");
        static_assert(std::is_standard_layout<T>::value, "T must be a standard layout type.");

        std::unique_lock<std::mutex> lock(mutex_);

        T data;
        if (*reinterpret_cast<bool *>(buffer_))
        {
            std::memcpy(&data, static_cast<const char *>(buffer_) + sizeof(bool), sizeof(T));
            *reinterpret_cast<bool *>(buffer_) = false;

            LOG_F(INFO, "Data read from shared memory.");
        }
        return data;
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        *reinterpret_cast<bool *>(buffer_) = false;

        LOG_F(INFO, "Shared memory cleared.");
    }

private:
    std::string name_;
#if defined(_WIN32) || defined(_WIN64) // Windows
    HANDLE handle_;
#else // Unix-like
    int fd_;
#endif
    void *buffer_;
    std::atomic_flag *mutex_;
    mutable std::mutex mutex_;
};