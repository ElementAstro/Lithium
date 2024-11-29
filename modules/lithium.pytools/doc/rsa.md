# Enhanced RSA/AES Encryption Tool Documentation

This document provides a comprehensive guide on how to use the **Enhanced RSA/AES Encryption Tool**, a Python script designed for secure encryption and decryption of files using RSA and AES algorithms. The tool includes functionalities for key generation, file signing, verification, and hashing, along with enhanced logging and user interaction features.

---

## Key Features

- **RSA Key Generation**: Generate RSA public and private key pairs.
- **File Encryption and Decryption**: Encrypt files using hybrid RSA and AES encryption methods.
- **File Signing and Verification**: Sign files with RSA private keys and verify signatures using RSA public keys.
- **File Hashing**: Generate SHA256 hashes for files.
- **User-Friendly Interface**: Utilizes `Rich` for beautified terminal outputs and prompts.
- **Detailed Logging**: Uses `Loguru` for comprehensive logging of operations and errors.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `pycryptodome`, `loguru`, `rich`.

Install the required libraries using pip:

```bash
pip install pycryptodome loguru rich
```

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python rsa_aes_tool.py <action> [options]
```

### Available Actions

1. **Generate RSA Key Pair**

   ```bash
   python rsa_aes_tool.py generate
   ```

2. **Encrypt a File**

   ```bash
   python rsa_aes_tool.py encrypt -i <input_file> -o <output_file> -k <public_key_file>
   ```

3. **Decrypt a File**

   ```bash
   python rsa_aes_tool.py decrypt -i <input_file> -o <output_file> -k <private_key_file>
   ```

4. **Sign a File**

   ```bash
   python rsa_aes_tool.py sign -i <input_file> -o <signature_file> -k <private_key_file>
   ```

5. **Verify a File**

   ```bash
   python rsa_aes_tool.py verify -i <input_file> -s <signature_file> -k <public_key_file>
   ```

6. **Hash a File**

   ```bash
   python rsa_aes_tool.py hash -i <input_file>
   ```

7. **AES Encrypt a File**

   ```bash
   python rsa_aes_tool.py aes-encrypt -i <input_file> -o <output_file>
   ```

8. **AES Decrypt a File**

   ```bash
   python rsa_aes_tool.py aes-decrypt -i <input_file> -o <output_file>
   ```

### Command-Line Options

- **`-i`, `--input`**: Path to the input file.
- **`-o`, `--output`**: Path to save the output file.
- **`-k`, `--key`**: Path to the key file (public or private).
- **`-s`, `--signature`**: Path to the signature file for verification.
- **`-p`, `--passphrase`**: Enable passphrase for key encryption/decryption (optional).

---

## Example Usage

### Generate RSA Key Pair

To generate an RSA key pair:

```bash
python rsa_aes_tool.py generate
```

### Encrypt a File

To encrypt a file using a public key:

```bash
python rsa_aes_tool.py encrypt -i myfile.txt -o myfile.enc -k public_key.pem
```

### Decrypt a File

To decrypt a file using a private key:

```bash
python rsa_aes_tool.py decrypt -i myfile.enc -o myfile_decrypted.txt -k private_key.pem
```

### Sign a File

To sign a file:

```bash
python rsa_aes_tool.py sign -i myfile.txt -o myfile.sig -k private_key.pem
```

### Verify a File

To verify a signed file:

```bash
python rsa_aes_tool.py verify -i myfile.txt -s myfile.sig -k public_key.pem
```

### Hash a File

To generate a SHA256 hash for a file:

```bash
python rsa_aes_tool.py hash -i myfile.txt
```

### AES Encrypt a File

To encrypt a file using AES:

```bash
python rsa_aes_tool.py aes-encrypt -i myfile.txt -o myfile_aes.enc
```

### AES Decrypt a File

To decrypt a file using AES:

```bash
python rsa_aes_tool.py aes-decrypt -i myfile_aes.enc -o myfile_decrypted.txt
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `rsa.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **Enhanced RSA/AES Encryption Tool** is a powerful utility for secure file encryption and decryption. It simplifies the process of managing cryptographic operations while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their encryption and decryption needs.
