# SSL Helper Tool Documentation

This document provides a comprehensive guide on how to use the **SSL Helper Tool**, a Python script designed for enhanced management of SSL certificates. The tool allows users to create, manage, renew, and export SSL certificates while providing robust logging and exception handling.

---

## Key Features

1. **Certificate Creation**: Generate self-signed SSL certificates with optional Subject Alternative Names (SANs).
2. **Certificate Management**: Renew existing certificates, check expiry, and revoke certificates.
3. **Export Options**: Export certificates and private keys as PKCS#12 (PFX) files.
4. **CRL Generation**: Generate Certificate Revocation Lists (CRLs).
5. **Detailed Logging**: Uses the `Loguru` library for detailed logging of operations and errors.
6. **Code Context Extraction**: Automatically extracts and highlights relevant lines of code from the source file where the error occurred.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `cryptography`, `loguru`.

Install the required libraries using pip:

```bash
pip install cryptography loguru
```

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python ssl_helper.py <hostname> [options]
```

### Command-Line Options

- **`<hostname>`**: The primary hostname for which the certificate is being generated.
- **`--cert-dir <DIR>`**: Directory to save the certificates (default: `./certs`).
- **`--key-size <SIZE>`**: Size of RSA key in bits (default: `2048`).
- **`--valid-days <DAYS>`**: Number of days the certificate is valid (default: `365`).
- **`--san <SAN ...>`**: List of Subject Alternative Names (e.g., `--san www.example.com api.example.com`).
- **`--view`**: View certificate details.
- **`--check-expiry`**: Check if the certificate is about to expire.
- **`--renew`**: Renew the certificate.
- **`--export-pfx`**: Export the certificate and key as a PKCS#12 file.
- **`--cert-type <TYPE>`**: Type of certificate to create (`server`, `client`, or `ca`).
- **`--crl`**: Generate a Certificate Revocation List (CRL).
- **`--crl-dir <DIR>`**: Directory to save the CRL file (default: `./crl`).
- **`--password <PASS>`**: Password for exporting PKCS#12 file.
- **`--revoke <SERIAL ...>`**: Revoke certificates by their serial numbers (hexadecimal).
- **`--help`**: Show help message and exit.

---

## Example Usage

### Create a Self-Signed Certificate

To create a self-signed SSL certificate for the hostname `example.com`:

```bash
python ssl_helper.py example.com --cert-dir ./certs --key-size 2048 --valid-days 365 --san www.example.com
```

### View Certificate Details

To view the details of the generated certificate:

```bash
python ssl_helper.py example.com --view
```

### Check Certificate Expiry

To check if the certificate is about to expire:

```bash
python ssl_helper.py example.com --check-expiry
```

### Renew a Certificate

To renew the existing certificate:

```bash
python ssl_helper.py example.com --renew
```

### Export to PKCS#12

To export the certificate and key as a PKCS#12 file:

```bash
python ssl_helper.py example.com --export-pfx --password your_password
```

### Generate a CRL

To generate a Certificate Revocation List:

```bash
python ssl_helper.py example.com --crl --crl-dir ./crl --revoke 1234567890abcdef
```

---

## How the Script Works

1. **Key Generation**: The script generates an RSA private key of specified size.
2. **Certificate Creation**: It creates a self-signed certificate for the specified hostname and optional SANs, with a defined validity period.
3. **Certificate Management**: Users can renew certificates, check for expiry, and revoke certificates by serial number.
4. **Exporting**: The certificate and private key can be exported to a PKCS#12 file for easier distribution.
5. **CRL Generation**: Users can generate a CRL to manage revoked certificates.
6. **Detailed Logging**: The script logs all operations and errors, providing a detailed history of actions taken.

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `ssl_helper.log`, capturing detailed information about operations, warnings, and errors. The logging configuration ensures that logs are rotated and compressed, preserving space and maintaining a history of actions.

---

## Conclusion

The **SSL Helper Tool** is a robust solution for managing SSL certificates, providing essential functionalities for certificate creation, management, and export. With detailed logging and error handling, it simplifies the process of working with SSL certificates for developers and system administrators. By following this documentation, users can effectively utilize the tool for their SSL certificate management needs.
