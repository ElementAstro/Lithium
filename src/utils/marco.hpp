#define CHECK_WEAK_PTR_EXPIRED(weakPtr, errorMsg) \
    do { \
        if (weakPtr.expired()) { \
            LOG_F(ERROR, "Failed to {}", errorMsg); \
            throw std::runtime_error("Failed to " errorMsg); \
        } \
    } while(0)
    