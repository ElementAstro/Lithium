import json
import pathlib
from setuptools import setup, find_packages

# Load project information from JSON file
project_json_path = pathlib.Path(__file__).parent / "project.json"
with project_json_path.open(encoding="utf-8") as f:
    project_info = json.load(f)

# Extract relevant information
name = project_info["name"]
version = project_info["version"]
description = project_info["description"]
license_type = project_info["license"]
author = project_info["author"]
repository_url = project_info["repository"]["url"]
keywords = project_info["keywords"]
dependencies = project_info["dependencies"]

# Convert dependencies from dict to list
install_requires = [f"{pkg}=={ver.lstrip('^')}" for pkg, ver in dependencies.items()]

# Setup function
setup(
    name=name,
    version=version,
    description=description,
    license=license_type,
    author=author,
    url=repository_url,
    packages=find_packages(exclude=["tests*"]),
    install_requires=install_requires,
    keywords=keywords,
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
)
