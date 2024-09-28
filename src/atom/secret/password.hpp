#ifndef PASSWORD_H
#define PASSWORD_H

#include <string>

class AESCipher {
public:
    static std::string encrypt(const std::string& plaintext,
                               const unsigned char* key);
    static std::string decrypt(const std::string& ciphertext,
                               const unsigned char* key);
};

class PasswordManager {
private:
    unsigned char key[16];

public:
    PasswordManager();
    void storePassword(const std::string& platformKey,
                       const std::string& password);
    std::string retrievePassword(const std::string& platformKey);
    void deletePassword(const std::string& platformKey);

private:
#if defined(_WIN32)
    void storeToWindowsCredentialManager(const std::string& target,
                                         const std::string& encryptedPassword);
    std::string retrieveFromWindowsCredentialManager(const std::string& target);
    void deleteFromWindowsCredentialManager(const std::string& target);
#elif defined(__APPLE__)
    void storeToMacKeychain(const std::string& service,
                            const std::string& account,
                            const std::string& encryptedPassword);
    std::string retrieveFromMacKeychain(const std::string& service,
                                        const std::string& account);
    void deleteFromMacKeychain(const std::string& service,
                               const std::string& account);
#elif defined(__linux__)
    void storeToLinuxKeyring(const std::string& schema_name,
                             const std::string& attribute_name,
                             const std::string& encryptedPassword);
    std::string retrieveFromLinuxKeyring(const std::string& schema_name,
                                         const std::string& attribute_name);
    void deleteFromLinuxKeyring(const std::string& schema_name,
                                const std::string& attribute_name);
#endif
};

#endif  // PASSWORD_H
