# `FocuserChooserVM` Class Overview

The `FocuserChooserVM` class in N.I.N.A. (Nighttime Imaging 'N' Astronomy) is responsible for managing the selection of focuser devices. This class extends `DeviceChooserVM<IFocuser>`, which provides base functionality for choosing a specific type of device, in this case, `IFocuser` devices. The class handles the initialization, retrieval, and selection of available focuser devices based on connected equipment, ASCOM interactions, and plugins.

## Key Components

- **`profileService`**: Manages the user's profile, including settings specific to the current configuration.
- **`equipmentProviders`**: A collection of providers that can supply additional focuser devices via plugins.
- **`lockObj`**: Semaphore ensuring thread-safe operations.
- **`ASCOMInteraction`**: Manages interactions with ASCOM-compatible focuser devices.
- **`AlpacaInteraction`**: Handles communication with Alpaca protocol-based focuser devices.

---

## Overall Flowchart

The overall flowchart below illustrates the general flow of the `GetEquipment` method in `FocuserChooserVM`:

```mermaid
flowchart TD
    A[Start] --> B[Lock lockObj]
    B --> C[Initialize devices List]
    C --> D[Add DummyDevice to devices]
    D --> E{Load Plugin Providers}
    E --> F[Loop Through Providers]
    F --> G[Get Equipment from Provider]
    G --> H{Is Equipment Available?}
    H -- Yes --> I[Add Equipment to devices]
    H -- No --> J[Log Info]
    I --> K[Continue to ASCOM Focusers]
    J --> K
    K --> L{Try to Add ASCOM Focusers}
    L -- Success --> M[Add ASCOM Focusers to devices]
    L -- Fail --> N[Log Error]
    M --> O{Try to Add Alpaca Focusers}
    N --> O
    O -- Success --> P[Add Alpaca Focusers to devices]
    O -- Fail --> Q[Log Error]
    P --> R[Select Device Based on Active Profile]
    Q --> R
    R --> S[Release lockObj]
    S --> T[End]
```

---

## Step-by-Step Explanation with Detailed Flowcharts

### 1. Initialization and Locking

When the `GetEquipment` method is invoked, it first locks the `lockObj` semaphore to ensure that the operation is thread-safe.

```mermaid
flowchart TD
    A[Start] --> B[Lock lockObj]
    B --> C[Initialize devices List]
    C --> D[Continue to Add DummyDevice]
```

### 2. Adding a Dummy Device

The method initializes a `devices` list and adds a `DummyDevice`, which represents the absence of a physical focuser.

```mermaid
flowchart TD
    A[Initialize devices List] --> B[Add DummyDevice to devices]
    B --> C[Continue to Plugin Providers]
```

### 3. Loading Plugin Providers

The method then attempts to load any additional equipment from plugin providers. It loops through each provider, adding any found equipment to the `devices` list.

```mermaid
flowchart TD
    A[Continue to Plugin Providers] --> B{Load Plugin Providers}
    B --> C[Loop Through Providers]
    C --> D[Get Equipment from Provider]
    D --> E{Is Equipment Available?}
    E -- Yes --> F[Add Equipment to devices]
    E -- No --> G[Log Info]
    F --> H[Continue to ASCOM Focusers]
    G --> H
```

### 4. Adding ASCOM Focusers

After processing plugin providers, the method attempts to add ASCOM-compatible focuser devices by creating an instance of `ASCOMInteraction` and calling its `GetFocusers` method.

```mermaid
flowchart TD
    A[Continue to ASCOM Focusers] --> B{Try to Add ASCOM Focusers}
    B -- Success --> C[Add ASCOM Focusers to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Alpaca Focusers]
    D --> E
```

### 5. Adding Alpaca Focusers

Following the ASCOM focuser devices, the method attempts to add any focuser devices available via the Alpaca protocol by creating an instance of `AlpacaInteraction` and calling its `GetFocusers` method.

```mermaid
flowchart TD
    A[Continue to Alpaca Focusers] --> B{Try to Add Alpaca Focusers}
    B -- Success --> C[Add Alpaca Focusers to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Selecting Device]
    D --> E
```

### 6. Selecting the Device Based on Active Profile

Finally, the method selects the appropriate focuser device based on the active profile settings and releases the `lockObj` semaphore to complete the operation.

```mermaid
flowchart TD
    A[Continue to Selecting Device] --> B[Select Device Based on Active Profile]
    B --> C[Release lockObj]
    C --> D[End]
```
