from loguru import logger
import argparse
from pathlib import Path
from typing import Optional, Tuple
from Crypto.PublicKey import RSA
from Crypto.Cipher import PKCS1_OAEP, AES
from Crypto.Signature import pkcs1_15
from Crypto.Hash import SHA256
from Crypto.Random import get_random_bytes
from getpass import getpass
import os
import sys
import struct

# Configure Loguru for logging
logger.remove()  # Remove the default logger
logger.add(
    "rsa.log",
    rotation="1 week",
    retention="4 weeks",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    level="DEBUG",
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
)
logger.add(
    sys.stderr,
    level="INFO",
    format="<level>{message}</level>",
)

CHUNK_SIZE_RSA = 214  # For RSA-2048 with PKCS1_OAEP padding
AES_CHUNK_SIZE = 64 * 1024  # 64KB for AES encryption chunks
RSA_KEY_SIZE = 2048  # Default RSA key size


def generate_keypair(key_size: int = RSA_KEY_SIZE) -> Tuple[bytes, bytes]:
    """Generate RSA key pair."""
    logger.info(f"Generating RSA key pair with size {key_size} bits.")
    try:
        key = RSA.generate(key_size)
        private_key = key.export_key()
        public_key = key.publickey().export_key()
        logger.debug("RSA key pair generated successfully.")
        return public_key, private_key
    except Exception as e:
        logger.exception(f"Failed to generate RSA key pair: {e}")
        raise


def save_key(key: bytes, filename: Path, passphrase: Optional[str] = None):
    """Save RSA key to a file."""
    try:
        with open(filename, 'wb') as f:
            if passphrase:
                logger.debug(
                    f"Encrypting key with passphrase for file {filename}.")
                encrypted_key = RSA.import_key(key).export_key(
                    passphrase=passphrase,
                    pkcs=8,
                    protection="scryptAndAES128-CBC"
                )
                f.write(encrypted_key)
            else:
                logger.debug(f"Saving unencrypted key to file {filename}.")
                f.write(key)
        logger.info(f"Key saved to {filename}.")
    except Exception as e:
        logger.error(f"Error saving key to {filename}: {e}")
        raise


def load_key(filename: Path, passphrase: Optional[str] = None) -> Optional[bytes]:
    """Load RSA key from a file."""
    try:
        logger.debug(f"Loading key from file {filename}.")
        with open(filename, 'rb') as f:
            key_data = f.read()
            key = RSA.import_key(key_data, passphrase=passphrase)
        logger.info(f"Key loaded successfully from {filename}.")
        return key
    except FileNotFoundError:
        logger.error(f"Key file {filename} not found.")
        return None
    except (ValueError, TypeError) as e:
        logger.error(
            f"Incorrect passphrase or corrupted key file {filename}: {e}")
        return None
    except Exception as e:
        logger.error(f"Error loading key from {filename}: {e}")
        return None


def hybrid_encrypt(input_file: Path, output_file: Path, public_key: RSA.RsaKey):
    """
    Encrypt a file using hybrid RSA and AES encryption.

    The AES key is encrypted with RSA and stored alongside the encrypted data.
    The file format:
    [RSA_encrypted_AES_key_length (4 bytes)][RSA_encrypted_AES_key][AES_nonce (16 bytes)][AES_tag (16 bytes)][AES_encrypted_data]
    """
    logger.info(
        f"Starting hybrid encryption for file {input_file} to {output_file}.")
    try:
        # Generate AES key
        aes_key = get_random_bytes(32)  # AES-256
        logger.debug("AES key generated for encryption.")

        # Encrypt AES key with RSA public key
        cipher_rsa = PKCS1_OAEP.new(public_key)
        encrypted_aes_key = cipher_rsa.encrypt(aes_key)
        encrypted_aes_key_length = len(encrypted_aes_key)
        logger.debug("AES key encrypted with RSA public key.")

        # Initialize AES cipher in GCM mode
        cipher_aes = AES.new(aes_key, AES.MODE_GCM)
        nonce = cipher_aes.nonce
        logger.debug("AES cipher initialized in GCM mode.")

        with open(input_file, 'rb') as f_in, open(output_file, 'wb') as f_out:
            # Write RSA_encrypted_AES_key_length
            f_out.write(struct.pack(">I", encrypted_aes_key_length))
            # Write RSA_encrypted_AES_key
            f_out.write(encrypted_aes_key)
            # Write AES nonce
            f_out.write(nonce)
            logger.debug(
                "RSA encrypted AES key and nonce written to output file.")

            while chunk := f_in.read(AES_CHUNK_SIZE):
                encrypted_chunk, tag = cipher_aes.encrypt_and_digest(chunk)
                f_out.write(encrypted_chunk)
                f_out.write(tag)
            logger.info(f"File encrypted successfully to {output_file}.")
    except Exception as e:
        logger.error(f"Error during hybrid encryption: {e}")
        raise


def hybrid_decrypt(input_file: Path, output_file: Path, private_key: RSA.RsaKey):
    """
    Decrypt a file using hybrid RSA and AES decryption.

    Expects the file format:
    [RSA_encrypted_AES_key_length (4 bytes)][RSA_encrypted_AES_key][AES_nonce (16 bytes)][AES_tag (16 bytes)][AES_encrypted_data]
    """
    logger.info(
        f"Starting hybrid decryption for file {input_file} to {output_file}.")
    try:
        with open(input_file, 'rb') as f_in:
            # Read RSA_encrypted_AES_key_length
            encrypted_aes_key_length = struct.unpack(">I", f_in.read(4))[0]
            logger.debug(
                f"Encrypted AES key length: {encrypted_aes_key_length} bytes.")

            # Read RSA_encrypted_AES_key
            encrypted_aes_key = f_in.read(encrypted_aes_key_length)
            logger.debug("Encrypted AES key read from file.")

            # Decrypt AES key with RSA private key
            cipher_rsa = PKCS1_OAEP.new(private_key)
            aes_key = cipher_rsa.decrypt(encrypted_aes_key)
            logger.debug("AES key decrypted with RSA private key.")

            # Read AES nonce
            nonce = f_in.read(16)
            logger.debug("AES nonce read from file.")

            cipher_aes = AES.new(aes_key, AES.MODE_GCM, nonce=nonce)
            logger.debug("AES cipher initialized for decryption.")

            with open(output_file, 'wb') as f_out:
                while True:
                    encrypted_chunk = f_in.read(AES_CHUNK_SIZE)
                    if not encrypted_chunk:
                        break
                    tag = f_in.read(16)
                    decrypted_chunk = cipher_aes.decrypt(encrypted_chunk)
                    try:
                        cipher_aes.verify(tag)
                        f_out.write(decrypted_chunk)
                    except ValueError:
                        logger.error(
                            "Integrity check failed during AES decryption.")
                        raise ValueError("Key incorrect or message corrupted.")
            logger.info(f"File decrypted successfully to {output_file}.")
    except Exception as e:
        logger.error(f"Error during hybrid decryption: {e}")
        raise


def aes_encrypt_file(input_file: Path, output_file: Path, key: bytes):
    """Encrypt a file using AES in GCM mode."""
    logger.info(
        f"Starting AES encryption for file {input_file} to {output_file}.")
    try:
        cipher = AES.new(key, AES.MODE_GCM)
        filename = output_file.name
        with open(input_file, 'rb') as f_in, open(output_file, 'wb') as f_out:
            f_out.write(cipher.nonce)
            logger.debug("AES nonce written to output file.")
            while chunk := f_in.read(AES_CHUNK_SIZE):
                ciphertext, tag = cipher.encrypt_and_digest(chunk)
                f_out.write(ciphertext)
                f_out.write(tag)
        logger.info(
            f"AES encryption completed successfully for {output_file}.")
    except Exception as e:
        logger.error(f"Error during AES encryption: {e}")
        raise


def aes_decrypt_file(input_file: Path, output_file: Path, key: bytes):
    """Decrypt a file using AES in GCM mode."""
    logger.info(
        f"Starting AES decryption for file {input_file} to {output_file}.")
    try:
        with open(input_file, 'rb') as f_in:
            nonce = f_in.read(16)
            cipher = AES.new(key, AES.MODE_GCM, nonce=nonce)
            logger.debug("AES cipher initialized with nonce.")

            with open(output_file, 'wb') as f_out:
                while True:
                    ciphertext = f_in.read(AES_CHUNK_SIZE)
                    if not ciphertext:
                        break
                    tag = f_in.read(16)
                    decrypted_chunk = cipher.decrypt(ciphertext)
                    try:
                        cipher.verify(tag)
                        f_out.write(decrypted_chunk)
                    except ValueError:
                        logger.error(
                            "Integrity check failed during AES decryption.")
                        raise ValueError("Key incorrect or message corrupted.")
        logger.info(
            f"AES decryption completed successfully for {output_file}.")
    except Exception as e:
        logger.error(f"Error during AES decryption: {e}")
        raise


def sign_file(input_file: Path, signature_file: Path, private_key: RSA.RsaKey):
    """Sign a file using RSA private key."""
    logger.info(
        f"Starting signing of file {input_file}, signature will be saved to {signature_file}.")
    try:
        with open(input_file, 'rb') as f:
            data = f.read()
        logger.debug("File data read for signing.")

        h = SHA256.new(data)
        signature = pkcs1_15.new(private_key).sign(h)
        with open(signature_file, 'wb') as f:
            f.write(signature)
        logger.info(
            f"File signed successfully. Signature saved to {signature_file}.")
    except (ValueError, TypeError) as e:
        logger.error(f"Failed to sign file {input_file}: {e}")
        raise
    except Exception as e:
        logger.error(f"Error during signing file {input_file}: {e}")
        raise


def verify_file(input_file: Path, signature_file: Path, public_key: RSA.RsaKey) -> bool:
    """Verify a file signature using RSA public key."""
    logger.info(
        f"Starting verification of file {input_file} with signature {signature_file}.")
    try:
        with open(input_file, 'rb') as f:
            data = f.read()
        with open(signature_file, 'rb') as f:
            signature = f.read()
        logger.debug("File data and signature read for verification.")

        h = SHA256.new(data)
        pkcs1_15.new(public_key).verify(h, signature)
        logger.info("Signature is valid.")
        return True
    except (ValueError, TypeError):
        logger.error("Signature is invalid.")
        return False
    except Exception as e:
        logger.error(f"Error during verification of file {input_file}: {e}")
        return False


def hash_file(input_file: Path) -> str:
    """Generate SHA256 hash of a file."""
    logger.info(f"Generating SHA256 hash for file {input_file}.")
    h = SHA256.new()
    try:
        with open(input_file, 'rb') as f:
            while chunk := f.read(8192):
                h.update(chunk)
        hash_digest = h.hexdigest()
        logger.info(f"SHA256 hash for {input_file}: {hash_digest}")
        return hash_digest
    except Exception as e:
        logger.error(f"Error hashing file {input_file}: {e}")
        return ""


def main():
    """Main function to parse arguments and perform actions."""
    parser = argparse.ArgumentParser(
        description='Enhanced RSA/AES Encryption Tool with Hybrid Encryption and File Hashing'
    )
    parser.add_argument(
        'action',
        choices=[
            'generate', 'encrypt', 'decrypt', 'sign', 'verify', 'hash', 'aes-encrypt', 'aes-decrypt'
        ],
        help='Action to perform'
    )
    parser.add_argument('-i', '--input', help='Input file path', type=Path)
    parser.add_argument('-o', '--output', help='Output file path', type=Path)
    parser.add_argument('-k', '--key', help='Key file path', type=Path)
    parser.add_argument('-s', '--signature',
                        help='Signature file path for verify', type=Path)
    parser.add_argument(
        '-p', '--passphrase',
        help='Enable passphrase for key encryption/decryption',
        action='store_true'
    )
    args = parser.parse_args()

    passphrase = None
    if args.passphrase:
        passphrase = getpass("Enter passphrase: ")

    try:
        if args.action == 'generate':
            public_key, private_key = generate_keypair()
            save_key(public_key, 'public_key.pem')
            save_key(private_key, 'private_key.pem', passphrase=passphrase)
            logger.info(
                "RSA key pair generated and saved as 'public_key.pem' and 'private_key.pem'.")

        elif args.action == 'encrypt':
            if not args.input or not args.output or not args.key:
                logger.error(
                    "Encryption requires input file, output file, and public key.")
                print(
                    "Error: Encryption requires input file, output file, and public key.")
                sys.exit(1)
            public_key = load_key(args.key)
            if public_key:
                hybrid_encrypt(args.input, args.output, public_key)
            else:
                logger.error("Public key loading failed. Encryption aborted.")
                print("Error: Public key loading failed. Check logs for details.")
                sys.exit(1)

        elif args.action == 'decrypt':
            if not args.input or not args.output or not args.key:
                logger.error(
                    "Decryption requires input file, output file, and private key.")
                print(
                    "Error: Decryption requires input file, output file, and private key.")
                sys.exit(1)
            private_key = load_key(args.key, passphrase=passphrase)
            if private_key:
                hybrid_decrypt(args.input, args.output, private_key)
            else:
                logger.error("Private key loading failed. Decryption aborted.")
                print("Error: Private key loading failed. Check logs for details.")
                sys.exit(1)

        elif args.action == 'sign':
            if not args.input or not args.output or not args.key:
                logger.error(
                    "Signing requires input file, signature file, and private key.")
                print(
                    "Error: Signing requires input file, signature file, and private key.")
                sys.exit(1)
            private_key = load_key(args.key, passphrase=passphrase)
            if private_key:
                sign_file(args.input, args.output, private_key)
            else:
                logger.error("Private key loading failed. Signing aborted.")
                print("Error: Private key loading failed. Check logs for details.")
                sys.exit(1)

        elif args.action == 'verify':
            if not args.input or not args.signature or not args.key:
                logger.error(
                    "Verification requires input file, signature file, and public key.")
                print(
                    "Error: Verification requires input file, signature file, and public key.")
                sys.exit(1)
            public_key = load_key(args.key)
            if public_key:
                is_valid = verify_file(args.input, args.signature, public_key)
                if is_valid:
                    print("Signature is valid.")
                else:
                    print("Signature is invalid.")
            else:
                logger.error(
                    "Public key loading failed. Verification aborted.")
                print("Error: Public key loading failed. Check logs for details.")
                sys.exit(1)

        elif args.action == 'hash':
            if not args.input:
                logger.error("Hashing requires an input file.")
                print("Error: Hashing requires an input file.")
                sys.exit(1)
            hash_digest = hash_file(args.input)
            if hash_digest:
                print(f"SHA256 Hash: {hash_digest}")
            else:
                print("Error: Failed to generate hash. Check logs for details.")

        elif args.action == 'aes-encrypt':
            if not args.input or not args.output:
                logger.error("AES encryption requires input and output files.")
                print("Error: AES encryption requires input and output files.")
                sys.exit(1)
            aes_key = get_random_bytes(32)  # AES-256
            aes_encrypt_file(args.input, args.output, aes_key)
            save_key(aes_key, 'aes_key.bin')
            logger.info("AES key generated and saved to 'aes_key.bin'.")

        elif args.action == 'aes-decrypt':
            if not args.input or not args.output:
                logger.error("AES decryption requires input and output files.")
                print("Error: AES decryption requires input and output files.")
                sys.exit(1)
            aes_key = load_key('aes_key.bin')
            if aes_key:
                aes_decrypt_file(args.input, args.output, aes_key)
            else:
                logger.error("AES key loading failed. AES decryption aborted.")
                print("Error: AES key loading failed. Check logs for details.")
                sys.exit(1)

        logger.info(f"Action '{args.action}' completed successfully.")

        # For actions that are not hashing or verification, notify user via print
        if args.action not in ['hash', 'verify']:
            print(f"Action '{args.action}' completed successfully.")

    except Exception as e:
        logger.critical(f"An unexpected error occurred: {e}")
        print(f"Critical Error: {e}. Check logs for details.")
        sys.exit(1)


if __name__ == '__main__':
    main()
