#include "password.hpp"

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <iostream>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <wincred.h>
// clang-format on
#elif defined(__APPLE__)
#include <Security/Security.h>
#elif defined(__linux__)
#include <glib.h>
#include <libsecret/secret.h>
#endif

std::string AESCipher::encrypt(const std::string& plaintext,
                               const unsigned char* key) {
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);

    std::string ciphertext(plaintext.size() + AES_BLOCK_SIZE, '\0');

    AES_KEY aesKey;
    AES_set_encrypt_key(key, 128, &aesKey);

    int ciphertext_len = 0;
    AES_cfb128_encrypt(
        reinterpret_cast<const unsigned char*>(plaintext.c_str()),
        reinterpret_cast<unsigned char*>(&ciphertext[AES_BLOCK_SIZE]),
        plaintext.size(), &aesKey, iv, &ciphertext_len, AES_ENCRYPT);

    std::copy(iv, iv + AES_BLOCK_SIZE, ciphertext.begin());

    return ciphertext;
}

std::string AESCipher::decrypt(const std::string& ciphertext,
                               const unsigned char* key) {
    if (ciphertext.size() <= AES_BLOCK_SIZE) {
        return "";
    }

    unsigned char iv[AES_BLOCK_SIZE];
    std::copy(ciphertext.begin(), ciphertext.begin() + AES_BLOCK_SIZE, iv);

    std::string plaintext(ciphertext.size() - AES_BLOCK_SIZE, '\0');

    AES_KEY aesKey;
    AES_set_decrypt_key(key, 128, &aesKey);

    int plaintext_len = 0;
    AES_cfb128_encrypt(reinterpret_cast<const unsigned char*>(
                           ciphertext.c_str() + AES_BLOCK_SIZE),
                       reinterpret_cast<unsigned char*>(&plaintext[0]),
                       ciphertext.size() - AES_BLOCK_SIZE, &aesKey, iv,
                       &plaintext_len, AES_DECRYPT);

    plaintext.resize(plaintext_len);
    return plaintext;
}

PasswordManager::PasswordManager() { RAND_bytes(key, AES_BLOCK_SIZE); }

void PasswordManager::storePassword(const std::string& platformKey,
                                    const std::string& password) {
    std::string encryptedPassword = AESCipher::encrypt(password, key);

#if defined(_WIN32)
    storeToWindowsCredentialManager(platformKey, encryptedPassword);
#elif defined(__APPLE__)
    storeToMacKeychain(platformKey, "PasswordManager", encryptedPassword);
#elif defined(__linux__)
    storeToLinuxKeyring("PasswordManager", platformKey, encryptedPassword);
#endif
}

std::string PasswordManager::retrievePassword(const std::string& platformKey) {
    std::string encryptedPassword;

#if defined(_WIN32)
    encryptedPassword = retrieveFromWindowsCredentialManager(platformKey);
#elif defined(__APPLE__)
    encryptedPassword = retrieveFromMacKeychain(platformKey, "PasswordManager");
#elif defined(__linux__)
    encryptedPassword =
        retrieveFromLinuxKeyring("PasswordManager", platformKey);
#endif

    return AESCipher::decrypt(encryptedPassword, key);
}

void PasswordManager::deletePassword(const std::string& platformKey) {
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
        std::cout
            << "Password stored successfully in Windows Credential Manager.\n";
    } else {
        std::cerr << "Failed to store password: " << GetLastError() << "\n";
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
        return encryptedPassword;
    } else {
        std::cerr << "Failed to retrieve password: " << GetLastError() << "\n";
        return "";
    }
}

void PasswordManager::deleteFromWindowsCredentialManager(
    const std::string& target) {
    std::wstring wideTarget(target.begin(), target.end());
    if (CredDeleteW(wideTarget.c_str(), CRED_TYPE_GENERIC, 0)) {
        std::cout << "Password deleted successfully from Windows Credential "
                     "Manager.\n";
    } else {
        std::cerr << "Failed to delete password: " << GetLastError() << "\n";
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
        std::cout << "Password stored successfully in macOS Keychain.\n";
    } else {
        std::cerr << "Failed to store password: " << status << "\n";
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
        return encryptedPassword;
    } else {
        std::cerr << "Failed to retrieve password: " << status << "\n";
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
            std::cout << "Password deleted successfully from macOS Keychain.\n";
        } else {
            std::cerr << "Failed to delete password: " << status << "\n";
        }
        CFRelease(itemRef);
    } else {
        std::cerr << "Failed to find password for deletion: " << status << "\n";
    }
}

#elif defined(__linux__)
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
        std::cerr << "Failed to store password: " << error->message << "\n";
        g_error_free(error);
    } else {
        std::cout << "Password stored successfully in Linux keyring.\n";
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
        std::cerr << "Failed to retrieve password: " << error->message << "\n";
        g_error_free(error);
        return "";
    } else if (password == nullptr) {
        std::cerr << "Password not found.\n";
        return "";
    } else {
        std::string encryptedPassword(password);
        secret_password_free(password);
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
        std::cerr << "Failed to delete password: " << error->message << "\n";
        g_error_free(error);
    } else if (result) {
        std::cout << "Password deleted successfully from Linux keyring.\n";
    } else {
        std::cerr << "Password not found for deletion.\n";
    }
}
#endif