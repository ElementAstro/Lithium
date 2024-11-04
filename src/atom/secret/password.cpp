#include "password.hpp"

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <vector>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <wincred.h>
// clang-format on
#elif defined(__APPLE__)
#include <Security/Security.h>
#elif defined(__linux__)
#if __has_include(<libsecret-1/secrets.h>)
#include <libsecret-1/secrets.h>
#elif __has_include(<libsecret/secret.h>)
#include <libsecret/secret.h>
#elif __has_include(<secrets.h>)
#include <secrets.h>
#endif
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/aes.hpp"

namespace atom::secret {
PasswordManager::PasswordManager() {
    if (RAND_bytes(key, AES_BLOCK_SIZE) != 1) {
        LOG_F(ERROR, "Failed to generate random key for AES encryption.");
        THROW_RUNTIME_ERROR(
            "Failed to generate random key for AES encryption.");
    }
    LOG_F(INFO, "PasswordManager initialized with a random AES key.");
}

void PasswordManager::storePassword(const std::string& platformKey,
                                    const std::string& password) {
    LOG_F(INFO, "Storing password for platform key: {}", platformKey);
    std::vector<unsigned char> iv;
    std::vector<unsigned char> tag;
    std::string encryptedPassword =
        utils::encryptAES(password, platformKey, iv, tag);
    LOG_F(INFO, "Password encrypted successfully.");

#if defined(_WIN32)
    storeToWindowsCredentialManager(platformKey, encryptedPassword);
#elif defined(__APPLE__)
    storeToMacKeychain(platformKey, "PasswordManager", encryptedPassword);
#elif defined(__linux__)
    storeToLinuxKeyring("PasswordManager", platformKey, encryptedPassword);
#endif
}

std::string PasswordManager::retrievePassword(const std::string& platformKey) {
    LOG_F(INFO, "Retrieving password for platform key: {}", platformKey);
    std::string encryptedPassword;

#if defined(_WIN32)
    encryptedPassword = retrieveFromWindowsCredentialManager(platformKey);
#elif defined(__APPLE__)
    encryptedPassword = retrieveFromMacKeychain(platformKey, "PasswordManager");
#elif defined(__linux__)
    encryptedPassword =
        retrieveFromLinuxKeyring("PasswordManager", platformKey);
#endif

    if (encryptedPassword.empty()) {
        LOG_F(ERROR,
              "Failed to retrieve encrypted password for platform key: {}",
              platformKey);
        return "";
    }

    std::vector<unsigned char> iv;
    std::vector<unsigned char> tag;
    std::string decryptedPassword =
        utils::decryptAES(encryptedPassword, platformKey, iv, tag);
    LOG_F(INFO, "Password decrypted successfully.");
    return decryptedPassword;
}

void PasswordManager::deletePassword(const std::string& platformKey) {
    LOG_F(INFO, "Deleting password for platform key: {}", platformKey);
#if defined(_WIN32)
    deleteFromWindowsCredentialManager(platformKey);
#elif defined(__APPLE__)
    deleteFromMacKeychain(platformKey, "PasswordManager");
#elif defined(__linux__)
    deleteFromLinuxKeyring("PasswordManager", platformKey);
#endif
}

#if defined(_WIN32)
void PasswordManager::storeToWindowsCredentialManager(
    const std::string& target, const std::string& encryptedPassword) {
    CREDENTIALW cred = {};
    cred.Type = CRED_TYPE_GENERIC;
    std::wstring wideTarget(target.begin(), target.end());
    cred.TargetName = const_cast<LPWSTR>(wideTarget.c_str());
    cred.CredentialBlobSize = encryptedPassword.length();
    cred.CredentialBlob =
        reinterpret_cast<LPBYTE>(const_cast<char*>(encryptedPassword.c_str()));
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;

    if (CredWriteW(&cred, 0)) {
        LOG_F(INFO,
              "Password stored successfully in Windows Credential Manager for "
              "target: {}",
              target);
    } else {
        LOG_F(ERROR,
              "Failed to store password in Windows Credential Manager for "
              "target: {}. Error: {}",
              target, GetLastError());
    }
}

std::string PasswordManager::retrieveFromWindowsCredentialManager(
    const std::string& target) {
    PCREDENTIALW cred;
    std::wstring wideTarget(target.begin(), target.end());
    if (CredReadW(wideTarget.c_str(), CRED_TYPE_GENERIC, 0, &cred)) {
        std::string encryptedPassword(
            reinterpret_cast<char*>(cred->CredentialBlob),
            cred->CredentialBlobSize);
        CredFree(cred);
        LOG_F(INFO,
              "Password retrieved successfully from Windows Credential Manager "
              "for target: {}",
              target);
        return encryptedPassword;
    } else {
        LOG_F(ERROR,
              "Failed to retrieve password from Windows Credential Manager for "
              "target: {}. Error: {}",
              target, GetLastError());
        return "";
    }
}

void PasswordManager::deleteFromWindowsCredentialManager(
    const std::string& target) {
    std::wstring wideTarget(target.begin(), target.end());
    if (CredDeleteW(wideTarget.c_str(), CRED_TYPE_GENERIC, 0)) {
        LOG_F(INFO,
              "Password deleted successfully from Windows Credential Manager "
              "for target: {}",
              target);
    } else {
        LOG_F(ERROR,
              "Failed to delete password from Windows Credential Manager for "
              "target: {}. Error: {}",
              target, GetLastError());
    }
}

#elif defined(__APPLE__)
void PasswordManager::storeToMacKeychain(const std::string& service,
                                         const std::string& account,
                                         const std::string& encryptedPassword) {
    OSStatus status = SecKeychainAddGenericPassword(
        nullptr, service.length(), service.c_str(), account.length(),
        account.c_str(), encryptedPassword.length(), encryptedPassword.c_str(),
        nullptr);

    if (status == errSecSuccess) {
        LOG_F(INFO,
              "Password stored successfully in macOS Keychain for service: {}, "
              "account: {}",
              service, account);
    } else {
        LOG_F(ERROR,
              "Failed to store password in macOS Keychain for service: {}, "
              "account: {}. Error: {}",
              service, account, status);
    }
}

std::string PasswordManager::retrieveFromMacKeychain(
    const std::string& service, const std::string& account) {
    void* passwordData;
    UInt32 passwordLength;
    OSStatus status = SecKeychainFindGenericPassword(
        nullptr, service.length(), service.c_str(), account.length(),
        account.c_str(), &passwordLength, &passwordData, nullptr);

    if (status == errSecSuccess) {
        std::string encryptedPassword(static_cast<char*>(passwordData),
                                      passwordLength);
        SecKeychainItemFreeContent(nullptr, passwordData);
        LOG_F(INFO,
              "Password retrieved successfully from macOS Keychain for "
              "service: {}, account: {}",
              service, account);
        return encryptedPassword;
    } else {
        LOG_F(ERROR,
              "Failed to retrieve password from macOS Keychain for service: "
              "{}, account: {}. Error: {}",
              service, account, status);
        return "";
    }
}

void PasswordManager::deleteFromMacKeychain(const std::string& service,
                                            const std::string& account) {
    SecKeychainItemRef itemRef;
    OSStatus status = SecKeychainFindGenericPassword(
        nullptr, service.length(), service.c_str(), account.length(),
        account.c_str(), nullptr, nullptr, &itemRef);

    if (status == errSecSuccess) {
        status = SecKeychainItemDelete(itemRef);
        if (status == errSecSuccess) {
            LOG_F(INFO,
                  "Password deleted successfully from macOS Keychain for "
                  "service: {}, account: {}",
                  service, account);
        } else {
            LOG_F(ERROR,
                  "Failed to delete password from macOS Keychain for service: "
                  "{}, account: {}. Error: {}",
                  service, account, status);
        }
        CFRelease(itemRef);
    } else {
        LOG_F(ERROR,
              "Failed to find password for deletion in macOS Keychain for "
              "service: {}, account: {}. Error: {}",
              service, account, status);
    }
}

#elif defined(__linux__) && (defined(__has_include) && __has_include(<libsecret-1/secrets.h>))
void PasswordManager::storeToLinuxKeyring(
    const std::string& schema_name, const std::string& attribute_name,
    const std::string& encryptedPassword) {
    SecretSchema schema = {
        schema_name.c_str(),
        SECRET_SCHEMA_NONE,
        {{attribute_name.c_str(), SECRET_SCHEMA_ATTRIBUTE_STRING},
         {nullptr, 0}}};

    GError* error = nullptr;
    secret_password_store_sync(
        &schema, nullptr, "PasswordManager", encryptedPassword.c_str(), nullptr,
        &error, attribute_name.c_str(), "encrypted_password", nullptr);

    if (error != nullptr) {
        LOG_F(ERROR,
              "Failed to store password in Linux keyring for schema: {}, "
              "attribute: {}. Error: {}",
              schema_name, attribute_name, error->message);
        g_error_free(error);
    } else {
        LOG_F(INFO,
              "Password stored successfully in Linux keyring for schema: {}, "
              "attribute: {}",
              schema_name, attribute_name);
    }
}

std::string PasswordManager::retrieveFromLinuxKeyring(
    const std::string& schema_name, const std::string& attribute_name) {
    SecretSchema schema = {
        schema_name.c_str(),
        SECRET_SCHEMA_NONE,
        {{attribute_name.c_str(), SECRET_SCHEMA_ATTRIBUTE_STRING},
         {nullptr, 0}}};

    GError* error = nullptr;
    char* password = secret_password_lookup_sync(&schema, nullptr, &error,
                                                 attribute_name.c_str(),
                                                 "encrypted_password", nullptr);

    if (error != nullptr) {
        LOG_F(ERROR,
              "Failed to retrieve password from Linux keyring for schema: {}, "
              "attribute: {}. Error: {}",
              schema_name, attribute_name, error->message);
        g_error_free(error);
        return "";
    } else if (password == nullptr) {
        LOG_F(
            ERROR,
            "Password not found in Linux keyring for schema: {}, attribute: {}",
            schema_name, attribute_name);
        return "";
    } else {
        std::string encryptedPassword(password);
        secret_password_free(password);
        LOG_F(INFO,
              "Password retrieved successfully from Linux keyring for schema: "
              "{}, attribute: {}",
              schema_name, attribute_name);
        return encryptedPassword;
    }
}

void PasswordManager::deleteFromLinuxKeyring(
    const std::string& schema_name, const std::string& attribute_name) {
    SecretSchema schema = {
        schema_name.c_str(),
        SECRET_SCHEMA_NONE,
        {{attribute_name.c_str(), SECRET_SCHEMA_ATTRIBUTE_STRING},
         {nullptr, 0}}};

    GError* error = nullptr;
    gboolean result = secret_password_clear_sync(&schema, nullptr, &error,
                                                 attribute_name.c_str(),
                                                 "encrypted_password", nullptr);

    if (error != nullptr) {
        LOG_F(ERROR,
              "Failed to delete password from Linux keyring for schema: {}, "
              "attribute: {}. Error: {}",
              schema_name, attribute_name, error->message);
        g_error_free(error);
    } else if (result) {
        LOG_F(INFO,
              "Password deleted successfully from Linux keyring for schema: "
              "{}, attribute: {}",
              schema_name, attribute_name);
    } else {
        LOG_F(ERROR,
              "Password not found for deletion in Linux keyring for schema: "
              "{}, attribute: {}",
              schema_name, attribute_name);
    }
}
#endif
}  // namespace atom::secret