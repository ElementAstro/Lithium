# `FilterWheelChooserVM` Class Overview

The `FilterWheelChooserVM` class in N.I.N.A. (Nighttime Imaging 'N' Astronomy) is responsible for managing the selection of filter wheel devices. This class extends `DeviceChooserVM<IFilterWheel>`, which provides base functionality for choosing a specific type of device, in this case, `IFilterWheel` devices. The class handles the initialization, retrieval, and selection of available filter wheel devices based on connected equipment, including FLI, Atik, QHY, ZWO, PlayerOne, SBIG, and other plugin or ASCOM-based devices.

## Key Components

- **`sbigSdk`**: SDK used for interacting with SBIG filter wheels.
- **`profileService`**: Manages the user's profile, including settings specific to the current configuration.
- **`equipmentProviders`**: A collection of providers that can supply additional filter wheel devices via plugins.
- **`lockObj`**: Semaphore ensuring thread-safe operations.
- **`ASCOMInteraction`**: Manages interactions with ASCOM-compatible filter wheel devices.
- **`AlpacaInteraction`**: Handles communication with Alpaca protocol-based filter wheel devices.

---

## Overall Flowchart

The overall flowchart below illustrates the general flow of the `GetEquipment` method in `FilterWheelChooserVM`:

```mermaid
flowchart TD
    A[Start] --> B[Lock lockObj]
    B --> C[Initialize devices List]
    C --> D[Add DummyDevice to devices]
    D --> E[Add FLI Filter Wheels]
    E --> F[Add Atik Filter Wheels]
    F --> G[Add QHY Filter Wheels]
    G --> H[Add ZWO Filter Wheels]
    H --> I[Add PlayerOne Filter Wheels]
    I --> J[Add SBIG Filter Wheels]
    J --> K[Add Plugin Providers]
    K --> L[Add ASCOM Filter Wheels]
    L --> M[Add Alpaca Filter Wheels]
    M --> N[Add Manual Filter Wheel]
    N --> O[Select Device Based on Active Profile]
    O --> P[Release lockObj]
    P --> Q[End]
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

The method initializes a `devices` list and adds a `DummyDevice`, which represents the absence of a physical filter wheel.

```mermaid
flowchart TD
    A[Initialize devices List] --> B[Add DummyDevice to devices]
    B --> C[Continue to Adding Filter Wheels]
```

### 3. Adding FLI Filter Wheels

The method attempts to add FLI filter wheels by calling `FLIFilterWheels.GetFilterWheels()` to retrieve a list of available FLI filter wheels and adding them to the `devices` list.

```mermaid
flowchart TD
    A[Continue to Adding Filter Wheels] --> B[Try to Add FLI Filter Wheels]
    B -- Success --> C[Add FLI Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Add Atik Filter Wheels]
    D --> E
```

### 4. Adding Atik Filter Wheels

The method attempts to add Atik filter wheels, both external (EFW) and internal. It first checks for external filter wheels using `AtikCameraDll.ArtemisEfwIsPresent()` and then checks for internal filter wheels by iterating over detected Atik cameras.

```mermaid
flowchart TD
    A[Continue to Add Atik Filter Wheels] --> B{Try to Add Atik EFW Filter Wheels}
    B -- Success --> C[Add Atik EFW Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E{Try to Add Atik Internal Filter Wheels}
    E -- Success --> F[Add Atik Internal Filter Wheels to devices]
    E -- Fail --> G[Log Error]
    F --> H[Continue to Add QHY Filter Wheels]
    G --> H
```

### 5. Adding QHY Filter Wheels

The method attempts to add QHY filter wheels using the `QHYFilterWheels` class to retrieve available QHY filter wheels and add them to the `devices` list.

```mermaid
flowchart TD
    A[Continue to Add QHY Filter Wheels] --> B{Try to Add QHY Filter Wheels}
    B -- Success --> C[Add QHY Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Add ZWO Filter Wheels]
    D --> E
```

### 6. Adding ZWO Filter Wheels

The method attempts to add ZWO (ZWOptical) filter wheels using the `EFWdll.GetNum()` method to detect and add them to the `devices` list.

```mermaid
flowchart TD
    A[Continue to Add ZWO Filter Wheels] --> B{Try to Add ZWO Filter Wheels}
    B -- Success --> C[Add ZWO Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Add PlayerOne Filter Wheels]
    D --> E
```

### 7. Adding PlayerOne Filter Wheels

The method attempts to add PlayerOne filter wheels using the `PlayerOneFilterWheelSDK.POAGetPWCount()` method to detect and add them to the `devices` list.

```mermaid
flowchart TD
    A[Continue to Add PlayerOne Filter Wheels] --> B{Try to Add PlayerOne Filter Wheels}
    B -- Success --> C[Add PlayerOne Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Add SBIG Filter Wheels]
    D --> E
```

### 8. Adding SBIG Filter Wheels

The method attempts to add SBIG filter wheels using the `SBIGFilterWheelProvider` class to retrieve and add them to the `devices` list.

```mermaid
flowchart TD
    A[Continue to Add SBIG Filter Wheels] --> B{Try to Add SBIG Filter Wheels}
    B -- Success --> C[Add SBIG Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Add Plugin Providers]
    D --> E
```

### 9. Adding Plugin Providers

The method then attempts to load any additional equipment from plugin providers. It loops through each provider, adding any found equipment to the `devices` list.

```mermaid
flowchart TD
    A[Continue to Add Plugin Providers] --> B{Load Plugin Providers}
    B --> C[Loop Through Providers]
    C --> D[Get Equipment from Provider]
    D --> E{Is Equipment Available?}
    E -- Yes --> F[Add Equipment to devices]
    E -- No --> G[Log Info]
    F --> H[Continue to Add ASCOM Filter Wheels]
    G --> H
```

### 10. Adding ASCOM Filter Wheels

After processing plugin providers, the method attempts to add ASCOM-compatible filter wheel devices by creating an instance of `ASCOMInteraction` and calling its `GetFilterWheels` method.

```mermaid
flowchart TD
    A[Continue to Add ASCOM Filter Wheels] --> B{Try to Add ASCOM Filter Wheels}
    B -- Success --> C[Add ASCOM Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Add Alpaca Filter Wheels]
    D --> E
```

### 11. Adding Alpaca Filter Wheels

Following the ASCOM filter wheel devices, the method attempts to add any filter wheel devices available via the Alpaca protocol by creating an instance of `AlpacaInteraction` and calling its `GetFilterWheels` method.

```mermaid
flowchart TD
    A[Continue to Add Alpaca Filter Wheels] --> B{Try to Add Alpaca Filter Wheels}
    B -- Success --> C[Add Alpaca Filter Wheels to devices]
    B -- Fail --> D[Log Error]
    C --> E[Continue to Add Manual Filter Wheel]
    D --> E
```

### 12. Adding Manual Filter Wheel

The method adds a `ManualFilterWheel` device to the `devices` list as a final fallback option.

```mermaid
flowchart TD
    A[Continue to Add Manual Filter Wheel] --> B[Add Manual Filter Wheel to devices]
    B --> C[Continue to Selecting Device]
```

### 13. Selecting the Device Based on Active Profile

Finally, the method selects the appropriate filter wheel device based on the active profile settings and releases the `lockObj` semaphore to complete the operation.

```mermaid
flowchart TD
    A[Continue to Selecting Device] --> B[Select Device Based on Active Profile]
    B --> C[Release lockObj]
    C --> D[End]
```
