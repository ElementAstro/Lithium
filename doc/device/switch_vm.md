# SwitchVM

## Overview

The `SwitchVM` class is a ViewModel for handling switch equipment in the N.I.N.A. (Nighttime Imaging 'N' Astronomy) application. It manages the connection to switch devices, provides commands for interaction, and updates the state of switches.

### Overall Flowchart

```mermaid
graph TD
    A[User Interaction] --> B[Execute Command]
    B --> C{Command Type}
    C -->|Connect| D[Connect Method]
    C -->|Disconnect| E[Disconnect Method]
    C -->|SetSwitchValue| F[SetSwitchValue Method]
    C -->|RescanDevices| G[RescanDevices Method]
    C -->|Other| H[Other Methods]

    D --> I[Update Switch Info]
    E --> J[Clear Switch Info]
    F --> K[Update Switch Value]
    G --> L[Update Device List]
    I --> M[Broadcast Switch Info]
    J --> M
    K --> M
    L --> M
    M --> N[Notify Status]
    H --> N
    N --> O[Update UI]
```

### Step-by-Step Flowcharts

#### 1. Connect Method

The `Connect` method establishes a connection to the selected switch device.

```mermaid
flowchart TD
    A[Start] --> B[Acquire Semaphore]
    B --> C[Disconnect Existing Connection]
    C --> D{Check Device ID}
    D -->|No_Device| E[Set Device ID]
    D -->|Valid| F[Update Status to Connecting]
    F --> G[Initialize Cancellation Token]
    G --> H[Connect to Device]
    H --> I{Check Connection}
    I -->|Connected| J[Update Switch Hub]
    I -->|Not Connected| K[Notify Error]
    J --> L[Initialize Switch Collections]
    L --> M[Update Switch Info]
    M --> N[Start Update Timer]
    N --> O[Notify Success]
    O --> P[Release Semaphore]
    P --> Q[End]
    K --> P
```

#### 2. Disconnect Method

The `Disconnect` method terminates the connection and cleans up resources.

```mermaid
flowchart TD
    A[Start] --> B{Check if Connected}
    B -->|Yes| C[Stop Update Timer]
    B -->|No| D[Skip Stop Timer]
    C --> E[Disconnect Device]
    E --> F[Clear Switch Collections]
    F --> G[Reset Switch Info]
    G --> H[Notify Disconnection]
    H --> I[Release Semaphore]
    I --> J[End]
    D --> I
```

#### 3. SetSwitchValue Method

The `SetSwitchValue` method sets the value of a writable switch and verifies the update.

```mermaid
flowchart TD
    A[Start] --> B[Set Switch Value]
    B --> C[Poll Switch]
    C --> D{Check Value Difference}
    D -->|Within Tolerance| E[Success]
    D -->|Not Within Tolerance| F[Wait and Retry]
    F --> G{Check Timeout}
    G -->|Timeout| H[Notify Error]
    G -->|No Timeout| C
    E --> I[Update UI]
    H --> I
    I --> J[End]
```

#### 4. RescanDevices Method

The `RescanDevices` method refreshes the list of available devices.

```mermaid
flowchart TD
    A[Start] --> B[Get Equipment from Device Chooser]
    B --> C[Update Device List]
    C --> D[Return Device IDs]
    D --> E[Update UI]
    E --> F[End]
```
