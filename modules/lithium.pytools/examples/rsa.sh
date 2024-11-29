# Example 1: Generate RSA key pair
# This example generates an RSA key pair and saves the keys to 'public_key.pem' and 'private_key.pem'.
$ python rsa.py generate

# Example 2: Encrypt a file using RSA and AES hybrid encryption
# This example encrypts the specified input file using the provided public key and saves the encrypted file to the output path.
$ python rsa.py encrypt -i input.txt -o encrypted.bin -k public_key.pem

# Example 3: Decrypt a file using RSA and AES hybrid decryption
# This example decrypts the specified input file using the provided private key and saves the decrypted file to the output path.
$ python rsa.py decrypt -i encrypted.bin -o decrypted.txt -k private_key.pem

# Example 4: Sign a file using an RSA private key
# This example signs the specified input file using the provided private key and saves the signature to the output path.
$ python rsa.py sign -i input.txt -o signature.sig -k private_key.pem

# Example 5: Verify a file's signature using an RSA public key
# This example verifies the signature of the specified input file using the provided public key.
$ python rsa.py verify -i input.txt -s signature.sig -k public_key.pem

# Example 6: Generate SHA256 hash of a file
# This example generates the SHA256 hash of the specified input file and prints the hash.
$ python rsa.py hash -i input.txt

# Example 7: Encrypt a file using AES
# This example encrypts the specified input file using AES and saves the encrypted file to the output path.
$ python rsa.py aes-encrypt -i input.txt -o encrypted_aes.bin

# Example 8: Decrypt a file using AES
# This example decrypts the specified input file using AES and saves the decrypted file to the output path.
$ python rsa.py aes-decrypt -i encrypted_aes.bin -o decrypted_aes.txt