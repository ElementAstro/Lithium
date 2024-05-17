import subprocess
import sys
from pathlib import Path
from typing import Literal, List, Optional


class CMakeBuilder:
    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        generator: Literal["Ninja", "Unix Makefiles"] = "Ninja",
        build_type: Literal["Debug", "Release"] = "Debug",
        install_prefix: Path = None,
        cmake_options: Optional[List[str]] = None,
    ):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.generator = generator
        self.build_type = build_type
        self.install_prefix = install_prefix or build_dir / "install"
        self.cmake_options = cmake_options or []

    def run_command(self, *cmd: str):
        """Helper function to run shell commands."""
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)

    def configure(self):
        """Configure the CMake build system."""
        self.build_dir.mkdir(parents=True, exist_ok=True)
        cmake_args = [
            "cmake",
            f"-G{self.generator}",
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
            f"-DCMAKE_INSTALL_PREFIX={self.install_prefix}",
            str(self.source_dir),
        ] + self.cmake_options
        self.run_command(*cmake_args)

    def build(self, target: str = ""):
        """Build the project."""
        build_cmd = ["cmake", "--build", str(self.build_dir)]
        if target:
            build_cmd += ["--target", target]
        self.run_command(*build_cmd)

    def install(self):
        """Install the project."""
        self.run_command("cmake", "--install", str(self.build_dir))

    def clean(self):
        """Clean the build directory."""
        if self.build_dir.exists():
            for item in self.build_dir.iterdir():
                if item.is_dir():
                    self.run_command("rm", "-rf", str(item))
                else:
                    item.unlink()

    def test(self):
        """Run CTest for the project."""
        self.run_command("ctest", "--output-on-failure", "-C", self.build_type, "-S", str(self.build_dir))

    def generate_docs(self, doc_target: str = "doc"):
        """Generate documentation if documentation target is available."""
        self.build(doc_target)

class MesonBuilder:
    def __init__(
        self,
        source_dir: Path,
        build_dir: Path,
        build_type: Literal["debug", "release"] = "debug",
        install_prefix: Path = None,
        meson_options: Optional[List[str]] = None,
    ):
        self.source_dir = source_dir
        self.build_dir = build_dir
        self.build_type = build_type
        self.install_prefix = install_prefix or build_dir / "install"
        self.meson_options = meson_options or []

    def run_command(self, *cmd: str):
        """Helper function to run shell commands."""
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)

    def configure(self):
        """Configure the Meson build system."""
        self.build_dir.mkdir(parents=True, exist_ok=True)
        meson_args = [
            "meson",
            "setup",
            str(self.build_dir),
            str(self.source_dir),
            f"--buildtype={self.build_type}",
            f"--prefix={self.install_prefix}",
        ] + self.meson_options
        self.run_command(*meson_args)

    def build(self, target: str = ""):
        """Build the project."""
        build_cmd = self.run_command("meson", "compile", "-C", str(self.build_dir))
        if target:
            build_cmd += ["--target", target]
        self.run_command(*build_cmd)

    def install(self):
        """Install the project."""
        self.run_command("meson", "install", "-C", str(self.build_dir))

    def clean(self):
        """Clean the build directory."""
        if self.build_dir.exists():
            for item in self.build_dir.iterdir():
                if item.is_dir():
                    self.run_command("rm", "-rf", str(item))
                else:
                    item.unlink()

def main():
    import argparse

    parser = argparse.ArgumentParser(description="CMake Python Builder")
    parser.add_argument(
        "--source_dir", type=Path, default=Path(".").resolve(), help="Source directory"
    )
    parser.add_argument(
        "--build_dir",
        type=Path,
        default=Path("build").resolve(),
        help="Build directory",
    )
    parser.add_argument(
        "--generator", choices=["Ninja", "Unix Makefiles"], default="Ninja"
    )
    parser.add_argument("--build_type", choices=["Debug", "Release"], default="Debug")
    parser.add_argument("--target", default="")
    parser.add_argument("--install", action="store_true", help="Install the project")
    parser.add_argument("--clean", action="store_true", help="Clean the build directory")
    parser.add_argument("--test", action="store_true", help="Run the tests")
    parser.add_argument(
        "--cmake_options",
        nargs="*",
        default=[],
        help="Custom CMake options (e.g. -DVAR=VALUE)",
    )
    parser.add_argument("--generate_docs", action="store_true", help="Generate documentation")

    args = parser.parse_args()

    builder = CMakeBuilder(
        source_dir=args.source_dir,
        build_dir=args.build_dir,
        generator=args.generator,
        build_type=args.build_type,
        cmake_options=args.cmake_options,
    )

    if args.clean:
        builder.clean()

    builder.configure()
    builder.build(args.target)

    if args.install:
        builder.install()

    if args.test:
        builder.test()

    if args.generate_docs:
        builder.generate_docs()


if __name__ == "__main__":
    main()
