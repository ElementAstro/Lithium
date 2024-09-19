# modules/lithium.pytools/tools/test_cbuilder.py

import pytest
from unittest.mock import patch, MagicMock
from pathlib import Path
import sys

# Import the main function from the cbuilder module
from tools.cbuilder import main

@pytest.fixture
def mock_args():
    return [
        "cbuilder.py",
        "--builder", "cmake",
        "--source_dir", "src",
        "--build_dir", "build",
        "--generator", "Ninja",
        "--build_type", "Debug",
        "--target", "all",
        "--install",
        "--clean",
        "--test",
        "--cmake_options", "-DVAR=VALUE",
        "--env", "VAR1=value1", "VAR2=value2",
        "--verbose",
        "--parallel", "4"
    ]

@pytest.fixture
def mock_meson_args():
    return [
        "cbuilder.py",
        "--builder", "meson",
        "--source_dir", "src",
        "--build_dir", "build",
        "--build_type", "debug",
        "--target", "all",
        "--install",
        "--clean",
        "--test",
        "--meson_options", "-Dvar=value",
        "--env", "VAR1=value1", "VAR2=value2",
        "--verbose",
        "--parallel", "4"
    ]

@patch("argparse.ArgumentParser.parse_args")
@patch("subprocess.run")
def test_main_cmake(mock_subprocess_run, mock_parse_args, mock_args):
    mock_parse_args.return_value = argparse.Namespace(
        source_dir=Path("src").resolve(),
        build_dir=Path("build").resolve(),
        builder="cmake",
        generator="Ninja",
        build_type="Debug",
        target="all",
        install=True,
        clean=True,
        test=True,
        cmake_options=["-DVAR=VALUE"],
        meson_options=[],
        env=["VAR1=value1", "VAR2=value2"],
        verbose=True,
        parallel=4,
        generate_docs=False
    )

    with patch("modules.lithium.pytools.tools.cbuilder.CMakeBuilder") as MockCMakeBuilder:
        mock_builder = MockCMakeBuilder.return_value
        mock_builder.configure = MagicMock()
        mock_builder.build = MagicMock()
        mock_builder.install = MagicMock()
        mock_builder.test = MagicMock()
        mock_builder.clean = MagicMock()

        main()

        mock_builder.clean.assert_called_once()
        mock_builder.configure.assert_called_once()
        mock_builder.build.assert_called_once_with("all")
        mock_builder.install.assert_called_once()
        mock_builder.test.assert_called_once()

@patch("argparse.ArgumentParser.parse_args")
@patch("subprocess.run")
def test_main_meson(mock_subprocess_run, mock_parse_args, mock_meson_args):
    mock_parse_args.return_value = argparse.Namespace(
        source_dir=Path("src").resolve(),
        build_dir=Path("build").resolve(),
        builder="meson",
        generator="Ninja",
        build_type="debug",
        target="all",
        install=True,
        clean=True,
        test=True,
        cmake_options=[],
        meson_options=["-Dvar=value"],
        env=["VAR1=value1", "VAR2=value2"],
        verbose=True,
        parallel=4,
        generate_docs=False
    )

    with patch("modules.lithium.pytools.tools.cbuilder.MesonBuilder") as MockMesonBuilder:
        mock_builder = MockMesonBuilder.return_value
        mock_builder.configure = MagicMock()
        mock_builder.build = MagicMock()
        mock_builder.install = MagicMock()
        mock_builder.test = MagicMock()
        mock_builder.clean = MagicMock()

        main()

        mock_builder.clean.assert_called_once()
        mock_builder.configure.assert_called_once()
        mock_builder.build.assert_called_once_with("all")
        mock_builder.install.assert_called_once()
        mock_builder.test.assert_called_once()

def test_env_parsing():
    env_vars = ["VAR1=value1", "VAR2=value2"]
    parsed_env = {var.split("=")[0]: var.split("=")[1] for var in env_vars}
    assert parsed_env == {"VAR1": "value1", "VAR2": "value2"}
