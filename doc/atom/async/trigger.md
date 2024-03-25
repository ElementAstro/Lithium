# Trigger Class

The `Trigger` class provides a mechanism to register callbacks for specific events and trigger those callbacks when the events occur. It supports registering callbacks with different priorities, scheduling triggers with delays, and asynchronous triggering of events.

## Usage Example

```cpp
// Define a Trigger object with int parameter type
Trigger<int> trigger;

// Define a callback function
Trigger<int>::Callback callback = [](int param) {
    std::cout << "Callback triggered with param: " << param << std::endl;
};

// Register the callback for an event
trigger.registerCallback("event1", callback, Trigger<int>::CallbackPriority::Normal);

// Trigger the event with parameter 42
trigger.trigger("event1", 42);
```

## Methods

### `registerCallback`

Register a callback function for a specific event.

### `unregisterCallback`

Unregister a callback function for a specific event.

### `trigger`

Trigger the callback functions registered for a specific event.

### `scheduleTrigger`

Schedule the triggering of an event with a specified delay.

### `scheduleAsyncTrigger`

Schedule the asynchronous triggering of an event and return a future object.

### `cancelTrigger`

Cancel the scheduled triggering of a specific event.

### `cancelAllTriggers`

Cancel the scheduled triggering of all events.

## Private Members

- `m_mutex`: Mutex used for synchronizing access to the callback data structure.
- `m_callbacks`: Hash map to store registered callbacks for events.

### Example Usage of Scheduling a Trigger

```cpp
// Schedule a trigger with a delay of 500 milliseconds
trigger.scheduleTrigger("event2", 55, std::chrono::milliseconds(500));
```

### Example Usage of Asynchronous Triggering

```cpp
// Schedule an asynchronous trigger and get a future object
std::future<void> future = trigger.scheduleAsyncTrigger("event3", 77);

// Wait for the asynchronous trigger to complete
future.wait();
```
