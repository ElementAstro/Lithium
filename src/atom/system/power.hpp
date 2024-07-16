#ifndef ATOM_SYSTEM_POWER_HPP
#define ATOM_SYSTEM_POWER_HPP

namespace atom::system {
/**
 * @brief Shutdown the system.
 * 关闭系统
 *
 * @return true if the system is successfully shutdown.
 *         如果系统成功关闭，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
auto shutdown() -> bool;

/**
 * @brief Reboot the system.
 * 重启系统
 *
 * @return true if the system is successfully rebooted.
 *         如果系统成功重启，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
auto reboot() -> bool;

/**
 * @brief Hibernate the system.
 * 休眠系统
 *
 * @return true if the system is successfully hibernated.
 *         如果系统成功休眠，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
auto hibernate() -> bool;

/**
 * @brief Logout the current user.
 * 注销当前用户
 *
 * @return true if the user is successfully logged out.
 *         如果用户成功注销，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
auto logout() -> bool;

/**
 * @brief Lock the screen.
 * 锁定屏幕
 *
 * @return true if the screen is successfully locked.
 *         如果屏幕成功锁定，则返回 true
 * @return false if an error occurred.
 *         如果发生错误，则返回 false
 */
auto lockScreen() -> bool;
}  // namespace atom::system

#endif