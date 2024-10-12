#ifndef ATOM_SYSINFO_VIRTUAL_HPP
#define ATOM_SYSINFO_VIRTUAL_HPP

#include <string>

namespace atom::system {

/**
 * @brief Retrieves the vendor information of the hypervisor.
 *
 * This function uses CPUID instruction to get the hypervisor vendor string.
 *
 * @return std::string The vendor string of the hypervisor.
 */
auto getHypervisorVendor() -> std::string;

/**
 * @brief Detects if the system is running inside a virtual machine.
 *
 * This function uses various techniques, including CPUID instruction, to
 * determine if the current system is a virtual machine.
 *
 * @return bool True if the system is a virtual machine, false otherwise.
 */
auto isVirtualMachine() -> bool;

/**
 * @brief Checks BIOS information to identify if the system is a virtual
 * machine.
 *
 * This function inspects the BIOS information for signs that indicate the
 * presence of a virtual machine.
 *
 * @return bool True if the BIOS information suggests a virtual machine, false
 * otherwise.
 */
auto checkBIOS() -> bool;

/**
 * @brief Checks the network adapter for common virtual machine adapters.
 *
 * This function looks for network adapters that are commonly used by virtual
 * machines, such as "VMware Virtual Ethernet Adapter".
 *
 * @return bool True if a virtual machine network adapter is found, false
 * otherwise.
 */
auto checkNetworkAdapter() -> bool;

/**
 * @brief Checks disk information for identifiers commonly used by virtual
 * machines.
 *
 * This function inspects the disk information to find identifiers that are
 * typically associated with virtual machine disks.
 *
 * @return bool True if virtual machine disk identifiers are found, false
 * otherwise.
 */
auto checkDisk() -> bool;

/**
 * @brief Checks the graphics card device for signs of virtualization.
 *
 * This function examines the graphics card device to determine if it is a type
 * commonly used by virtual machines.
 *
 * @return bool True if a virtual machine graphics card is detected, false
 * otherwise.
 */
auto checkGraphicsCard() -> bool;

/**
 * @brief Checks for the presence of common virtual machine processes.
 *
 * This function scans the system processes to identify any that are typically
 * associated with virtual machines.
 *
 * @return bool True if virtual machine processes are found, false otherwise.
 */
auto checkProcesses() -> bool;

/**
 * @brief Checks PCI bus devices for virtualization indicators.
 *
 * This function inspects the PCI bus devices to see if any of them are known
 * to be used by virtual machines.
 *
 * @return bool True if virtual machine PCI bus devices are found, false
 * otherwise.
 */
auto checkPCIBus() -> bool;

/**
 * @brief Detects time drift and offset issues that may indicate a virtual
 * machine.
 *
 * This function checks for irregularities in system time management, which can
 * be a sign of running inside a virtual machine.
 *
 * @return bool True if time drift or offset issues are detected, false
 * otherwise.
 */
auto checkTimeDrift() -> bool;

}  // namespace atom::system

#endif