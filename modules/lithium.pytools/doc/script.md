# Project Management Script Documentation

This document provides a comprehensive guide on how to use the **Project Management Script**, a Python utility designed to manage Python projects through a JSON configuration file. The script can install dependencies, execute predefined scripts, clean temporary files, run tests, deploy the project, and generate project documentation.

---

## Key Features

- **Install Dependencies**: Installs project dependencies specified in a JSON configuration file.
- **Execute Scripts**: Runs predefined scripts from the configuration.
- **List Scripts**: Displays available scripts along with descriptions.
- **Clean Temporary Files**: Cleans temporary files and directories to free up space.
- **Run Tests**: Executes project tests to ensure code quality.
- **Deploy Project**: Deploys the project using specified commands.
- **Generate Documentation**: Creates project documentation using predefined commands.
- **Enhanced Logging**: Utilizes the `Loguru` library for detailed logging of operations and errors.
- **Rich Console Output**: Provides beautified terminal outputs using the `Rich` library.
- **Environment Variable Support**: Supports configuration via environment variables.

---

## Requirements

- Python 3.7 or higher.
- Required libraries: `loguru`, `rich`, `python-dotenv`.

Install the required libraries using pip:

```bash
pip install loguru rich python-dotenv
```

---

## Usage

The script can be executed from the command line with various options. The command syntax is as follows:

```bash
python project_manager.py <action> [options]
```

### Available Actions

1. **Build the Project**

   ```bash
   python project_manager.py build --config project.json
   ```

2. **Install Dependencies**

   ```bash
   python project_manager.py install --config project.json
   ```

3. **List Available Scripts**

   ```bash
   python project_manager.py --list --config project.json
   ```

4. **Clean Temporary Files**

   ```bash
   python project_manager.py clean --config project.json
   ```

5. **Run Tests**

   ```bash
   python project_manager.py test --config project.json
   ```

6. **Deploy the Project**

   ```bash
   python project_manager.py deploy --config project.json
   ```

7. **Generate Documentation**

   ```bash
   python project_manager.py docs --config project.json
   ```

### Command-Line Options

- **`--config`**: Path to the JSON configuration file (default: `project.json`).
- **`--env`**: Path to the environment variables file (default: `.env`).

---

## Example Usage

### Install Dependencies

To install dependencies specified in `project.json`:

```bash
python project_manager.py install --config project.json
```

### Execute Build Script

To build the project using the specified build script:

```bash
python project_manager.py build --config project.json
```

### List Available Scripts

To list all available scripts defined in the configuration:

```bash
python project_manager.py --list --config project.json
```

### Clean Temporary Files

To clean temporary files and directories:

```bash
python project_manager.py clean --config project.json
```

### Run Tests

To run tests defined in the configuration:

```bash
python project_manager.py test --config project.json
```

### Deploy the Project

To deploy the project using the specified deploy script:

```bash
python project_manager.py deploy --config project.json
```

### Generate Documentation

To generate project documentation:

```bash
python project_manager.py docs --config project.json
```

---

## Error Handling and Logging

The script uses the `Loguru` library for logging. Logs are written to `project_manager.log` and the console, providing detailed information about operations, warnings, and errors. This helps in tracking the actions performed by the script and diagnosing issues.

---

## Conclusion

The **Project Management Script** is a versatile utility for managing Python projects. It simplifies the process of handling dependencies, executing scripts, and performing project maintenance tasks while providing robust error handling and logging capabilities. By following this documentation, users can effectively utilize the tool for their project management needs.
