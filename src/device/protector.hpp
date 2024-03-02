/*
 * protector.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: License Protector, Class for monitoring changes to license files or
directories and taking action upon changes.

**************************************************/

#include <memory>  // For std::unique_ptr
#include <string>


namespace Lithium {
class LicenseProtectorImpl;  // 前向声明实现类 Forward declaration of the
                             // implementation class

/**
 * @brief 用于监控证书文件或目录的变化，并在变化时采取相应措施的类。
 *        Class for monitoring changes to license files or directories and
 * taking action upon changes.
 *
 * @details
 * 这个类使用Pimpl模式隐藏具体的实现细节，降低了库使用者与库内部实现之间的耦合。
 *          This class uses the Pimpl idiom to hide the implementation details,
 * reducing coupling between library users and the internal implementation.
 *
 * @note
 * 这个类主要是为了安全性，参考ZWO的证书被破解，只要有对于证书的操作就会采取安全措施，虽然这是开源软件
 *       但是为了安全，还是建议使用这个类。
 *       This class is mainly for security, refer to the certificate cracked by
 * ZWO, as long as there is any operation on the certificate, it will take
 * security measures, although this is an open source software.
 */
class LicenseProtector {
public:
    /**
     * @brief 构造函数。Constructor.
     *
     * @param path 要监控的文件或目录的路径。Path to the file or directory to
     * monitor.
     */
    explicit LicenseProtector(const std::string &path);

    /**
     * @brief 析构函数。Responsible for cleaning up resources. Destructor.
     */
    ~LicenseProtector();

    // 禁止拷贝构造和赋值操作。Disable copy construction and assignment.
    LicenseProtector(const LicenseProtector &) = delete;
    LicenseProtector &operator=(const LicenseProtector &) = delete;

    /**
     * @brief 开始监控文件或目录的变化。Start monitoring changes to the file or
     * directory.
     */
    void startMonitoring();

    /**
     * @brief 停止监控文件或目录的变化。Stop monitoring changes to the file or
     * directory.
     */
    void stopMonitoring();

private:
    std::unique_ptr<LicenseProtectorImpl>
        pImpl;  ///< 指向具体实现的指针。Pointer to the implementation.
};

}  // namespace Lithium
