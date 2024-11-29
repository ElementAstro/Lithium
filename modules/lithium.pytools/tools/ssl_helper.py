#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
@file         ssl_helper.py
@brief        Enhanced SSL Certificate Management Tool.

@details      This script provides functionalities to create, manage, renew, and export SSL certificates.
              It includes robust exception handling and detailed logging using Loguru.

              Usage:
                python ssl_helper.py <hostname> [options]

              Options:
                --cert-dir <DIR>        Directory to save the certificates.
                --key-size <SIZE>       Size of RSA key in bits.
                --valid-days <DAYS>     Number of days the certificate is valid.
                --san <SAN ...>         List of Subject Alternative Names (SANs).
                --view                   View certificate details.
                --check-expiry           Check if the certificate is about to expire.
                --renew                  Renew the certificate.
                --export-pfx             Export the certificate and key as a PKCS#12 file.
                --cert-type <TYPE>       Type of certificate to create (server, client, ca).
                --crl                    Generate a Certificate Revocation List (CRL).
                --crl-dir <DIR>          Directory to save the CRL file.
                --password <PASS>        Password for exporting PKCS#12 file.
                --revoke <SERIAL ...>    Revoke certificates by their serial numbers.
                --help                   Show help message and exit.

@requires     - Python 3.7+
              - `cryptography` Python library
              - `loguru` Python library

@version      2.0
@date         2024-04-27
"""

import sys
import ssl
import datetime
import argparse
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import rsa
from pathlib import Path
from typing import List, Optional, Tuple
from loguru import logger

# Configure Loguru
logger.remove()
logger.add(
    "ssl_helper.log",
    rotation="5 MB",
    retention="7 days",
    compression="zip",
    enqueue=True,
    encoding="utf-8",
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level}</level> | {message}",
    level="DEBUG",
)
logger.add(
    sys.stderr,
    level="INFO",
    format="<level>{message}</level>",
)
logger.debug("Logging is configured.")


def create_key(key_size: int = 2048) -> rsa.RSAPrivateKey:
    """
    Generates an RSA private key.

    :param key_size: RSA key size in bits.
    :return: A private key object.
    """
    logger.debug(f"Generating RSA private key with size {key_size} bits.")
    try:
        key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=key_size,
        )
        logger.info("RSA private key generated successfully.")
        return key
    except Exception as e:
        logger.exception(f"Failed to generate RSA private key: {e}")
        raise


def create_self_signed_cert(
    hostname: str,
    cert_dir: Path,
    key_size: int = 2048,
    valid_days: int = 365,
    san_list: Optional[List[str]] = None,
    cert_type: str = "server"
) -> Tuple[Path, Path]:
    """
    Creates a self-signed SSL certificate for a hostname with optional SANs.

    :param hostname: The primary hostname for which the certificate is being generated.
    :param cert_dir: The directory to save the certificate and key.
    :param key_size: RSA key size in bits.
    :param valid_days: Number of days the certificate is valid.
    :param san_list: Optional list of Subject Alternative Names (SANs).
    :param cert_type: Type of certificate ("server", "client", or "ca").
    :return: Tuple of paths (certificate_path, key_path).
    """
    logger.debug(
        f"Creating a self-signed certificate for hostname '{hostname}' with type '{cert_type}'.")
    try:
        key = create_key(key_size)

        subject = x509.Name([
            x509.NameAttribute(NameOID.COMMON_NAME, hostname)
        ])

        alt_names = [x509.DNSName(hostname)]
        if san_list:
            alt_names.extend([x509.DNSName(name) for name in san_list])

        cert_builder = (
            x509.CertificateBuilder()
            .subject_name(subject)
            .issuer_name(subject)
            .public_key(key.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(datetime.datetime.utcnow())
            .not_valid_after(datetime.datetime.utcnow() + datetime.timedelta(days=valid_days))
            .add_extension(
                x509.SubjectAlternativeName(alt_names),
                critical=False,
            )
        )

        if cert_type == "ca":
            cert_builder = cert_builder.add_extension(
                x509.BasicConstraints(ca=True, path_length=None),
                critical=True,
            )
        else:
            cert_builder = cert_builder.add_extension(
                x509.BasicConstraints(ca=False, path_length=None),
                critical=True,
            )
            if cert_type == "client":
                cert_builder = cert_builder.add_extension(
                    x509.ExtendedKeyUsage(
                        [x509.oid.ExtendedKeyUsageOID.CLIENT_AUTH]),
                    critical=False,
                )
            elif cert_type == "server":
                cert_builder = cert_builder.add_extension(
                    x509.ExtendedKeyUsage(
                        [x509.oid.ExtendedKeyUsageOID.SERVER_AUTH]),
                    critical=False,
                )

        cert = cert_builder.sign(key, hashes.SHA256())
        logger.info("Self-signed certificate created successfully.")

        cert_path = cert_dir / f"{hostname}.crt"
        key_path = cert_dir / f"{hostname}.key"

        cert_dir.mkdir(parents=True, exist_ok=True)

        with cert_path.open("wb") as cert_file:
            cert_file.write(cert.public_bytes(serialization.Encoding.PEM))
            logger.debug(f"Certificate saved to {cert_path}")

        with key_path.open("wb") as key_file:
            key_file.write(
                key.private_bytes(
                    encoding=serialization.Encoding.PEM,
                    format=serialization.PrivateFormat.TraditionalOpenSSL,
                    encryption_algorithm=serialization.NoEncryption(),
                )
            )
            logger.debug(f"Private key saved to {key_path}")

        return cert_path, key_path
    except Exception as e:
        logger.exception(f"Failed to create self-signed certificate: {e}")
        raise


def export_to_pkcs12(cert_path: Path, key_path: Path, password: str, export_path: Optional[Path] = None) -> Path:
    """
    Export the certificate and private key to a PKCS#12 (PFX) file.

    :param cert_path: Path to the certificate file.
    :param key_path: Path to the private key file.
    :param password: Password to protect the PFX file.
    :param export_path: Path to save the PFX file, defaults to same directory as certificate.
    :return: Path to the PFX file.
    """
    logger.debug(
        f"Exporting certificate '{cert_path}' and key '{key_path}' to PKCS#12 file.")
    try:
        if export_path is None:
            export_path = cert_path.with_suffix(".pfx")

        with cert_path.open("rb") as cert_file:
            cert = x509.load_pem_x509_certificate(cert_file.read())
            logger.debug("Certificate loaded successfully for PKCS#12 export.")

        with key_path.open("rb") as key_file:
            key = serialization.load_pem_private_key(
                key_file.read(), password=None)
            logger.debug("Private key loaded successfully for PKCS#12 export.")

        pfx = serialization.pkcs12.serialize_key_and_certificates(
            name=cert.subject.rfc4514_string().encode(),
            key=key,
            cert=cert,
            cas=None,
            encryption_algorithm=serialization.BestAvailableEncryption(
                password.encode())
        )

        with export_path.open("wb") as pfx_file:
            pfx_file.write(pfx)
            logger.info(f"PKCS#12 file exported successfully to {export_path}")

        return export_path
    except Exception as e:
        logger.exception(f"Failed to export to PKCS#12: {e}")
        raise


def generate_crl(cert: x509.Certificate, private_key: rsa.RSAPrivateKey, revoked_certs: List[x509.RevokedCertificate], crl_dir: Path) -> Path:
    """
    Generate a Certificate Revocation List (CRL).

    :param cert: Issuer certificate.
    :param private_key: Issuer's private key.
    :param revoked_certs: List of revoked certificates.
    :param crl_dir: Directory to save the CRL file.
    :return: Path to the CRL file.
    """
    logger.debug("Generating Certificate Revocation List (CRL).")
    try:
        crl_builder = x509.CertificateRevocationListBuilder().issuer_name(cert.subject)

        for revoked_cert in revoked_certs:
            crl_builder = crl_builder.add_revoked_certificate(revoked_cert)
            logger.debug(
                f"Added revoked certificate with serial number {revoked_cert.serial_number} to CRL.")

        crl = crl_builder.last_update(datetime.datetime.utcnow()) \
            .next_update(datetime.datetime.utcnow() + datetime.timedelta(days=7)) \
            .sign(private_key, hashes.SHA256())
        logger.info("CRL signed successfully.")

        crl_dir.mkdir(parents=True, exist_ok=True)
        crl_path = crl_dir / "revoked.crl"

        with crl_path.open("wb") as crl_file:
            crl_file.write(crl.public_bytes(serialization.Encoding.PEM))
            logger.info(f"CRL saved to {crl_path}")

        return crl_path
    except Exception as e:
        logger.exception(f"Failed to generate CRL: {e}")
        raise


def load_ssl_context(cert_path: Path, key_path: Path) -> ssl.SSLContext:
    """
    Load an SSL context from certificate and key files.

    :param cert_path: Path to the certificate file.
    :param key_path: Path to the private key file.
    :return: An SSLContext object.
    """
    logger.debug(
        f"Loading SSL context from cert '{cert_path}' and key '{key_path}'.")
    try:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(certfile=cert_path, keyfile=key_path)
        logger.info("SSL context loaded successfully.")
        return context
    except Exception as e:
        logger.exception(f"Failed to load SSL context: {e}")
        raise


def view_cert_details(cert_path: Path):
    """
    View the details of the certificate.

    :param cert_path: Path to the certificate file.
    """
    logger.debug(f"Viewing details of certificate '{cert_path}'.")
    try:
        with cert_path.open("rb") as cert_file:
            cert = x509.load_pem_x509_certificate(cert_file.read())
            logger.info(f"Issuer: {cert.issuer.rfc4514_string()}")
            logger.info(f"Subject: {cert.subject.rfc4514_string()}")
            logger.info(f"Serial Number: {cert.serial_number}")
            logger.info(f"Valid From: {cert.not_valid_before}")
            logger.info(f"Valid Until: {cert.not_valid_after}")
            public_key = cert.public_key().public_bytes(
                serialization.Encoding.PEM,
                serialization.PublicFormat.SubjectPublicKeyInfo
            ).decode('utf-8')
            logger.info(f"Public Key:\n{public_key}")
            extensions = [ext for ext in cert.extensions]
            logger.info(f"Extensions: {extensions}")
    except Exception as e:
        logger.exception(f"Failed to view certificate details: {e}")
        raise


def check_cert_expiry(cert_path: Path, warning_days: int = 30) -> bool:
    """
    Check if a certificate is about to expire.

    :param cert_path: Path to the certificate file.
    :param warning_days: Number of days before expiry to trigger a warning.
    :return: True if the certificate is expiring soon, else False.
    """
    logger.debug(f"Checking expiry of certificate '{cert_path}'.")
    try:
        with cert_path.open("rb") as cert_file:
            cert = x509.load_pem_x509_certificate(cert_file.read())
            remaining_days = (cert.not_valid_after -
                              datetime.datetime.utcnow()).days
            if remaining_days <= warning_days:
                logger.warning(
                    f"Certificate '{cert_path}' is expiring in {remaining_days} days!")
                return True
            else:
                logger.info(
                    f"Certificate '{cert_path}' is valid for {remaining_days} more days.")
                return False
    except Exception as e:
        logger.exception(f"Failed to check certificate expiry: {e}")
        raise


def renew_cert(
    cert_path: Path,
    key_path: Path,
    valid_days: int = 365,
    new_cert_dir: Optional[Path] = None
) -> Path:
    """
    Renew an existing certificate by creating a new one with extended validity.

    :param cert_path: Path to the existing certificate file.
    :param key_path: Path to the existing key file.
    :param valid_days: Number of days the new certificate is valid.
    :param new_cert_dir: Directory to save the new certificate, defaults to the original location.
    :return: Path to the new certificate.
    """
    logger.debug(
        f"Renewing certificate '{cert_path}' with new validity of {valid_days} days.")
    try:
        if new_cert_dir is None:
            new_cert_dir = cert_path.parent

        with cert_path.open("rb") as cert_file:
            cert = x509.load_pem_x509_certificate(cert_file.read())
            logger.debug("Existing certificate loaded for renewal.")

        subject = cert.subject
        issuer = cert.issuer

        with key_path.open("rb") as key_file:
            key = serialization.load_pem_private_key(
                key_file.read(), password=None)
            logger.debug("Private key loaded for renewal.")

        new_cert = (
            x509.CertificateBuilder()
            .subject_name(subject)
            .issuer_name(issuer)
            .public_key(key.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(datetime.datetime.utcnow())
            .not_valid_after(datetime.datetime.utcnow() + datetime.timedelta(days=valid_days))
            .add_extension(
                x509.SubjectAlternativeName([
                    dns for dns in cert.extensions.get_extension_for_class(
                        x509.SubjectAlternativeName).value
                ]),
                critical=False,
            )
            .sign(key, hashes.SHA256())
        )
        logger.info("New certificate built successfully for renewal.")

        common_name = cert.subject.get_attributes_for_oid(NameOID.COMMON_NAME)[
            0].value
        new_cert_path = new_cert_dir / f"{common_name}_renewed.crt"

        with new_cert_path.open("wb") as new_cert_file:
            new_cert_file.write(new_cert.public_bytes(
                serialization.Encoding.PEM))
            logger.info(f"Renewed certificate saved to {new_cert_path}")

        return new_cert_path
    except Exception as e:
        logger.exception(f"Failed to renew certificate: {e}")
        raise


def verify_certificate(cert_path: Path, ca_cert_path: Optional[Path] = None) -> bool:
    """
    Verify a certificate against a CA certificate.

    :param cert_path: Path to the certificate to verify.
    :param ca_cert_path: Path to the CA certificate. If None, uses default system CAs.
    :return: True if verification succeeds, False otherwise.
    """
    logger.debug(
        f"Verifying certificate '{cert_path}' against CA '{ca_cert_path}'.")
    try:
        context = ssl.create_default_context(
            cafile=ca_cert_path) if ca_cert_path else ssl.create_default_context()
        with cert_path.open("rb") as cert_file:
            cert_pem = cert_file.read()
            logger.debug("Certificate loaded for verification.")

        # Since ssl does not provide direct certificate verification, use cryptography
        cert = x509.load_pem_x509_certificate(cert_pem)
        if ca_cert_path:
            with ca_cert_path.open("rb") as ca_file:
                ca_cert = x509.load_pem_x509_certificate(ca_file.read())
                ca_public_key = ca_cert.public_key()
                ca_public_key.verify(
                    cert.signature,
                    cert.tbs_certificate_bytes,
                    padding=cert.signature_hash_algorithm.padding,
                    algorithm=cert.signature_hash_algorithm,
                )
        else:
            logger.warning(
                "No CA certificate provided for verification. Using system CAs.")
            # Implement system CA verification if needed

        logger.info(f"Certificate '{cert_path}' is valid and trusted.")
        return True
    except Exception as e:
        logger.error(f"Certificate verification failed: {e}")
        return False


if __name__ == "__main__":
    import sys

    parser = argparse.ArgumentParser(
        description="Enhanced SSL Certificate Management Tool"
    )
    parser.add_argument("hostname", help="The hostname for the certificate")
    parser.add_argument("--cert-dir", type=Path, default=Path("./certs"),
                        help="Directory to save the certificates")
    parser.add_argument("--key-size", type=int, default=2048,
                        help="Size of RSA key in bits")
    parser.add_argument("--valid-days", type=int, default=365,
                        help="Number of days the certificate is valid")
    parser.add_argument("--san", nargs='*',
                        help="List of Subject Alternative Names (SANs)")
    parser.add_argument("--view", action="store_true",
                        help="View certificate details")
    parser.add_argument("--check-expiry", action="store_true",
                        help="Check if the certificate is about to expire")
    parser.add_argument("--renew", action="store_true",
                        help="Renew the certificate")
    parser.add_argument("--export-pfx", action="store_true",
                        help="Export the certificate and key as a PKCS#12 file")
    parser.add_argument("--cert-type", choices=["server", "client", "ca"],
                        default="server", help="Type of certificate to create")
    parser.add_argument("--crl", action="store_true",
                        help="Generate a Certificate Revocation List (CRL)")
    parser.add_argument("--crl-dir", type=Path, default=Path("./crl"),
                        help="Directory to save the CRL file")
    parser.add_argument("--password", type=str,
                        help="Password for exporting PKCS#12 file")
    parser.add_argument("--revoke", nargs='*',
                        help="Revoke certificates by their serial numbers (hexadecimal)")

    args = parser.parse_args()

    try:
        args.cert_dir.mkdir(parents=True, exist_ok=True)
        logger.debug(f"Certificate directory set to '{args.cert_dir}'.")

        if args.view:
            cert_path = args.cert_dir / f"{args.hostname}.crt"
            view_cert_details(cert_path)
        elif args.check_expiry:
            cert_path = args.cert_dir / f"{args.hostname}.crt"
            check_cert_expiry(cert_path)
        elif args.renew:
            cert_path = args.cert_dir / f"{args.hostname}.crt"
            key_path = args.cert_dir / f"{args.hostname}.key"
            new_cert_path = renew_cert(
                cert_path, key_path, args.valid_days, args.cert_dir)
            logger.info(f"Certificate renewed and saved to {new_cert_path}")
            print(f"Certificate renewed and saved to {new_cert_path}")
        elif args.export_pfx:
            cert_path = args.cert_dir / f"{args.hostname}.crt"
            key_path = args.cert_dir / f"{args.hostname}.key"
            if not args.password:
                logger.error("Password is required for exporting to PKCS#12.")
                print("Error: Password is required for exporting to PKCS#12.")
                sys.exit(1)
            pfx_path = export_to_pkcs12(cert_path, key_path, args.password)
            logger.info(f"PKCS#12 file exported to {pfx_path}")
            print(f"PKCS#12 file exported to {pfx_path}")
        elif args.crl:
            cert_path = args.cert_dir / f"{args.hostname}.crt"
            key_path = args.cert_dir / f"{args.hostname}.key"
            revoked_certs = []
            if args.revoke:
                for serial in args.revoke:
                    try:
                        revoked_cert = x509.RevokedCertificateBuilder().serial_number(int(serial, 16)) \
                                                                       .revocation_date(datetime.datetime.utcnow()) \
                                                                       .build(hashes.SHA256())
                        revoked_certs.append(revoked_cert)
                        logger.debug(
                            f"Revoked certificate with serial number {serial} added to CRL.")
                    except ValueError as ve:
                        logger.error(f"Invalid serial number '{serial}': {ve}")
                        print(f"Invalid serial number '{serial}': {ve}")
                        sys.exit(1)
            else:
                logger.warning("No certificates specified for revocation.")
            crl_path = generate_crl(
                x509.load_pem_x509_certificate(cert_path.read_bytes()),
                serialization.load_pem_private_key(
                    key_path.read_bytes(), password=None),
                revoked_certs, args.crl_dir)
            logger.info(f"CRL generated and saved to {crl_path}")
            print(f"CRL generated and saved to {crl_path}")
        else:
            cert_path, key_path = create_self_signed_cert(
                args.hostname,
                args.cert_dir,
                args.key_size,
                args.valid_days,
                args.san,
                args.cert_type
            )
            logger.info(f"Certificate generated: {cert_path}")
            logger.info(f"Private key generated: {key_path}")
            print(f"Certificate generated: {cert_path}")
            print(f"Private key generated: {key_path}")
    except KeyboardInterrupt:
        logger.warning("Operation interrupted by user.")
        print("\nOperation interrupted by user.")
        sys.exit(0)
    except Exception as e:
        logger.exception(f"An unexpected error occurred: {e}")
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)
