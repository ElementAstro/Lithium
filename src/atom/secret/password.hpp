#ifndef ATOM_SECRET_PASSWORD_HPP
#define ATOM_SECRET_PASSWORD_HPP

#include <string>

namespace atom::secret {
/**
 * @brief Class for managing passwords securely.
 *
 * The PasswordManager class provides methods to store, retrieve, and delete
 * passwords securely using platform-specific credential storage mechanisms.
 */
class PasswordManager {
private:
    unsigned char key[16];  ///< The encryption key used for AES encryption.

public:
    /**
     * @brief Constructs a PasswordManager object.
     */
    PasswordManager();

    /**
     * @brief Stores a password for a specific platform key.
     * @param platformKey The key associated with the platform.
     * @param password The password to store.
     */
    void storePassword(const std::string& platformKey,
                       const std::string& password);

    /**
     * @brief Retrieves a password for a specific platform key.
     * @param platformKey The key associated with the platform.
     * @return The retrieved password.
     */
    std::string retrievePassword(const std::string& platformKey);

    /**
     * @brief Deletes a password for a specific platform key.
     * @param platformKey The key associated with the platform.
     */
    void deletePassword(const std::string& platformKey);

private:
#if defined(_WIN32)
    /**
     * @brief Stores an encrypted password in the Windows Credential Manager.
     * @param target The target name for the credential.
     * @param encryptedPassword The encrypted password to store.
     */
    void storeToWindowsCredentialManager(const std::string& target,
                                         const std::string& encryptedPassword);

    /**
     * @brief Retrieves an encrypted password from the Windows Credential
     * Manager.
     * @param target The target name for the credential.
     * @return The retrieved encrypted password.
     */
    std::string retrieveFromWindowsCredentialManager(const std::string& target);

    /**
     * @brief Deletes a password from the Windows Credential Manager.
     * @param target The target name for the credential.
     */
    void deleteFromWindowsCredentialManager(const std::string& target);

#elif defined(__APPLE__)
    /**
     * @brief Stores an encrypted password in the macOS Keychain.
     * @param service The service name for the keychain item.
     * @param account The account name for the keychain item.
     * @param encryptedPassword The encrypted password to store.
     */
    void storeToMacKeychain(const std::string& service,
                            const std::string& account,
                            const std::string& encryptedPassword);

    /**
     * @brief Retrieves an encrypted password from the macOS Keychain.
     * @param service The service name for the keychain item.
     * @param account The account name for the keychain item.
     * @return The retrieved encrypted password.
     */
    std::string retrieveFromMacKeychain(const std::string& service,
                                        const std::string& account);

    /**
     * @brief Deletes a password from the macOS Keychain.
     * @param service The service name for the keychain item.
     * @param account The account name for the keychain item.
     */
    void deleteFromMacKeychain(const std::string& service,
                               const std::string& account);

#elif defined(__linux__)
    /**
     * @brief Stores an encrypted password in the Linux Keyring.
     * @param schema_name The schema name for the keyring item.
     * @param attribute_name The attribute name for the keyring item.
     * @param encryptedPassword The encrypted password to store.
     */
    void storeToLinuxKeyring(const std::string& schema_name,
                             const std::string& attribute_name,
                             const std::string& encryptedPassword);

    /**
     * @brief Retrieves an encrypted password from the Linux Keyring.
     * @param schema_name The schema name for the keyring item.
     * @param attribute_name The attribute name for the keyring item.
     * @return The retrieved encrypted password.
     */
    std::string retrieveFromLinuxKeyring(const std::string& schema_name,
                                         const std::string& attribute_name);

    /**
     * @brief Deletes a password from the Linux Keyring.
     * @param schema_name The schema name for the keyring item.
     * @param attribute_name The attribute name for the keyring item.
     */
    void deleteFromLinuxKeyring(const std::string& schema_name,
                                const std::string& attribute_name);
#endif
};
}  // namespace atom::secret

#endif  // ATOM_SECRET_PASSWORD_HPP
