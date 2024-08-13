# GuiderVM Class Overview

The `GuiderVM` class is part of the N.I.N.A. software, responsible for managing the guiding operations within the application. It interacts with guider devices to establish connections, start/stop guiding sessions, handle RMS (Root Mean Square) error calculations, and more.

## Overall Structure

The overall structure of the `GuiderVM` class can be visualized in a flowchart that outlines its main components and interactions:

```mermaid
flowchart TD
    A[GuiderVM] --> B[ProfileService]
    A --> C[GuiderMediator]
    A --> D[ApplicationStatusMediator]
    A --> E[DeviceChooserVM]
    A --> F[Guider]
    A --> G[GuideStepsHistory]
    F --> H[GuiderInfo]

    B -- Handles --> I[Profile Settings]
    F -- Raises --> J[Property Changed Events]
    F -- Handles --> K[Guide Events]

    J --> A
    K --> A
    A --> L[UI Commands]
    A --> M[Events]
```

### Components:

- **GuiderVM**: The main ViewModel that controls the guiding functionalities.
- **ProfileService**: Manages the active profile settings.
- **GuiderMediator**: Facilitates communication between various parts of the application related to guiding.
- **ApplicationStatusMediator**: Manages and updates the application status.
- **DeviceChooserVM**: Handles device selection and management.
- **Guider**: Represents the connected guider device.
- **GuiderInfo**: Holds information about the connected guider device.
- **GuideStepsHistory**: Maintains a history of guide steps and calculates RMS errors.

## Step-by-Step Processes

### 1. Initialization

The initialization process sets up the `GuiderVM` class, registers event handlers, and prepares commands for execution.

```mermaid
flowchart TD
    A[GuiderVM Constructor] --> B[Initialize Properties]
    B --> C[Register Event Handlers]
    C --> D[Initialize Commands]
    D --> E[Set Initial States]
    E --> F[Completed]
```

### 2. Connecting to the Guider

The process of connecting to a guider involves selecting the device, establishing the connection, and setting up necessary event handlers.

```mermaid
flowchart TD
    A[Connect Command Execution] --> B[Reset Graph Values]
    B --> C[Select Device]
    C --> D[Attempt Connection]
    D --> E[Connection Successful?]
    E -- Yes --> F[Set Up Event Handlers]
    F --> G[Broadcast Guider Info]
    E -- No --> H[Handle Connection Failure]
    H --> I[Completed]
    G --> I
```

### 3. Starting Guiding

This process details how the guiding session is started, including checking the connection status, reporting progress, and initiating guiding operations.

```mermaid
flowchart TD
    A[Start Guiding Command Execution] --> B[Check Connection Status]
    B --> C[Report Status: Starting Guiding]
    C --> D[Start Guiding Process]
    D --> E[Guiding Started Successfully?]
    E -- Yes --> F[Invoke GuidingStarted Event]
    E -- No --> G[Handle Guiding Failure]
    F --> H[Completed]
    G --> H
```

### 4. Handling Guide Events

The process of handling guide events involves updating the guide steps history, calculating RMS errors, and invoking any subscribed events.

```mermaid
flowchart TD
    A[Guide Event Received] --> B[Update GuideStepsHistory]
    B --> C[Calculate RMS Errors]
    C --> D[Update GuiderInfo with RMS]
    D --> E[Invoke GuideEvent Subscribers]
    E --> F[Completed]
```

### 5. Disconnecting the Guider

Disconnecting from the guider device involves stopping any ongoing processes, unregistering event handlers, and resetting the state.

```mermaid
flowchart TD
    A[Disconnect Command Execution] --> B[Stop Guiding Process]
    B --> C[Unregister Event Handlers]
    C --> D[Disconnect from Guider]
    D --> E[Reset Guider Info]
    E --> F[Broadcast Updated Info]
    F --> G[Completed]
```

## Key Methods and Commands

### Methods

- **`Connect`**: Attempts to establish a connection with the selected guider device.
- **`Disconnect`**: Disconnects the current guider and resets related states.
- **`StartGuiding`**: Initiates the guiding process.
- **`StopGuiding`**: Stops the ongoing guiding process.
- **`Rescan`**: Rescans for available guider devices.
- **`AutoSelectGuideStar`**: Automatically selects a guide star.

### Commands

- **`ConnectCommand`**: Triggers the connection process.
- **`DisconnectCommand`**: Triggers the disconnection process.
- **`RescanDevicesCommand`**: Initiates the device rescan process.
- **`ClearGraphCommand`**: Clears the guide graph.
- **`SetShiftRateCommand`**: Sets the shift rate for guiding.
- **`StopShiftCommand`**: Stops the shifting process.

## Event Handling

### Events

- **`Connected`**: Fired when the guider is successfully connected.
- **`Disconnected`**: Fired when the guider is disconnected.
- **`GuideEvent`**: Fired when a new guide step event occurs.
- **`GuidingStarted`**: Fired when guiding has successfully started.
- **`GuidingStopped`**: Fired when guiding has been stopped.
