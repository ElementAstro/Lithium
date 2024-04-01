# INIFile Class

The `INIFile` class provides functionality to load, save, set, get, and manipulate data in INI files. It also supports conversion of the data to JSON and XML formats.

## Public Methods

### Load

```cpp
void load(const std::string &filename);
```

- **Description:** Loads an INI file.
- **Usage Example:**

```cpp
INIFile iniFile;
iniFile.load("config.ini");
```

### Save

```cpp
void save(const std::string &filename);
```

- **Description:** Saves the current state to an INI file.
- **Usage Example:**

```cpp
INIFile iniFile;
iniFile.save("config_saved.ini");
```

### Set

```cpp
template <typename T>
void set(const std::string &section, const std::string &key, const T &value);
```

- **Description:** Sets a value in the INI file.
- **Usage Example:**

```cpp
INIFile iniFile;
iniFile.set("Section1", "Key1", 42);
```

### Get

```cpp
template <typename T>
std::optional<T> get(const std::string &section, const std::string &key) const;
```

- **Description:** Gets a value from the INI file.
- **Usage Example:**

```cpp
INIFile iniFile;
auto value = iniFile.get<int>("Section1", "Key1");
if (value.has_value()) {
    std::cout << "Value: " << value.value() << std::endl;
}
```

### Has

```cpp
bool has(const std::string &section, const std::string &key) const;
```

- **Description:** Checks if a key exists in the specified section.
- **Usage Example:**

```cpp
INIFile iniFile;
bool exists = iniFile.has("Section1", "Key1");
```

### HasSection

```cpp
bool hasSection(const std::string &section) const;
```

- **Description:** Checks if a section exists in the INI file.
- **Usage Example:**

```cpp
INIFile iniFile;
bool exists = iniFile.hasSection("Section1");
```

### Operator[]

```cpp
std::unordered_map<std::string, std::any> operator[](const std::string &section);
```

- **Description:** Returns the content of a section as a map.
- **Usage Example:**

```cpp
INIFile iniFile;
auto sectionData = iniFile["Section1"];
```

### toJson

```cpp
std::string toJson() const;
```

- **Description:** Converts the data in the INI file to a JSON string.
- **Usage Example:**

```cpp
INIFile iniFile;
std::string jsonStr = iniFile.toJson();
```

### toXml

```cpp
std::string toXml() const;
```

- **Description:** Converts the data in the INI file to an XML string.
- **Usage Example:**

```cpp
INIFile iniFile;
std::string xmlStr = iniFile.toXml();
```

## Private Methods

### parseLine

```cpp
void parseLine(const std::string &line, std::string &currentSection);
```

- **Description:** Parses a line from the INI file and updates the current section.

### trim

```cpp
std::string trim(const std::string &str);
```

- **Description:** Trims leading and trailing spaces from a string.

## Overall Example

```cpp
INIFile iniFile;
iniFile.load("config.ini");

iniFile.set("Section1", "Key1", 42);
iniFile.set("Section1", "Key2", "Value");

if (iniFile.has("Section1", "Key1")) {
    auto value = iniFile.get<int>("Section1", "Key1");
    if (value.has_value()) {
        std::cout << "Value: " << value.value() << std::endl;
    }
}

std::string jsonStr = iniFile.toJson();
std::string xmlStr = iniFile.toXml();

std::cout << "JSON Representation: " << jsonStr << std::endl;
std::cout << "XML Representation: " << xmlStr << std::endl;
```
