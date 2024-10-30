import pytest
import sys
import os
import json
import subprocess
from unittest.mock import patch, MagicMock
from src.client.astrometry.client import main, Client, RequestError


@pytest.fixture
def mock_client():
    with patch('src.client.astrometry.client.Client') as MockClient:
        yield MockClient


@pytest.fixture
def mock_urlopen():
    with patch('src.client.astrometry.client.urlopen') as mock_urlopen:
        yield mock_urlopen


@pytest.fixture
def mock_request():
    with patch('src.client.astrometry.client.Request') as mock_request:
        yield mock_request


@pytest.fixture
def mock_subprocess_run():
    with patch('subprocess.run') as mock_run:
        yield mock_run


def test_main_login_success(mock_client):
    mock_instance = mock_client.return_value
    mock_instance.login.return_value = None

    test_args = ['client.py', '--apikey', 'test_api_key']
    with patch.object(sys, 'argv', test_args):
        main()

    mock_instance.login.assert_called_once_with('test_api_key')


def test_main_login_failure(mock_client):
    mock_instance = mock_client.return_value
    mock_instance.login.side_effect = RequestError('Login failed')

    test_args = ['client.py', '--apikey', 'test_api_key']
    with patch.object(sys, 'argv', test_args):
        with pytest.raises(SystemExit):
            main()

    mock_instance.login.assert_called_once_with('test_api_key')


def test_main_upload_file(mock_client, tmp_path):
    mock_instance = mock_client.return_value
    mock_instance.upload.return_value = {'status': 'success', 'subid': '12345'}

    test_file = tmp_path / "test.fits"
    test_file.write_text("dummy content")

    test_args = ['client.py', '--apikey',
                 'test_api_key', '--upload', str(test_file)]
    with patch.object(sys, 'argv', test_args):
        main()

    mock_instance.upload.assert_called_once()


def test_main_job_status(mock_client):
    mock_instance = mock_client.return_value
    mock_instance.job_status.return_value = 'success'

    test_args = ['client.py', '--apikey',
                 'test_api_key', '--jobstatus', '12345']
    with patch.object(sys, 'argv', test_args):
        main()

    mock_instance.job_status.assert_called_once_with('12345')


def test_main_download_files(mock_client, mock_urlopen, tmp_path):
    mock_instance = mock_client.return_value
    mock_instance.job_status.return_value = 'success'
    mock_instance.upload.return_value = {'status': 'success', 'subid': '12345'}
    mock_instance.sub_status.return_value = {'jobs': ['12345']}
    mock_instance.annotate_data.return_value = {'annotations': 'data'}

    test_file = tmp_path / "test.fits"
    test_file.write_text("dummy content")

    test_args = [
        'client.py', '--apikey', 'test_api_key', '--upload', str(test_file),
        '--wcs', str(tmp_path /
                     'wcs.fits'), '--annotate', str(tmp_path / 'annotations.json')
    ]
    with patch.object(sys, 'argv', test_args):
        main()

    mock_instance.upload.assert_called_once()
    mock_instance.sub_status.assert_called_once()
    mock_instance.job_status.assert_called_once()
    mock_instance.annotate_data.assert_called_once()


def test_main_sdss_plot(mock_client, tmp_path):
    mock_instance = mock_client.return_value

    test_args = ['client.py', '--apikey',
                 'test_api_key', '--sdss', 'wcs.fits', 'out.png']
    with patch.object(sys, 'argv', test_args):
        main()

    mock_instance.sdss_plot.assert_called_once_with('out.png', 'wcs.fits')


def test_main_galex_plot(mock_client, tmp_path):
    mock_instance = mock_client.return_value

    test_args = ['client.py', '--apikey',
                 'test_api_key', '--galex', 'wcs.fits', 'out.png']
    with patch.object(sys, 'argv', test_args):
        main()

    mock_instance.galex_plot.assert_called_once_with('out.png', 'wcs.fits')


def test_main_delete_job(mock_client):
    mock_instance = mock_client.return_value
    mock_instance.delete_job.return_value = {'status': 'success'}

    test_args = ['client.py', '--apikey',
                 'test_api_key', '--delete_job', '12345']
    with patch.object(sys, 'argv', test_args):
        main()

    mock_instance.delete_job.assert_called_once_with('12345')
