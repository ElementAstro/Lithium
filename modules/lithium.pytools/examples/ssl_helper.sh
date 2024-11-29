# Example 1: Create a self-signed SSL certificate
# This example creates a self-signed SSL certificate for the specified hostname and saves it in the specified directory.
$ python ssl_helper.py example.com --cert-dir /path/to/certs

# Example 2: Create a self-signed SSL certificate with SANs
# This example creates a self-signed SSL certificate with Subject Alternative Names (SANs) for the specified hostname.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --san www.example.com api.example.com

# Example 3: View certificate details
# This example views the details of the SSL certificate for the specified hostname.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --view

# Example 4: Check certificate expiry
# This example checks if the SSL certificate for the specified hostname is about to expire.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --check-expiry

# Example 5: Renew an SSL certificate
# This example renews the SSL certificate for the specified hostname and extends its validity.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --renew

# Example 6: Export certificate and key as PKCS#12 file
# This example exports the SSL certificate and private key as a PKCS#12 (PFX) file with the specified password.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --export-pfx --password mypassword

# Example 7: Create a CA certificate
# This example creates a self-signed CA certificate for the specified hostname.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --cert-type ca

# Example 8: Generate a Certificate Revocation List (CRL)
# This example generates a CRL and saves it in the specified directory.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --crl --crl-dir /path/to/crl

# Example 9: Revoke certificates by serial numbers
# This example revokes certificates with the specified serial numbers and generates a CRL.
$ python ssl_helper.py example.com --cert-dir /path/to/certs --crl --crl-dir /path/to/crl --revoke 1234567890ABCDEF 0987654321FEDCBA