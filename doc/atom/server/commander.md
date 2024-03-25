# Command Dispatcher

The `CommandDispatcher` class is a generic command dispatcher for handling and dispatching commands. It allows registration of handler functions for specific commands, along with optional undo handlers and decorators to modify or enhance the behavior of the registered handlers. This document provides a detailed explanation of the class interface along with usage examples and expected outputs.

## Class Template Parameters

- `Result`: The result type of the command handler function.
- `Argument`: The argument type of the command handler function.

## Class Declaration

```cpp
template <typename Result, typename Argument>
class CommandDispatcher : public NonCopyable {
public:
    using HandlerFunc = std::function<Result(const Argument &)>;
    using DecoratorFunc = std::shared_ptr<decorator<HandlerFunc>>;
    using LoopDecoratorFunc = std::shared_ptr<LoopDecorator<HandlerFunc>>;
    using ConditionalDecoratorFunc =
        std::shared_ptr<ConditionCheckDecorator<HandlerFunc>>;

    // Constructor and destructor
    CommandDispatcher() = default;
    ~CommandDispatcher();

    // Factory methods
    static std::shared_ptr<CommandDispatcher> createShared();
    static std::unique_ptr<CommandDispatcher> createUnique();

    // Member functions for registration and dispatching
    void registerHandler(const std::string &name, const HandlerFunc &handler,
                         const HandlerFunc &undoHandler = nullptr);
    template <class T>
    void registerMemberHandler(const std::string &name, T *object,
                               Result (T::*memberFunc)(const Argument &));
    void registerDecorator(const std::string &name,
                           const DecoratorFunc &decorator);
    void registerLoopDecorator(const std::string &name,
                               const LoopDecoratorFunc &decorator);
    void registerConditionalDecorator(const std::string &name,
                                    const ConditionalDecoratorFunc &decorator);
    HandlerFunc getHandler(const std::string &name);
    bool hasHandler(const std::string &name);
    Result dispatch(const std::string &name, const Argument &data);
    bool undo(); // Not yet implemented
    bool redo(); // Not yet implemented
    bool removeAll();
    void registerFunctionDescription(const std::string &name,
                                     const std::string &description);
    std::string getFunctionDescription(const std::string &name);
    void removeFunctionDescription(const std::string &name);
    void clearFunctionDescriptions();
    void setMaxHistorySize(size_t maxSize);
    size_t getMaxHistorySize() const;

private:
    // Data members
    std::unordered_map<std::string, HandlerFunc> m_handlers;
    std::unordered_map<std::string, DecoratorFunc> m_decorators;
    std::unordered_map<std::string, HandlerFunc> m_undoHandlers;
    std::unordered_map<std::string, std::string> m_descriptions;
    std::stack<std::pair<std::string, Argument>> m_commandHistory;
    std::stack<std::pair<std::string, Argument>> m_undoneCommands;
    mutable std::shared_mutex m_sharedMutex;
    size_t m_maxHistorySize = 100;
};
```

## Usage Examples

### 1. Registering a Handler

```cpp
// Create a CommandDispatcher instance
auto dispatcher = CommandDispatcher<int, std::string>::createShared();

// Define a handler function
CommandDispatcher<int, std::string>::HandlerFunc handlerFunc =
    [](const std::string &arg) { return arg.length(); };

// Register the handler for a command
dispatcher->registerHandler("length", handlerFunc);

// Dispatch the command
int result = dispatcher->dispatch("length", "example");
std::cout << "Length: " << result << std::endl; // Expected output: Length: 7
```

### 2. Registering a Member Function Handler

```cpp
class StringHandler {
public:
    int getLength(const std::string &str) {
        return str.length();
    }
};

// Create an instance of the handler class
StringHandler handler;

// Register the member function handler
dispatcher->registerMemberHandler("length", &handler, &StringHandler::getLength);

// Dispatch the command
int result = dispatcher->dispatch("length", "example");
std::cout << "Length: " << result << std::endl; // Expected output: Length: 7
```

### 3. Registering a Decorator

```cpp
// Define a decorator function
auto decoratorFunc = std::make_shared<decorator<CommandDispatcher<int, std::string>::HandlerFunc>>(
    [](CommandDispatcher<int, std::string>::HandlerFunc func) {
        return [=](const std::string &arg) {
            std::cout << "Before executing command" << std::endl;
            int result = func(arg);
            std::cout << "After executing command" << std::endl;
            return result;
        };
    });

// Register the decorator for a command
dispatcher->registerDecorator("length", decoratorFunc);

// Dispatch the command
int result = dispatcher->dispatch("length", "example");
std::cout << "Length: " << result << std::endl; // Expected output: Before executing command
                                                 // Length: 7
                                                 // After executing command
```

### 4. Setting Maximum History Size

```cpp
// Set the maximum history size
dispatcher->setMaxHistorySize(50);

// Get the maximum history size
size_t maxSize = dispatcher->getMaxHistorySize();
std::cout << "Max history size: " << maxSize << std::endl; // Expected output: Max history size: 50
```

## Notes

- The `undo()` and `redo()` methods are not yet implemented. They will be available in future versions.
- The `removeAll()` method removes all registered handler functions.
- Function descriptions can be registered using `registerFunctionDescription()` and retrieved using `getFunctionDescription()`.
- The command history is managed internally for undo and redo operations, but those functionalities are not yet ready for use.
