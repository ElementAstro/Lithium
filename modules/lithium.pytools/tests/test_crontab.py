import pytest
import os
from crontab import CrontabManager

# FILE: test/11.25/test_crontab.py


@pytest.fixture
def crontab_manager():
    manager = CrontabManager(crontab_path="/tmp/test_crontab")
    yield manager
    if os.path.exists(manager.crontab_path):
        os.remove(manager.crontab_path)

def test_list_jobs_empty(crontab_manager):
    jobs = crontab_manager.list_jobs()
    assert jobs == []

def test_add_job(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    jobs = crontab_manager.list_jobs()
    assert len(jobs) == 1
    assert jobs[0].strip() == "* * * * * echo 'Hello World'"
    crontab_manager.clear_jobs()

def test_remove_job(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    crontab_manager.remove_job("echo 'Hello World'")
    jobs = crontab_manager.list_jobs()
    assert jobs == []
    crontab_manager.clear_jobs()

def test_update_job(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    crontab_manager.update_job("echo 'Hello World'", "0 0 * * *", "echo 'Updated'")
    jobs = crontab_manager.list_jobs()
    assert len(jobs) == 1
    assert jobs[0].strip() == "0 0 * * * echo 'Updated'"
    crontab_manager.clear_jobs()

def test_clear_jobs(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    crontab_manager.clear_jobs()
    jobs = crontab_manager.list_jobs()
    assert jobs == []
    crontab_manager.clear_jobs()

def test_job_exists(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    assert crontab_manager.job_exists("echo 'Hello World'")
    assert not crontab_manager.job_exists("echo 'Nonexistent'")
    crontab_manager.clear_jobs()

def test_disable_job(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    crontab_manager.disable_job("echo 'Hello World'")
    jobs = crontab_manager.list_jobs()
    assert len(jobs) == 1
    assert jobs[0].strip() == "#* * * * * echo 'Hello World'"
    crontab_manager.clear_jobs()

def test_enable_job(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    crontab_manager.disable_job("echo 'Hello World'")
    crontab_manager.enable_job("echo 'Hello World'")
    jobs = crontab_manager.list_jobs()
    assert len(jobs) == 1
    assert jobs[0].strip() == "* * * * * echo 'Hello World'"
    crontab_manager.clear_jobs()

def test_search_jobs(crontab_manager):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    crontab_manager.add_job("0 0 * * *", "echo 'Goodbye World'")
    results = crontab_manager.search_jobs("Hello")
    assert len(results) == 1
    assert results[0].strip() == "* * * * * echo 'Hello World'"
    crontab_manager.clear_jobs()

def test_export_jobs(crontab_manager, tmp_path):
    crontab_manager.add_job("* * * * *", "echo 'Hello World'")
    export_path = tmp_path / "exported_crontab"
    crontab_manager.export_jobs(str(export_path))
    assert os.path.exists(export_path)
    with open(export_path, "r") as file:
        jobs = file.readlines()
    assert len(jobs) == 1
    assert jobs[0].strip() == "* * * * * echo 'Hello World'"
    crontab_manager.clear_jobs()

def test_import_jobs(crontab_manager, tmp_path):
    import_path = tmp_path / "imported_crontab"
    with open(import_path, "w") as file:
        file.write("* * * * * echo 'Hello World'\n")
    crontab_manager.import_jobs(str(import_path))
    jobs = crontab_manager.list_jobs()
    assert len(jobs) == 1
    assert jobs[0].strip() == "* * * * * echo 'Hello World'"
    crontab_manager.clear_jobs()

def test_view_logs(crontab_manager, tmp_path):
    log_path = tmp_path / "test_syslog"
    with open(log_path, "w") as file:
        file.write("Oct 1 00:00:00 myhost CRON[12345]: (root) CMD (echo 'Hello World')\n")
    logs = crontab_manager.view_logs(str(log_path))
    assert "CRON[12345]" in logs
    assert "echo 'Hello World'" in logs
    crontab_manager.clear_jobs()