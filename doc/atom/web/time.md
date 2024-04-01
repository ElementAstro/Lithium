# System Time Management API Documentation

## Function: getSystemTime

```cpp
std::time_t getSystemTime();
```

- Retrieves the current system time in seconds since Unix epoch.

```cpp
std::time_t currentTime = getSystemTime();
std::cout << "Current system time: " << currentTime << " seconds since Unix epoch" << std::endl;
```

Expected Output

```txt
Current system time: 1648323625 seconds since Unix epoch
```

---

## Function: setSystemTime

```cpp
void setSystemTime(int year, int month, int day, int hour, int minute, int second);
```

- Sets the system time to the specified date and time.

```cpp
setSystemTime(2024, 3, 25, 15, 0, 0); // Set the system time to March 25, 2024, 3:00:00 PM
```

---

## Function: setSystemTimezone

```cpp
bool setSystemTimezone(const std::string &timezone);
```

- Sets the system timezone to the specified timezone.

```cpp
bool success = setSystemTimezone("America/New_York");
if (success) {
    std::cout << "Timezone set successfully" << std::endl;
} else {
    std::cout << "Failed to set timezone" << std::endl;
}
```

---

## Function: syncTimeFromRTC

```cpp
bool syncTimeFromRTC();
```

- Synchronizes the system time with an RTC (real-time clock) device.

```cpp
bool syncSuccess = syncTimeFromRTC();
if (syncSuccess) {
    std::cout << "System time synchronized with RTC" << std::endl;
} else {
    std::cout << "Failed to synchronize system time with RTC" << std::endl;
}
```

---

## Complete Example

Here's a complete example of using the system time management functions:

```cpp
int main() {
    std::time_t currentTime = getSystemTime();
    std::cout << "Current system time: " << currentTime << " seconds since Unix epoch" << std::endl;

    setSystemTime(2024, 3, 25, 15, 0, 0);

    bool tzSetSuccess = setSystemTimezone("America/New_York");
    if (tzSetSuccess) {
        std::cout << "Timezone set successfully" << std::endl;
    } else {
        std::cout << "Failed to set timezone" << std::endl;
    }

    bool syncSuccess = syncTimeFromRTC();
    if (syncSuccess) {
        std::cout << "System time synchronized with RTC" << std::endl;
    } else {
        std::cout << "Failed to synchronize system time with RTC" << std::endl;
    }

    return 0;
}
```
