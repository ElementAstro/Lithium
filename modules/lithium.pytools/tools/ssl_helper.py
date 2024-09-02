import ssl
import datetime
import argparse
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import rsa
from pathlib import Path
from typing import List, Optional


def create_key(key_size: int = 2048):
    """
    Generates an RSA private key.

    :param key_size: RSA key size in bits.
    :return: A private key object.
    """
    return rsa.generate_private_key(
        public_exponent=65537,
        key_size=key_size,
    )


def create_self_signed_cert(
    hostname: str,
    cert_dir: Path,
    key_size: int = 2048,
    valid_days: int = 365,
    san_list: Optional[List[str]] = None,
    cert_type: str = "server"
):
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

    cert_path = cert_dir / f"{hostname}.crt"
    key_path = cert_dir / f"{hostname}.key"

    with cert_path.open("wb") as cert_file:
        cert_file.write(cert.public_bytes(serialization.Encoding.PEM))

    with key_path.open("wb") as key_file:
        key_file.write(
            key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption(),
            )
        )

    return cert_path, key_path


def export_to_pkcs12(cert_path: Path, key_path: Path, password: str, export_path: Optional[Path] = None):
    """
    Export the certificate and private key to a PKCS#12 (PFX) file.

    :param cert_path: Path to the certificate file.
    :param key_path: Path to the private key file.
    :param password: Password to protect the PFX file.
    :param export_path: Path to save the PFX file, defaults to same directory as certificate.
    :return: Path to the PFX file.
    """
    if export_path is None:
        export_path = cert_path.with_suffix(".pfx")

    with cert_path.open("rb") as cert_file:
        cert = x509.load_pem_x509_certificate(cert_file.read())

    with key_path.open("rb") as key_file:
        key = serialization.load_pem_private_key(
            key_file.read(), password=None)

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

    return export_path


def generate_crl(cert: x509.Certificate, private_key: rsa.RSAPrivateKey, revoked_certs: List[x509.RevokedCertificate], crl_dir: Path):
    """
    Generate a Certificate Revocation List (CRL).

    :param cert: Issuer certificate.
    :param private_key: Issuer's private key.
    :param revoked_certs: List of revoked certificates.
    :param crl_dir: Directory to save the CRL file.
    :return: Path to the CRL file.
    """
    crl_builder = x509.CertificateRevocationListBuilder().issuer_name(cert.subject)

    for revoked_cert in revoked_certs:
        crl_builder = crl_builder.add_revoked_certificate(revoked_cert)

    crl = crl_builder.last_update(datetime.datetime.utcnow()) \
        .next_update(datetime.datetime.utcnow() + datetime.timedelta(days=7)) \
        .sign(private_key, hashes.SHA256())

    crl_path = crl_dir / "revoked.crl"

    with crl_path.open("wb") as crl_file:
        crl_file.write(crl.public_bytes(serialization.Encoding.PEM))

    return crl_path


def load_ssl_context(cert_path: Path, key_path: Path) -> ssl.SSLContext:
    """
    Load an SSL context from certificate and key files.

    :param cert_path: Path to the certificate file.
    :param key_path: Path to the private key file.
    :return: An SSLContext object.
    """
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile=cert_path, keyfile=key_path)
    return context


def view_cert_details(cert_path: Path):
    """
    View the details of the certificate.

    :param cert_path: Path to the certificate file.
    """
    with cert_path.open("rb") as cert_file:
        cert = x509.load_pem_x509_certificate(cert_file.read())
        print(f"Issuer: {cert.issuer.rfc4514_string()}")
        print(f"Subject: {cert.subject.rfc4514_string()}")
        print(f"Serial Number: {cert.serial_number}")
        print(f"Valid From: {cert.not_valid_before}")
        print(f"Valid Until: {cert.not_valid_after}")
        print(
            f"Public Key: {cert.public_key().public_bytes(serialization.Encoding.PEM, serialization.PublicFormat.SubjectPublicKeyInfo).decode('utf-8')}")
        print(f"Extensions: {[ext for ext in cert.extensions]}")


def check_cert_expiry(cert_path: Path, warning_days: int = 30):
    """
    Check if a certificate is about to expire.

    :param cert_path: Path to the certificate file.
    :param warning_days: Number of days before expiry to trigger a warning.
    :return: True if the certificate is expiring soon, else False.
    """
    with cert_path.open("rb") as cert_file:
        cert = x509.load_pem_x509_certificate(cert_file.read())
        remaining_days = (cert.not_valid_after -
                          datetime.datetime.utcnow()).days
        if remaining_days <= warning_days:
            print(
                f"Warning: Certificate is expiring in {remaining_days} days!")
            return True
        else:
            print(f"Certificate is valid for {remaining_days} more days.")
            return False


def renew_cert(
    cert_path: Path,
    key_path: Path,
    valid_days: int = 365,
    new_cert_dir: Optional[Path] = None
):
    """
    Renew an existing certificate by creating a new one with extended validity.

    :param cert_path: Path to the existing certificate file.
    :param key_path: Path to the existing key file.
    :param valid_days: Number of days the new certificate is valid.
    :param new_cert_dir: Directory to save the new certificate, defaults to the original location.
    :return: Path to the new certificate.
    """
    if new_cert_dir is None:
        new_cert_dir = cert_path.parent

    with cert_path.open("rb") as cert_file:
        cert = x509.load_pem_x509_certificate(cert_file.read())

    subject = cert.subject
    issuer = cert.issuer
    key = None

    with key_path.open("rb") as key_file:
        key = serialization.load_pem_private_key(
            key_file.read(), password=None)

    new_cert = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer)
        .public_key(key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(datetime.datetime.utcnow())
        .not_valid_after(datetime.datetime.utcnow() + datetime.timedelta(days=valid_days))
        .sign(key, hashes.SHA256())
    )

    new_cert_path = new_cert_dir / \
        f"{cert.subject.get_attributes_for_oid(NameOID.COMMON_NAME)[0].value}_renewed.crt"

    with new_cert_path.open("wb") as new_cert_file:
        new_cert_file.write(new_cert.public_bytes(serialization.Encoding.PEM))

    return new_cert_path


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="SSL Certificate Management Tool")
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
    parser.add_argument("--revoke", action="append",
                        help="Revoke certificates by their serial numbers")

    args = parser.parse_args()

    args.cert_dir.mkdir(exist_ok=True)

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
        print(f"Certificate renewed and saved to {new_cert_path}")
    elif args.export_pfx:
        cert_path = args.cert_dir / f"{args.hostname}.crt"
        key_path = args.cert_dir / f"{args.hostname}.key"
        if not args.password:
            raise ValueError("Password is required for exporting to PKCS#12")
        pfx_path = export_to_pkcs12(cert_path, key_path, args.password)
        print(f"PKCS#12 file exported to {pfx_path}")
    elif args.crl:
        cert_path = args.cert_dir / f"{args.hostname}.crt"
        key_path = args.cert_dir / f"{args.hostname}.key"
        revoked_certs = []
        if args.revoke:
            for serial in args.revoke:
                revoked_cert = x509.RevokedCertificateBuilder().serial_number(int(serial, 16)) \
                                                               .revocation_date(datetime.datetime.utcnow()) \
                                                               .build(hashes.SHA256())
                revoked_certs.append(revoked_cert)
        crl_path = generate_crl(x509.load_pem_x509_certificate(cert_path.read_bytes()),
                                serialization.load_pem_private_key(
                                    key_path.read_bytes(), password=None),
                                revoked_certs, args.crl_dir)
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
        print(f"Certificate generated: {cert_path}")
        print(f"Private key generated: {key_path}")
