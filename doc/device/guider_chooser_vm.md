# `GuiderChooserVM` Class Overview

The `GuiderChooserVM` class is part of the N.I.N.A. (Nighttime Imaging 'N' Astronomy) software and is responsible for managing the selection of guider devices. It extends the `DeviceChooserVM` class, which provides a base implementation for selecting a specific type of device, in this case, `IGuider` devices. This class handles the initialization, retrieval, and selection of different guider devices that are available based on the connected equipment and plugins.

## Key Components

- **`cameraMediator`**: Mediates the interaction with camera devices.
- **`telescopeMediator`**: Mediates the interaction with telescope devices.
- **`windowServiceFactory`**: Manages the creation of windows and UI elements.
- **`profileService`**: Handles the current profile, which includes settings specific to the user's setup.
- **`equipmentProviders`**: A collection of providers that can supply additional guider devices via plugins.

---

## Overall Flowchart

Below is the overall flowchart representing the flow of the `GetEquipment` method in `GuiderChooserVM`:

```mermaid
flowchart TD
    A[Start] --> B[Lock lockObj]
    B --> C[Initialize devices List]
    C --> D[Add DummyGuider to devices]
    D --> E[Add PHD2Guider to devices]
    E --> F[Add DirectGuider to devices]
    F --> G[Add MetaGuideGuider to devices]
    G --> H[Add SkyGuardGuider to devices]
    H --> I{Try to Add MGEN2}
    I -- Success --> J[Add MGENGuider to devices]
    I -- Fail --> K[Log Error]
    J --> L{Try to Add MGEN3}
    K --> L
    L -- Success --> M[Add MGEN3Guider to devices]
    L -- Fail --> N[Log Error]
    M --> O[Load Plugin Providers]
    N --> O
    O --> P{Loop Through Providers}
    P --> Q[Get Equipment from Provider]
    Q --> R{Is Equipment Available?}
    R -- Yes --> S[Add Equipment to devices]
    R -- No --> T[Log Info]
    S --> U[Select Device Based on Active Profile]
    T --> U
    U --> V[Release lockObj]
    V --> W[End]
```

---

## Step-by-Step Explanation with Detailed Flowcharts

### 1. Initialization and Locking

When the `GetEquipment` method is called, it first locks the `lockObj` semaphore to ensure thread safety.

```mermaid
flowchart TD
    A[Start] --> B[Lock lockObj]
    B --> C[Initialize devices List]
    C --> D[Continue to Add Built-in Devices]
```

### 2. Adding Built-in Guider Devices

The method initializes a `devices` list and adds several built-in guider devices (`DummyGuider`, `PHD2Guider`, `DirectGuider`, `MetaGuideGuider`, `SkyGuardGuider`).

```mermaid
flowchart TD
    A[Initialize devices List] --> B[Add DummyGuider to devices]
    B --> C[Add PHD2Guider to devices]
    C --> D[Add DirectGuider to devices]
    D --> E[Add MetaGuideGuider to devices]
    E --> F[Add SkyGuardGuider to devices]
    F --> G[Continue to MGEN Devices]
```

### 3. Adding MGEN Devices

The method attempts to add MGEN2 and MGEN3 devices, handling any exceptions that might occur during this process.

```mermaid
flowchart TD
    A[Add SkyGuardGuider to devices] --> B{Try to Add MGEN2}
    B -- Success --> C[Add MGENGuider to devices]
    B -- Fail --> D[Log Error]
    C --> E{Try to Add MGEN3}
    E -- Success --> F[Add MGEN3Guider to devices]
    E -- Fail --> G[Log Error]
    F --> H[Continue to Plugin Providers]
    G --> H
```

### 4. Loading Plugin Providers

After adding the built-in devices, the method loads additional equipment from plugin providers.

```mermaid
flowchart TD
    A[Continue to Plugin Providers] --> B[Load Plugin Providers]
    B --> C{Loop Through Providers}
    C --> D[Get Equipment from Provider]
    D --> E{Is Equipment Available?}
    E -- Yes --> F[Add Equipment to devices]
    E -- No --> G[Log Info]
    F --> H[Continue to Selecting Device]
    G --> H
```

### 5. Selecting the Device Based on Active Profile

Finally, the method selects the appropriate device based on the active profile settings and then releases the `lockObj` semaphore.

```mermaid
flowchart TD
    A[Continue to Selecting Device] --> B[Select Device Based on Active Profile]
    B --> C[Release lockObj]
    C --> D[End]
```
