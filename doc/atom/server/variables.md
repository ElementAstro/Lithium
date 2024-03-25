# VariableRegistry Class

The `VariableRegistry` class is designed to register, retrieve, and observe variable values.

## Constructor

### `explicit VariableRegistry(const std::string &name)`

- **Description:** Constructor to create a `VariableRegistry` object with a specified name.

  ```cpp
  VariableRegistry registry("MyRegistry");
  ```

## Public Methods

### RegisterVariable

```cpp
template <typename T>
bool RegisterVariable(const std::string &name, const T &initialValue,
                      const std::string description = "");
```

- **Description:** Registers a new variable with an initial value and an optional description.

  ```cpp
  registry.RegisterVariable("myVar", 42); // Register a variable named "myVar" with initial value 42
  ```

### SetVariable

```cpp
template <typename T>
bool SetVariable(const std::string &name, const T &value);
```

- **Description:** Sets the value of a registered variable.

  ```cpp
  registry.SetVariable("myVar", 100); // Set the value of "myVar" to 100
  ```

### GetVariable

```cpp
template <typename T>
std::optional<T> GetVariable(const std::string &name) const;
```

- **Description:** Retrieves the value of a registered variable.

  ```cpp
  auto value = registry.GetVariable<int>("myVar");
  if (value.has_value()) {
      std::cout << "Value of myVar: " << value.value() << std::endl;
  }
  ```

### AddObserver

```cpp
void AddObserver(const std::string &name, const Observer &observer);
```

- **Description:** Adds an observer to monitor changes in a variable's value.

  ```cpp
  VariableRegistry::Observer observer{"myObserver", [](const std::string &name) {
      std::cout << "Value of " << name << " changed." << std::endl;
  }};
  registry.AddObserver("myVar", observer);
  ```

### NotifyObservers

```cpp
template <typename T>
void NotifyObservers(const std::string &name, const T &value) const;
```

- **Description:** Notifies all observers that a variable's value has changed.

  ```cpp
  registry.NotifyObservers("myVar", 200);
  ```

### GetAll

```cpp
[[nodiscard]] emhash8::HashMap<std::string, std::any> GetAll() const;
```

- **Description:** Retrieves a map of all registered variables.

  ```cpp
  auto allVariables = registry.GetAll();
  ```

### RemoveAll

- **Description:** Clears all registered variables.

  ```cpp
  registry.RemoveAll();
  ```

### AddGetter

```cpp
template <typename T>
void AddGetter(const std::string &name, const std::function<T()> &getter);
```

- **Description:** Adds a callback function to retrieve the value of a variable.

  ```cpp
  registry.AddGetter<int>("myVar", []() { return 500; });
  ```

### AddSetter

```cpp
template <typename T>
void AddSetter(const std::string &name, const std::function<void(const std::any &)> &setter);
```

- **Description:** Adds a callback function to monitor changes in a variable.

  ```cpp
  registry.AddSetter<int>("myVar", [](const std::any &newValue) {
      // Custom validation logic here
  });
  ```

## Notes

- Ensure to use `RegisterVariable` before calling `SetVariable` or `GetVariable` for a specific variable.
- Callback functions added via `AddGetter` and `AddSetter` should match the variable type registered.

This `VariableRegistry` class provides a flexible and efficient way to manage and observe variables in your application.
