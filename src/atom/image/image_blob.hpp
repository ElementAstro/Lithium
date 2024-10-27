#ifndef ATOM_IMAGE_BLOB_HPP
#define ATOM_IMAGE_BLOB_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <span>
#include <vector>

#include "atom/error/exception.hpp"

#if __has_include(<opencv2/core.hpp>)
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

namespace atom::image {

template <class T>
concept BlobType =
    std::same_as<T, std::byte> || std::same_as<T, const std::byte>;

template <class T>
concept BlobValueType = std::is_trivially_copyable_v<T>;

enum class BlobMode { NORMAL, FAST };

constexpr int DEFAULT_DEPTH = 8;

template <BlobType T, BlobMode Mode = BlobMode::NORMAL>
class Blob {
private:
    std::conditional_t<Mode == BlobMode::FAST, std::span<T>, std::vector<T>>
        storage_;
    int rows_ = 0;
    int cols_ = 0;
    int channels_ = 1;
#if __has_include(<opencv2/core.hpp>)
    int depth_ = CV_8U;
#else
    int depth_ = DEFAULT_DEPTH;
#endif

public:
    Blob() noexcept = default;
    Blob(const Blob&) = default;
    Blob(Blob&&) noexcept = default;
    auto operator=(const Blob&) -> Blob& = default;
    auto operator=(Blob&&) noexcept -> Blob& = default;
    ~Blob() = default;

    template <class U>
        requires(std::is_const_v<T> && std::same_as<const U, T>)
    explicit Blob(const Blob<U, Mode>& that) noexcept
        : storage_(that.storage_),
          rows_(that.rows_),
          cols_(that.cols_),
          channels_(that.channels_),
          depth_(that.depth_) {}

    template <class U>
        requires(std::is_const_v<T> && std::same_as<const U, T>)
    explicit Blob(Blob<U, Mode>&& that) noexcept
        : storage_(std::move(that.storage_)),
          rows_(that.rows_),
          cols_(that.cols_),
          channels_(that.channels_),
          depth_(that.depth_) {}

    Blob(void* ptr, size_t n) noexcept
        requires(Mode == BlobMode::FAST)
        : storage_(reinterpret_cast<T*>(ptr), n) {}

    template <BlobValueType U>
    explicit Blob(U& var) noexcept
        requires(Mode == BlobMode::FAST)
        : storage_(reinterpret_cast<T*>(&var), sizeof(U)) {}

    template <BlobValueType U>
    Blob(U* ptr, size_t n)
        : storage_(Mode == BlobMode::FAST
                       ? std::span<T>(reinterpret_cast<T*>(ptr), n * sizeof(U))
                       : std::vector<T>(
                             reinterpret_cast<T*>(ptr),
                             reinterpret_cast<T*>(ptr) + n * sizeof(U))) {}

    template <BlobValueType U, size_t N>
    explicit Blob(std::array<U, N>& arr)
        : storage_(Mode == BlobMode::FAST
                       ? std::span<T>(reinterpret_cast<T*>(arr.data()),
                                      sizeof(U) * N)
                       : std::vector<T>(reinterpret_cast<T*>(arr.data()),
                                        reinterpret_cast<T*>(arr.data()) +
                                            sizeof(U) * N)) {}

#if __has_include(<opencv2/core.hpp>)
    explicit Blob(const cv::Mat& mat)
        : rows_(mat.rows),
          cols_(mat.cols),
          channels_(mat.channels()),
          depth_(mat.depth()) {
        if (mat.isContinuous()) {
            if constexpr (Mode == BlobMode::FAST) {
                storage_ = std::span<T>(reinterpret_cast<T*>(mat.data),
                                        mat.total() * mat.elemSize());
            } else {
                storage_.assign(reinterpret_cast<T*>(mat.data),
                                reinterpret_cast<T*>(mat.data) +
                                    mat.total() * mat.elemSize());
            }
        } else {
            if constexpr (Mode == BlobMode::FAST) {
                THROW_RUNTIME_ERROR(
                    "Cannot create fast blob from non-continuous cv::Mat");
            } else {
                storage_.reserve(mat.total() * mat.elemSize());
                for (int i = 0; i < mat.rows; ++i) {
                    storage_.insert(storage_.end(), mat.ptr<T>(i),
                                    mat.ptr<T>(i) + mat.cols * mat.elemSize());
                }
            }
        }
    }
#endif

    auto operator[](size_t idx) -> T& { return storage_[idx]; }
    auto operator[](size_t idx) const -> const T& { return storage_[idx]; }

    auto begin() { return storage_.begin(); }
    auto end() { return storage_.end(); }
    auto begin() const { return storage_.begin(); }
    auto end() const { return storage_.end(); }

    [[nodiscard]] auto size() const -> size_t { return storage_.size(); }

    auto slice(size_t offset, size_t length) const -> Blob {
        if (offset + length > size()) {
            THROW_OUT_OF_RANGE("Slice range out of bounds");
        }
        Blob result;
        result.rows_ = 1;
        result.cols_ = length;
        result.channels_ = channels_;
        result.depth_ = depth_;
        if constexpr (Mode == BlobMode::FAST) {
            result.storage_ = std::span<T>(storage_.data() + offset, length);
        } else {
            result.storage_.assign(storage_.begin() + offset,
                                   storage_.begin() + offset + length);
        }
        return result;
    }

    auto operator==(const Blob& other) const -> bool {
        return std::ranges::equal(storage_, other.storage_) &&
               rows_ == other.rows_ && cols_ == other.cols_ &&
               channels_ == other.channels_ && depth_ == other.depth_;
    }

    void fill(T value) { std::ranges::fill(storage_, value); }

    void append(const Blob& other) {
        if constexpr (Mode == BlobMode::FAST) {
            THROW_RUNTIME_ERROR("Cannot append in Fast mode");
        } else {
            storage_.insert(storage_.end(), other.storage_.begin(),
                            other.storage_.end());
            rows_ += other.rows_;
        }
    }

    void append(const void* ptr, size_t n) {
        if constexpr (Mode == BlobMode::FAST) {
            THROW_RUNTIME_ERROR("Cannot append in Fast mode");
        } else {
            const auto* bytePtr = reinterpret_cast<const T*>(ptr);
            storage_.insert(storage_.end(), bytePtr, bytePtr + n);
            rows_ += static_cast<size_t>(n) / (static_cast<size_t>(cols_) *
                                               static_cast<size_t>(channels_));
        }
    }

    void allocate(size_t size) {
        if constexpr (Mode == BlobMode::FAST) {
            THROW_RUNTIME_ERROR("Cannot allocate in Fast mode");
        } else {
            storage_.resize(size);
        }
    }

    void deallocate() {
        if constexpr (Mode == BlobMode::FAST) {
            THROW_RUNTIME_ERROR("Cannot deallocate in Fast mode");
        } else {
            storage_.clear();
            storage_.shrink_to_fit();
        }
    }

    void xorWith(const Blob& other) {
        if (size() != other.size()) {
            THROW_RUNTIME_ERROR(
                "Blobs must be of the same size for XOR operation");
        }
        std::transform(begin(), end(), other.begin(), begin(), std::bit_xor());
    }

    [[nodiscard]] auto compress() const -> Blob {
        Blob compressed;
        for (size_t i = 0; i < size(); ++i) {
            T current = storage_[i];
            size_t count = 1;
#pragma unroll
            while (i + 1 < size() && storage_[i + 1] == current) {
                ++count;
                ++i;
            }
            compressed.append(&current, sizeof(T));
            compressed.append(&count, sizeof(size_t));
        }
        return compressed;
    }

    [[nodiscard]] auto decompress() const -> Blob {
        Blob decompressed;
        for (size_t i = 0; i < size(); i += sizeof(T) + sizeof(size_t)) {
            T value;
            size_t count;
            std::memcpy(&value, &storage_[i], sizeof(T));
            std::memcpy(&count, &storage_[i + sizeof(T)], sizeof(size_t));
#pragma unroll
            for (size_t j = 0; j < count; ++j) {
                decompressed.append(&value, sizeof(T));
            }
        }
        return decompressed;
    }

    [[nodiscard]] auto serialize() const -> std::vector<std::byte> {
        std::vector<std::byte> serialized;
        size_t size = storage_.size();
        serialized.insert(
            serialized.end(), reinterpret_cast<const std::byte*>(&size),
            reinterpret_cast<const std::byte*>(&size) + sizeof(size_t));
        serialized.insert(serialized.end(),
                          reinterpret_cast<const std::byte*>(storage_.data()),
                          reinterpret_cast<const std::byte*>(storage_.data()) +
                              storage_.size());
        return serialized;
    }

    static auto deserialize(const std::vector<std::byte>& data) -> Blob {
        if (data.size() < sizeof(size_t)) {
            THROW_RUNTIME_ERROR("Invalid serialized data");
        }
        size_t size;
        std::memcpy(&size, data.data(), sizeof(size_t));
        if (data.size() != sizeof(size_t) + size) {
            THROW_RUNTIME_ERROR("Invalid serialized data size");
        }
        return Blob(reinterpret_cast<const T*>(data.data() + sizeof(size_t)),
                    size);
    }

#if __has_include(<opencv2/core.hpp>)
    cv::Mat to_mat() const {
        int type = CV_MAKETYPE(depth_, channels_);
        cv::Mat mat(rows_, cols_, type);
        std::memcpy(mat.data, storage_.data(), size());
        return mat;
    }

    void apply_filter(const cv::Mat& kernel) {
        cv::Mat src = to_mat();
        cv::Mat dst;
        cv::filter2D(src, dst, -1, kernel);
        *this = Blob(dst);
    }

    void resize(int new_rows, int new_cols) {
        cv::Mat src = to_mat();
        cv::Mat dst;
        cv::resize(src, dst, cv::Size(new_cols, new_rows));
        *this = Blob(dst);
    }

    void convert_color(int code) {
        cv::Mat src = to_mat();
        cv::Mat dst;
        cv::cvtColor(src, dst, code);
        *this = Blob(dst);
    }

    void rotate(double angle) {
        cv::Mat src = to_mat();
        cv::Mat dst;
        cv::Point2f center(src.cols / 2.0, src.rows / 2.0);
        cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::warpAffine(src, dst, rot, src.size());
        *this = Blob(dst);
    }

    void flip(int flipCode) {
        cv::Mat src = to_mat();
        cv::Mat dst;
        cv::flip(src, dst, flipCode);
        *this = Blob(dst);
    }

    void save(const std::string& filename) const {
        cv::Mat mat = to_mat();
        cv::imwrite(filename, mat);
    }

    static Blob load(const std::string& filename) {
        cv::Mat mat = cv::imread(filename, cv::IMREAD_UNCHANGED);
        if (mat.empty()) {
            THROW_RUNTIME_ERROR("Failed to load image from file");
        }
        return Blob(mat);
    }

    std::vector<Blob> split_channels() const {
        cv::Mat src = to_mat();
        std::vector<cv::Mat> channels;
        cv::split(src, channels);
        std::vector<Blob> channel_blobs;
        for (const auto& channel : channels) {
            channel_blobs.emplace_back(channel);
        }
        return channel_blobs;
    }

    static Blob merge_channels(const std::vector<Blob>& channel_blobs) {
        std::vector<cv::Mat> channels;
        for (const auto& blob : channel_blobs) {
            channels.push_back(blob.to_mat());
        }
        cv::Mat merged;
        cv::merge(channels, merged);
        return Blob(merged);
    }
#endif

    [[nodiscard]] auto getRows() const -> int { return rows_; }
    [[nodiscard]] auto getCols() const -> int { return cols_; }
    [[nodiscard]] auto getChannels() const -> int { return channels_; }
    [[nodiscard]] auto getDepth() const -> int { return depth_; }
};

using cblob = Blob<const std::byte>;
using blob = Blob<std::byte>;

using fast_cblob = Blob<const std::byte, BlobMode::FAST>;
using fast_blob = Blob<std::byte, BlobMode::FAST>;

}  // namespace atom::image

#endif  // ATOM_IMAGE_BLOB_HPP
