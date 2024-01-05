# C++ Naming Conventions

## Variable Naming

- Use meaningful variable names that clearly express the purpose of the variable.
- Follow the camelCase convention for all code.
  - camelCase: Start with a lowercase letter, and capitalize the first letter of subsequent words.
  
    ```cpp
    int studentCount;
    ```

## Constant Naming

- Use uppercase letters and underscores to represent constants.

  ```cpp
  const int MAX_STUDENTS = 100;
  ```

## Function Naming

- Use verb plus noun format to name functions.
- It is more important to have a readable name than a concise one.
- Follow the camelCase convention.

  ```cpp
  void displayStudentInfo(const Student& student) {
      //...
  }
  ```

## Class Naming

- Use nouns or noun phrases to name classes.
- Follow the camelCase convention.

  ```cpp
  class Student {
      //...
  };
  ```

## Class Member Variables

- Use nouns or noun phrases to name class member variables.
- Follow the camelCase convention.
- Limit the usage of class member variables. If necessary, declare them as private variables and prefix them with "m_".

  ```cpp
  class Student {
      int m_id;
      string m_name;
      //...
  };
  ```

## Enum Type Naming

- Use uppercase letters and underscores to represent enum types.

  ```cpp
  enum Color { RED, GREEN, BLUE };
  ```

## Namespace Naming

- Use PascalCase for namespace naming.
- Split components into separate namespaces instead of putting everything into a single namespace to avoid complexity.

  ```cpp
  namespace Atom {
      //...
  }
  ```

## File Naming

- Use meaningful file names that clearly represent the content of the file.
- Use lowercase letters and underscores for file naming.

  ```cpp
  student_info.cpp
  ```

- If there is a c++ header file, please use `.hpp` as the file extension.

  ```cpp
  student_info.hpp
  ```

## Comments

- Use meaningful comments to explain the meaning and purpose of the code.
- Comments should be clear, concise, and kept up to date with the code.
- Prefer using Doxygen-style comments.

  ```cpp
  /**
   * @brief Display student information
   * @param student Student information
   */
  void displayStudentInfo(const Student& student) {
      //...
  }
  ```

## Summary

Good naming conventions improve code readability and maintainability, making it easier for others to understand and use your code.

# C++命名规则

## 变量命名

- 使用有意义的变量名，能够清晰表达变量用途。
- 所有代码建议采用驼峰命名法（camelCase）。
  - 驼峰命名法：首字母小写，单词之间没有下划线，后续单词首字母大写。

    ```cpp
    int studentCount;
    ```

## 常量命名

- 使用全大写字母和下划线来表示常量。

  ```cpp
  const int MAX_STUDENTS = 100;
  ```

## 函数命名

- 使用动词加名词的形式来命名函数。
- 能够看懂比简洁的命名更加重要。
- 采用驼峰命名法。

    ```cpp
    void displayStudentInfo(const Student& student) {
        //...
    }
    ```

## 类命名

- 使用名词或名词短语来命名类。
- 采用驼峰命名法。

  ```cpp
  class Student {
      //...
  };
  ```

## 类内变量

- 使用名词或名词短语来命名类内变量。
- 采用驼峰命名法。
- 类内变量应该尽量少，如果需要使用类内变量，应该将其声明为私有变量，所有类内变量前面都要加上m_前缀。

  ```cpp
  class Student {
      int m_id;
      string m_name;
      //...
  };
  ```

## 枚举类型命名

- 使用全大写字母和下划线来表示枚举类型。

  ```cpp
  enum Color { RED, GREEN, BLUE };
  ```

## 命名空间命名

- 采用大驼峰命名法。
- 具体的拆分组件，不要所有东西集中在一个命名空间中，不然会非常复杂。

  ```cpp
  namespace Atom {
      //...
  }
  ```

## 文件命名

- 使用有意义的文件名，能够清晰表达文件内容。
- 采用小写字母和下划线命名法。

  ```cpp
  student_info.cpp
  ```

- 如果是c++头文件，请使用`.hpp`结尾。

  ```txt
  student_info.hpp
  ```

## 注释

- 使用有意义的注释来解释代码的含义和目的。
- 注释应该清晰、简洁，并和代码保持同步。
- 最好使用Doxygen风格的注释。

  ```cpp
  /**
   * @brief 显示学生信息
   * @param student 学生信息
   */
  void displayStudentInfo(const Student& student) {
      //...
  }
  ```

## 总结

良好的命名规范能够提高代码的可读性和可维护性，帮助他人更容易理解和使用你的代码。
