# Logger Class

## Brief

The Logger class provides logging functionality with support for dynamic log level adjustment, asynchronous logging, and multiple subscribers.

## Constructor

```cpp
Logger logger;
```

## addSubscriber

Adds a subscriber to receive log messages.

```cpp
class CustomSubscriber : public Logger::Subscriber {
public:
    void log(LogLevel level, const std::string &message) override {
        // Custom log handling
    }
};

std::shared_ptr<CustomSubscriber> subscriber = std::make_shared<CustomSubscriber>();
logger.addSubscriber(subscriber);
```

## setLogLevel

Sets the log level for filtering log messages.

```cpp
logger.setLogLevel(LogLevel::Info);
```

## log

Logs a message with the specified log level.

```cpp
logger.log(LogLevel::Error, "An error message");
```

## getLogLevelString

Gets the string representation of a log level.

```cpp
std::string levelString = Logger::getLogLevelString(LogLevel::Debug);
// Expected output: "Debug"
```

## workerFunction

Worker function for asynchronous processing of log messages.

```cpp
std::thread workerThread(&Logger::workerFunction, &logger);
```

## writeToLogFile

Writes a log message to a log file.

```cpp
logger.writeToLogFile("Error", "An error message");
```
