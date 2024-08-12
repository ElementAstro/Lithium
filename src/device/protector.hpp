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

#include <functional>
#include <memory>  // For std::unique_ptr
#include <string>

namespace lithium {
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
     * @param path 要监控的文件或目录的路径。Path to the file or directory to be
     * monitored.
     */
    explicit LicenseProtector(const std::string &path);

    /**
     * @brief 析构函数。Destructor.
     */
    ~LicenseProtector();

    /**
     * @brief 开始监控文件或目录的变化。Starts monitoring changes to the file or
     * directory.
     */
    void startMonitoring();

    /**
     * @brief 停止监控文件或目录的变化。Stops monitoring changes to the file or
     * directory.
     */
    void stopMonitoring();

    /**
     * @brief 设置删除事件处理函数。Sets the handler function for delete events.
     */
    void setDeleteHandler(std::function<void()> handler);

    /**
     * @brief 设置修改事件处理函数。Sets the handler function for modify events.
     */
    void setModifyHandler(std::function<void()> handler);

private:
    std::unique_ptr<LicenseProtectorImpl>
        pImpl_;  ///< 使用Pimpl模式的具体实现。Concrete implementation using
                 ///< Pimpl idiom.
};

}  // namespace lithium
