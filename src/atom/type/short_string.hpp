#include <algorithm>
#include <string>

class ShortString {
private:
    static constexpr size_t MAX_LENGTH =
        15;           ///< Maximum length of the short string
    std::string str;  ///< Actual string stored

public:
    /**
     * @brief Default constructor for ShortString.
     */
    ShortString() = default;

    /**
     * @brief Constructor taking a std::string.
     * @param s The input std::string.
     */
    explicit ShortString(const std::string& s);

    /**
     * @brief Constructor taking a std::string_view.
     * @param s The input std::string_view.
     */
    explicit ShortString(std::string_view s);

    /**
     * @brief Constructor taking a C-style string.
     * @param s The input C-style string.
     */
    ShortString(const char* s);

    /**
     * @brief Copy constructor.
     * @param other The ShortString to copy from.
     */
    ShortString(const ShortString& other);

    /**
     * @brief Copy assignment operator.
     * @param other The ShortString to assign from.
     * @return Reference to the modified ShortString.
     */
    ShortString& operator=(const ShortString& other);

    ShortString& operator=(const std::string& s);

    ShortString& operator=(const char* s);

    ShortString& operator=(std::string_view s);

    /**
     * @brief Overloaded stream insertion operator for ShortString.
     * @param os The output stream to write to.
     * @param ss The ShortString to write.
     * @return Reference to the output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const ShortString& ss);

    /**
     * @brief Overloaded addition operator for ShortString.
     * @param other The ShortString to add.
     * @return The result of concatenating two ShortStrings.
     */
    ShortString operator+(const ShortString& other) const;

    ShortString& operator+=(const ShortString& other);

    ShortString& operator+=(std::string_view other);

    [[nodiscard]] bool operator==(const ShortString& other) const noexcept;

    [[nodiscard]] bool operator!=(const ShortString& other) const noexcept;

    [[nodiscard]] bool operator<(const ShortString& other) const noexcept;

    [[nodiscard]] bool operator>(const ShortString& other) const noexcept;

    [[nodiscard]] bool operator<=(const ShortString& other) const noexcept;

    [[nodiscard]] bool operator>=(const ShortString& other) const noexcept;

    [[nodiscard]] char& operator[](size_t index) noexcept;

    [[nodiscard]] const char& operator[](size_t index) const noexcept;

    [[nodiscard]] size_t length() const noexcept;

    ShortString substr(size_t pos = 0, size_t count = std::string::npos) const;

    void clear() noexcept;

    void swap(ShortString& other) noexcept;
};
