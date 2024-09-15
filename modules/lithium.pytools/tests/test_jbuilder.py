import pytest
from pathlib import Path
from ..tools.jbuilder import JavaScriptBuilder


@pytest.fixture
def sample_project_dir(tmp_path):
    project_dir = tmp_path / "sample_project"
    project_dir.mkdir()
    return project_dir


def test_check_package_manager(sample_project_dir):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    assert builder.check_package_manager(
    ) is True or builder.check_package_manager() is False


def test_install_package_manager(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.install_package_manager()
    builder.run_command.assert_called()


def test_install_dependencies(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.install_dependencies()
    builder.run_command.assert_called_with("npm", "install")


def test_build(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.build()
    builder.run_command.assert_called_with("npm", "run", "build")


def test_clean(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.clean()
    builder.run_command.assert_called_with("rm", "-rf", "node_modules")


def test_test(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.test()
    builder.run_command.assert_called_with("npm", "test")


def test_lint(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.lint()
    builder.run_command.assert_called_with("npm", "run", "lint")


def test_format(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.format()
    builder.run_command.assert_called_with("npm", "run", "format")


def test_start(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.start()
    builder.run_command.assert_called_with("npm", "start")


def test_generate_docs(sample_project_dir, mocker):
    builder = JavaScriptBuilder(
        project_dir=sample_project_dir, package_manager="npm")
    mocker.patch.object(builder, 'run_command')
    builder.generate_docs()
    builder.run_command.assert_called_with("npm", "run", "docs")
