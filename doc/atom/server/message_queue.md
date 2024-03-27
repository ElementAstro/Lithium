# MessageQueue Class Documentation

The `MessageQueue` class provides a message queue implementation that allows subscribers to receive messages of type T.

## Template Class Definition

### Template Parameter

- **T**: The type of messages that can be published and subscribed to.

## Constructor

The class does not have a constructor as it is a template class.

## Public Methods

### subscribe()

Subscribes a callback function to receive messages.

```cpp
messageQueue.subscribe([](const int& message) {
    std::cout << "Received message: " << message << std::endl;
}, "Subscriber1");
```

### unsubscribe()

Unsubscribes a callback function from receiving messages.

```cpp
messageQueue.unsubscribe(callbackFunction);
```

### publish()

Publishes a new message to all subscribed callback functions.

```cpp
messageQueue.publish(42);
```

_Expected Output_: "Published message: 42"

### startProcessingThread()

Starts the processing thread(s) to receive and handle messages.

This method will start the processing thread(s) to handle incoming messages.

```cpp
messageQueue.startProcessingThread();
```

### stopProcessingThread()

Stops the processing thread(s) from receiving and handling messages.

This method will stop the processing thread(s) from handling any more messages.

```cpp
messageQueue.stopProcessingThread();
```

## Private Members

### Subscriber Struct

Contains information about a subscribed callback function.

- **name**: The name of the subscriber.
- **callback**: The callback function to be called when a new message is received.

### Member Variables

- **m_messages**: Queue containing all published messages.
- **m_subscribers**: Vector containing all subscribed callback functions.
- **m_mutex**: Mutex used to protect access to the message queue.
- **m_subscriberMutex**: Mutex used to protect access to the subscriber vector.
- **m_condition**: Condition variable used to notify processing threads of new messages.
- **m_isRunning**: Flag to indicate whether the processing thread(s) should continue running.
- **m_processingThreads**: Vector containing all processing threads.
- **m_numThreads**: Number of processing threads to spawn (initialized with `std::thread::hardware_concurrency()`).
