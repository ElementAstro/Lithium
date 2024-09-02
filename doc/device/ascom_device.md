# ASCOM Device Handler Class Overview

The provided C# code defines an abstract class `AscomDevice<DeviceT>`, which is responsible for managing the connection and interaction with ASCOM devices. The class handles common operations such as connecting, disconnecting, setting up, and communicating with ASCOM devices in a unified manner.

## Overall Structure

The class has the following structure:

1. **Initialization**: The constructor initializes the ASCOM device with an ID and name.
2. **Properties and Methods**:
   - **Properties**: Various properties provide access to device information, such as `Name`, `Description`, `DriverVersion`, `Connected`, etc.
   - **Methods**:
     - `Connect`: Connects to the device.
     - `Disconnect`: Disconnects from the device.
     - `SetupDialog`: Displays a setup dialog for configuring the device.
     - `Action`, `SendCommandString`, `SendCommandBool`, `SendCommandBlind`: Methods for sending commands to the device.
     - `GetProperty`, `SetProperty`: Methods to get and set device properties, with caching and error handling.

### Overall Flowchart

```mermaid
graph TD
    A[Start] --> B[Initialize AscomDevice]
    B --> C{Is device connected?}
    C -- Yes --> D[Perform Operations]
    C -- No --> E[Connect to Device]
    E --> D
    D --> F{Is operation successful?}
    F -- Yes --> G[Return Success]
    F -- No --> H[Handle Error]
    H --> G
    G --> I[End]
```

---

## Step-by-Step Flowcharts

### 1. `Connect` Method

The `Connect` method is responsible for establishing a connection with the ASCOM device. It first initializes the device, then attempts to connect and run any post-connection tasks.

#### Flowchart

```mermaid
graph TD
    A[Start] --> B[Initialize Property Memory]
    B --> C[Call PreConnect Hook]
    C --> D[Create Device Instance]
    D --> E[Set Connected to true]
    E --> F{Is Connected?}
    F -- Yes --> G[Call PostConnect Hook]
    G --> H[Raise PropertyChanged Event]
    F -- No --> I[Show Error Notification]
    H --> J[End]
    I --> J
```

### 2. `Disconnect` Method

The `Disconnect` method handles the disconnection process from the ASCOM device. It ensures that all necessary pre-disconnection tasks are performed before safely disconnecting and disposing of the device.

#### Flowchart

```mermaid
graph TD
    A[Start] --> B[Call PreDisconnect Hook]
    B --> C[Set Connected to false]
    C --> D{Is Disconnection Successful?}
    D -- Yes --> E[Call PostDisconnect Hook]
    E --> F[Dispose Device]
    F --> G[Raise PropertyChanged Event]
    G --> H[End]
    D -- No --> I[Handle Disconnection Error]
    I --> H
```

### 3. `GetProperty` Method

The `GetProperty` method retrieves a property value from the ASCOM device, with caching and error handling to ensure reliability.

#### Flowchart

```mermaid
graph TD
    A[Start] --> B{Is Property Cached?}
    B -- Yes --> C[Return Cached Value]
    B -- No --> D[Get Property Value from Device]
    D --> E{Is Get Successful?}
    E -- Yes --> F[Cache Value and Return]
    F --> G[End]
    E -- No --> H{Is Retrying Allowed?}
    H -- Yes --> D
    H -- No --> I{Use Last Known Value?}
    I -- Yes --> J[Return Last Known Value]
    I -- No --> K[Return Default Value]
    J --> G
    K --> G
```

### 4. `SetProperty` Method

The `SetProperty` method sets a property value on the ASCOM device, with error handling for potential issues during the setting process.

#### Flowchart

```mermaid
graph TD
    A[Start] --> B{Is Property Implemented?}
    B -- No --> C[Log Not Implemented]
    C --> D[Return False]
    B -- Yes --> E{Is Device Connected?}
    E -- No --> F[Return False]
    E -- Yes --> G[Set Property Value on Device]
    G --> H{Is Set Successful?}
    H -- Yes --> I[Invalidate Cache]
    I --> J[Raise PropertyChanged Event]
    J --> K[Return True]
    H -- No --> L[Handle Set Error]
    L --> D
```
