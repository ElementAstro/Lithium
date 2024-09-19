import pytest
from pathlib import Path
from tools.ssl_helper import (
    create_key,
    create_self_signed_cert,
    export_to_pkcs12,
    generate_crl,
    load_ssl_context,
    view_cert_details,
    check_cert_expiry,
    renew_cert
)
from cryptography import x509
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
import datetime


@pytest.fixture
def tmp_cert_dir(tmp_path):
    return tmp_path / "certs"


@pytest.fixture
def hostname():
    return "example.com"


def test_create_key():
    key = create_key()
    assert isinstance(key, rsa.RSAPrivateKey)


def test_create_self_signed_cert(tmp_cert_dir, hostname):
    tmp_cert_dir.mkdir()
    cert_path, key_path = create_self_signed_cert(
        hostname=hostname,
        cert_dir=tmp_cert_dir,
        key_size=2048,
        valid_days=365,
        san_list=None,
        cert_type="server"
    )
    assert cert_path.exists()
    assert key_path.exists()


def test_export_to_pkcs12(tmp_cert_dir, hostname):
    tmp_cert_dir.mkdir()
    cert_path, key_path = create_self_signed_cert(
        hostname=hostname,
        cert_dir=tmp_cert_dir,
        key_size=2048,
        valid_days=365,
        san_list=None,
        cert_type="server"
    )
    pfx_path = export_to_pkcs12(cert_path, key_path, "password")
    assert pfx_path.exists()


def test_generate_crl(tmp_cert_dir, hostname):
    tmp_cert_dir.mkdir()
    cert_path, key_path = create_self_signed_cert(
        hostname=hostname,
        cert_dir=tmp_cert_dir,
        key_size=2048,
        valid_days=365,
        san_list=None,
        cert_type="ca"
    )
    cert = x509.load_pem_x509_certificate(cert_path.read_bytes())
    key = serialization.load_pem_private_key(
        key_path.read_bytes(), password=None)
    revoked_cert = x509.RevokedCertificateBuilder().serial_number(
        123456789).revocation_date(datetime.datetime.utcnow()).build()
    crl_path = generate_crl(cert, key, [revoked_cert], tmp_cert_dir)
    assert crl_path.exists()


def test_load_ssl_context(tmp_cert_dir, hostname):
    tmp_cert_dir.mkdir()
    cert_path, key_path = create_self_signed_cert(
        hostname=hostname,
        cert_dir=tmp_cert_dir,
        key_size=2048,
        valid_days=365,
        san_list=None,
        cert_type="server"
    )
    context = load_ssl_context(cert_path, key_path)
    assert isinstance(context, ssl.SSLContext)


def test_view_cert_details(tmp_cert_dir, hostname, capsys):
    tmp_cert_dir.mkdir()
    cert_path, key_path = create_self_signed_cert(
        hostname=hostname,
        cert_dir=tmp_cert_dir,
        key_size=2048,
        valid_days=365,
        san_list=None,
        cert_type="server"
    )
    view_cert_details(cert_path)
    captured = capsys.readouterr()
    assert "Issuer" in captured.out
    assert "Subject" in captured.out


def test_check_cert_expiry(tmp_cert_dir, hostname, capsys):
    tmp_cert_dir.mkdir()
    cert_path, key_path = create_self_signed_cert(
        hostname=hostname,
        cert_dir=tmp_cert_dir,
        key_size=2048,
        valid_days=1,
        san_list=None,
        cert_type="server"
    )
    check_cert_expiry(cert_path, warning_days=2)
    captured = capsys.readouterr()
    assert "Warning: Certificate is expiring in" in captured.out


def test_renew_cert(tmp_cert_dir, hostname):
    tmp_cert_dir.mkdir()
    cert_path, key_path = create_self_signed_cert(
        hostname=hostname,
        cert_dir=tmp_cert_dir,
        key_size=2048,
        valid_days=1,
        san_list=None,
        cert_type="server"
    )
    new_cert_path = renew_cert(
        cert_path, key_path, valid_days=365, new_cert_dir=tmp_cert_dir)
    assert new_cert_path.exists()
