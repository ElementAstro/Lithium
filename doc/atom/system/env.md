# Atom Utils Environment Management Library Documentation

This document provides a detailed explanation of the `Env` class available in the `atom::utils` namespace for managing program environment variables, command-line arguments, and other related information.

## Table of Contents

1. [Env Class](#env-class)
   - [Constructors](#constructors)
   - [Static Methods](#static-methods)
   - [Instance Methods](#instance-methods)
2. [Usage Examples](#usage-examples)
3. [Best Practices and Tips](#best-practices-and-tips)

## Env Class

The `Env` class provides functionality for managing environment variables, command-line arguments, and other program-related information.

### Constructors

#### Default Constructor

```cpp
Env();
```

Initializes environment variable information.

#### Constructor with Command-line Arguments

```cpp
explicit Env(int argc, char** argv);
```

Initializes environment variable information with command-line arguments.

- `argc`: Number of command-line arguments.
- `argv`: Array of command-line arguments.

### Static Methods

#### createShared

```cpp
static auto createShared(int argc, char** argv) -> std::shared_ptr<Env>;
```

Creates a shared pointer to an `Env` object.

- `argc`: Number of command-line arguments.
- `argv`: Array of command-line arguments.
- Returns: Shared pointer to an `Env` object.

#### Environ

```cpp
static auto Environ() -> std::unordered_map<std::string, std::string>;
```

Gets the current environment variables.

- Returns: Unordered map of environment variables.

#### setVariable

```cpp
static void setVariable(const std::string& name, const std::string& value, bool overwrite = true);
```

Sets an environment variable.

- `name`: The variable name.
- `value`: The variable value.
- `overwrite`: Whether to overwrite the variable if it already exists (default: true).

#### getVariable

```cpp
static auto getVariable(const std::string& name) -> std::string;
```

Gets the value of an environment variable.

- `name`: The variable name.
- Returns: The variable value.

#### unsetVariable

```cpp
static void unsetVariable(const std::string& name);
```

Unsets (deletes) an environment variable.

- `name`: The variable name.

#### listVariables

```cpp
static auto listVariables() -> std::vector<std::string>;
```

Lists all environment variables.

- Returns: A vector of environment variable names.

#### printAllVariables

```cpp
static void printAllVariables();
```

Prints all environment variables.

### Instance Methods

#### add

```cpp
void add(const std::string& key, const std::string& val);
```

Adds a key-value pair to the environment variables.

- `key`: The key name.
- `val`: The value associated with the key.

#### has

```cpp
bool has(const std::string& key);
```

Checks if a key exists in the environment variables.

- `key`: The key name.
- Returns: True if the key exists, otherwise false.

#### del

```cpp
void del(const std::string& key);
```

Deletes a key-value pair from the environment variables.

- `key`: The key name.

#### get

```cpp
ATOM_NODISCARD auto get(const std::string& key, const std::string& default_value = "") -> std::string;
```

Gets the value associated with a key, or returns a default value if the key does not exist.

- `key`: The key name.
- `default_value`: The default value to return if the key does not exist (default: "").
- Returns: The value associated with the key, or the default value.

#### addHelp

```cpp
void addHelp(const std::string& key, const std::string& desc);
```

Adds a command-line argument and its description to the help information list.

- `key`: The argument name.
- `desc`: The argument description.

#### removeHelp

```cpp
void removeHelp(const std::string& key);
```

Removes a command-line argument from the help information list.

- `key`: The argument name.

#### printHelp

```cpp
void printHelp();
```

Prints the program's help information, including all added command-line arguments and their descriptions.

#### setEnv

```cpp
auto setEnv(const std::string& key, const std::string& val) -> bool;
```

Sets the value of an environment variable.

- `key`: The key name.
- `val`: The value to set.
- Returns: True if the environment variable was set successfully, otherwise false.

#### getEnv

```cpp
ATOM_NODISCARD auto getEnv(const std::string& key, const std::string& default_value = "") -> std::string;
```

Gets the value of an environment variable, or returns a default value if the variable does not exist.

- `key`: The key name.
- `default_value`: The default value to return if the variable does not exist (default: "").
- Returns: The value of the environment variable, or the default value.

#### getAbsolutePath

```cpp
ATOM_NODISCARD auto getAbsolutePath(const std::string& path) const -> std::string;
```

Gets the absolute path of a given path.

- `path`: The path to convert to an absolute path.
- Returns: The absolute path.

#### getAbsoluteWorkPath

```cpp
ATOM_NODISCARD auto getAbsoluteWorkPath(const std::string& path) const -> std::string;
```

Gets the absolute path of a given path relative to the working directory.

- `path`: The path to convert to an absolute path relative to the working directory.
- Returns: The absolute path.

#### getConfigPath

```cpp
ATOM_NODISCARD auto getConfigPath() -> std::string;
```

Gets the path of the configuration file. By default, the configuration file is in the same directory as the program.

- Returns: The configuration file path.

## Usage Examples

### Creating an Env Object

```cpp
int main(int argc, char** argv) {
    atom::utils::Env env(argc, argv);
    // Or using createShared
    auto env_ptr = atom::utils::Env::createShared(argc, argv);

    // Rest of the program...
}
```

### Managing Environment Variables

```cpp
atom::utils::Env env;

// Adding a variable
env.add("MY_VAR", "my_value");

// Checking if a variable exists
if (env.has("MY_VAR")) {
    std::cout << "MY_VAR exists" << std::endl;
}

// Getting a variable value
std::string value = env.get("MY_VAR", "default_value");
std::cout << "MY_VAR value: " << value << std::endl;

// Deleting a variable
env.del("MY_VAR");

// Setting an environment variable
env.setEnv("GLOBAL_VAR", "global_value");

// Getting an environment variable
std::string global_value = env.getEnv("GLOBAL_VAR", "default_global");
std::cout << "GLOBAL_VAR value: " << global_value << std::endl;
```

### Managing Help Information

```cpp
atom::utils::Env env;

env.addHelp("--verbose", "Enable verbose output");
env.addHelp("--config", "Specify configuration file path");

// Print help information
env.printHelp();

// Remove help information
env.removeHelp("--verbose");
```

### Working with Paths

```cpp
atom::utils::Env env;

std::string rel_path = "config/settings.json";
std::string abs_path = env.getAbsolutePath(rel_path);
std::cout << "Absolute path: " << abs_path << std::endl;

std::string work_rel_path = "data/input.txt";
std::string work_abs_path = env.getAbsoluteWorkPath(work_rel_path);
std::cout << "Absolute work path: " << work_abs_path << std::endl;

std::string config_path = env.getConfigPath();
std::cout << "Configuration file path: " << config_path << std::endl;
```

### Using Static Methods

```cpp
// Set an environment variable
atom::utils::Env::setVariable("APP_MODE", "production", true);

// Get an environment variable
std::string app_mode = atom::utils::Env::getVariable("APP_MODE");
std::cout << "Application mode: " << app_mode << std::endl;

// Unset an environment variable
atom::utils::Env::unsetVariable("TEMP_VAR");

// List all environment variables
std::vector<std::string> all_vars = atom::utils::Env::listVariables();
for (const auto& var : all_vars) {
    std::cout << var << std::endl;
}

// Print all environment variables
atom::utils::Env::printAllVariables();
```

### Thread-Safe Operations

The `Env` class uses a mutex to protect its member variables, making it safe to use in multi-threaded environments. Here's an example of how you might use it in a multi-threaded context:

```cpp
#include <thread>
#include <vector>

void worker_function(atom::utils::Env& env, const std::string& key, const std::string& value) {
    env.add(key, value);
    std::string retrieved_value = env.get(key);
    std::cout << "Thread " << std::this_thread::get_id() << ": " << key << " = " << retrieved_value << std::endl;
}

int main() {
    atom::utils::Env env;

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(worker_function, std::ref(env), "KEY_" + std::to_string(i), "VALUE_" + std::to_string(i));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
```

## Best Practices and Tips

1. **Initialization**: Always initialize the `Env` object at the beginning of your program, preferably in the `main` function, to ensure all environment variables and command-line arguments are properly set up.

   ```cpp
   int main(int argc, char** argv) {
       atom::utils::Env env(argc, argv);
       // Rest of the program...
   }
   ```

2. **Error Handling**: When using methods that return values, always check for empty or default values to handle cases where the requested key doesn't exist.

   ```cpp
   std::string value = env.get("IMPORTANT_VAR");
   if (value.empty()) {
       std::cerr << "IMPORTANT_VAR is not set!" << std::endl;
       // Handle the error...
   }
   ```

3. **Configuration Management**: Use the `getConfigPath()` method to locate your application's configuration file consistently across different environments.

   ```cpp
   std::string config_path = env.getConfigPath();
   // Use config_path to load your configuration file
   ```

4. **Path Handling**: Always use `getAbsolutePath()` or `getAbsoluteWorkPath()` when working with file paths to ensure consistency across different working directories.

   ```cpp
   std::string data_file = env.getAbsoluteWorkPath("data/input.csv");
   // Use data_file to open and process your input
   ```

5. **Help Information**: Maintain clear and concise help information for all command-line arguments to improve user experience.

   ```cpp
   env.addHelp("--log-level", "Set the logging level (debug, info, warn, error)");
   env.addHelp("--output-dir", "Specify the output directory for generated files");
   ```

6. **Environment Variables**: Use environment variables for configuration that might change between different environments (development, staging, production).

   ```cpp
   std::string db_url = env.getEnv("DATABASE_URL", "localhost:5432");
   // Use db_url to connect to your database
   ```

7. **Thread Safety**: While the `Env` class is thread-safe for its internal operations, be cautious when sharing the same `Env` object across multiple threads if you're also manipulating it.

8. **Static vs. Instance Methods**: Use static methods (`setVariable`, `getVariable`, etc.) when you need to interact with the system's environment variables directly. Use instance methods when you want to work with the `Env` object's internal state.

9. **Performance Considerations**: For frequently accessed values, consider caching them in your application rather than repeatedly calling `get()` or `getEnv()`.

   ```cpp
   class MyApp {
       atom::utils::Env& m_env;
       std::string m_cached_value;

   public:
       MyApp(atom::utils::Env& env) : m_env(env) {
           m_cached_value = m_env.get("FREQUENTLY_USED_VAR");
       }

       // Use m_cached_value instead of calling m_env.get() repeatedly
   };
   ```

10. **Cleanup**: If you've set any temporary environment variables during your program's execution, make sure to unset them before the program exits.

    ```cpp
    env.setEnv("TEMP_VAR", "some_value");
    // ... use TEMP_VAR ...
    env.unsetVariable("TEMP_VAR");  // Clean up when done
    ```

By following these best practices, you can effectively use the `atom::utils::Env` class to manage your application's environment, improve its configurability, and ensure consistent behavior across different execution contexts.
