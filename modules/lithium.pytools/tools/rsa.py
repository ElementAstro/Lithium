from loguru import logger
import argparse
from pathlib import Path
from typing import Optional
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_OAEP, AES
from Crypto.Signature import pkcs1_15
from Crypto.Hash import SHA256
from Crypto.Random import get_random_bytes
from getpass import getpass

# Configure loguru
logger.add("file_{time}.log", rotation="1 week")

CHUNK_SIZE = 214  # RSA-2048 max chunk size for PKCS1_OAEP encryption
AES_CHUNK_SIZE = 64 * 1024  # 64KB for AES encryption chunks


def generate_keypair(key_size=2048):
    """Generate RSA key pair."""
    key = RSA.generate(key_size)
    private_key = key.export_key()
    public_key = key.publickey().export_key()
    return public_key, private_key


def save_key(key: bytes, filename: Path, passphrase: Optional[str] = None):
    """Save RSA key to a file."""
    try:
        with open(filename, 'wb') as f:
            if passphrase:
                encrypted_key = RSA.import_key(key).export_key(
                    passphrase=passphrase, pkcs=8, protection="scryptAndAES128-CBC"
                )
                f.write(encrypted_key)
            else:
                f.write(key)
        logger.info("Key saved to {}", filename)
    except Exception as e:
        logger.error("Error saving key to {}: {}", filename, e)


def load_key(filename: Path, passphrase: Optional[str] = None) -> Optional[bytes]:
    """Load RSA key from a file."""
    try:
        with open(filename, 'rb') as f:
            key = f.read()
            if passphrase:
                return RSA.import_key(key, passphrase=passphrase)
            return RSA.import_key(key)
    except FileNotFoundError:
        logger.error("Key file {} not found.", filename)
        return None
    except Exception as e:
        logger.error("Error loading key from {}: {}", filename, e)
        return None


def encrypt_file(input_file: Path, output_file: Path, public_key: bytes):
    """Encrypt a file using RSA public key."""
    key = RSA.import_key(public_key)
    cipher_rsa = PKCS1_OAEP.new(key)

    try:
        with open(input_file, 'rb') as f_in, open(output_file, 'wb') as f_out:
            while chunk := f_in.read(CHUNK_SIZE):
                encrypted_chunk = cipher_rsa.encrypt(chunk)
                f_out.write(encrypted_chunk)
        logger.info("File encrypted successfully to {}", output_file)
    except Exception as e:
        logger.error("Error during encryption: {}", e)


def decrypt_file(input_file: Path, output_file: Path, private_key: bytes):
    """Decrypt a file using RSA private key."""
    key = RSA.import_key(private_key)
    cipher_rsa = PKCS1_OAEP.new(key)

    try:
        with open(input_file, 'rb') as f_in, open(output_file, 'wb') as f_out:
            # Size of encrypted chunk for RSA-2048
            while chunk := f_in.read(256):
                decrypted_chunk = cipher_rsa.decrypt(chunk)
                f_out.write(decrypted_chunk)
        logger.info("File decrypted successfully to {}", output_file)
    except Exception as e:
        logger.error("Error during decryption: {}", e)


def aes_encrypt_file(input_file: Path, output_file: Path, key: bytes):
    """Encrypt a file using AES."""
    try:
        cipher = AES.new(key, AES.MODE_GCM)
        with open(input_file, 'rb') as f_in, open(output_file, 'wb') as f_out:
            f_out.write(cipher.nonce)
            while chunk := f_in.read(AES_CHUNK_SIZE):
                ciphertext, _ = cipher.encrypt_and_digest(chunk)
                f_out.write(ciphertext)
        logger.info("File AES encrypted successfully to {}", output_file)
    except Exception as e:
        logger.error("Error during AES encryption: {}", e)


def aes_decrypt_file(input_file: Path, output_file: Path, key: bytes):
    """Decrypt a file using AES."""
    try:
        with open(input_file, 'rb') as f_in, open(output_file, 'wb') as f_out:
            nonce = f_in.read(16)  # AES GCM nonce size
            cipher = AES.new(key, AES.MODE_GCM, nonce=nonce)
            while chunk := f_in.read(AES_CHUNK_SIZE):
                decrypted_chunk = cipher.decrypt(chunk)
                f_out.write(decrypted_chunk)
        logger.info("File AES decrypted successfully to {}", output_file)
    except Exception as e:
        logger.error("Error during AES decryption: {}", e)


def sign_file(input_file: Path, output_file: Path, private_key: bytes):
    """Sign a file using RSA private key."""
    key = RSA.import_key(private_key)

    try:
        with open(input_file, 'rb') as f:
            data = f.read()

        h = SHA256.new(data)
        signature = pkcs1_15.new(key).sign(h)

        with open(output_file, 'wb') as f:
            f.write(signature)
        logger.info("File signed and signature saved to {}", output_file)
    except Exception as e:
        logger.error("Error during signing: {}", e)


def verify_file(input_file: Path, signature_file: Path, public_key: bytes) -> bool:
    """Verify a file signature using RSA public key."""
    key = RSA.import_key(public_key)

    try:
        with open(input_file, 'rb') as f:
            data = f.read()

        with open(signature_file, 'rb') as f:
            signature = f.read()

        h = SHA256.new(data)
        pkcs1_15.new(key).verify(h, signature)
        logger.info("Signature is valid.")
        return True
    except (ValueError, TypeError):
        logger.error("Signature is invalid.")
        return False
    except Exception as e:
        logger.error("Error during verification: {}", e)
        return False


def hash_file(input_file: Path) -> str:
    """Generate SHA256 hash of a file."""
    h = SHA256.new()
    try:
        with open(input_file, 'rb') as f:
            while chunk := f.read(8192):
                h.update(chunk)
        return h.hexdigest()
    except Exception as e:
        logger.error("Error hashing file: {}", e)
        return ""


def main():
    """Main function to parse arguments and perform actions."""
    parser = argparse.ArgumentParser(
        description=(
            'Enhanced RSA/AES Encryption Tool with password protection and file hashing'
        )
    )
    parser.add_argument(
        'action',
        choices=[
            'generate', 'encrypt', 'decrypt', 'sign', 'verify', 'hash', 'aes-encrypt', 'aes-decrypt'
        ],
        help='Action to perform'
    )
    parser.add_argument('-i', '--input', help='Input file', type=Path)
    parser.add_argument('-o', '--output', help='Output file', type=Path)
    parser.add_argument('-k', '--key', help='Key file', type=Path)
    parser.add_argument('-s', '--signature', help='Signature file', type=Path)
    parser.add_argument(
        '-p', '--passphrase',
        help='Passphrase for key encryption/decryption',
        action='store_true'
    )
    args = parser.parse_args()

    passphrase = None
    if args.passphrase:
        passphrase = getpass("Enter passphrase: ")

    if args.action == 'generate':
        public_key, private_key = generate_keypair()
        save_key(public_key, 'public_key.pem')
        save_key(private_key, 'private_key.pem', passphrase=passphrase)
        logger.info(
            "Keys generated and saved as 'public_key.pem' and 'private_key.pem'")

    elif args.action == 'encrypt':
        public_key = load_key(args.key)
        if public_key:
            encrypt_file(args.input, args.output, public_key)

    elif args.action == 'decrypt':
        private_key = load_key(args.key, passphrase=passphrase)
        if private_key:
            decrypt_file(args.input, args.output, private_key)

    elif args.action == 'sign':
        private_key = load_key(args.key, passphrase=passphrase)
        if private_key:
            sign_file(args.input, args.output, private_key)

    elif args.action == 'verify':
        public_key = load_key(args.key)
        if public_key:
            verify_file(args.input, args.signature, public_key)

    elif args.action == 'hash':
        hash_value = hash_file(args.input)
        logger.info("SHA256 hash of {}: {}", args.input, hash_value)

    elif args.action == 'aes-encrypt':
        aes_key = get_random_bytes(16)  # AES-128 key
        aes_encrypt_file(args.input, args.output, aes_key)
        save_key(aes_key, 'aes_key.bin')  # Save the AES key to a file

    elif args.action == 'aes-decrypt':
        aes_key = load_key('aes_key.bin')  # Load the AES key from file
        if aes_key:
            aes_decrypt_file(args.input, args.output, aes_key)


if __name__ == '__main__':
    main()
