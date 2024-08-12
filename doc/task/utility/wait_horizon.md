# WaitUntilAboveHorizon

The `WaitUntilAboveHorizon` class is part of the `NINA.Sequencer.SequenceItem.Utility` namespace and is used to wait until an astronomical object is above a specified horizon level.

## Namespace

```csharp
namespace NINA.Sequencer.SequenceItem.Utility
```

## Class Declaration

```csharp
[ExportMetadata("Name", "Lbl_SequenceItem_Utility_WaitUntilAboveHorizon_Name")]
[ExportMetadata("Description", "Lbl_SequenceItem_Utility_WaitUntilAboveHorizon_Description")]
[ExportMetadata("Icon", "WaitForAltitudeSVG")]
[ExportMetadata("Category", "Lbl_SequenceCategory_Utility")]
[Export(typeof(ISequenceItem))]
[JsonObject(MemberSerialization.OptIn)]
public class WaitUntilAboveHorizon : WaitForAltitudeBase, IValidatable
```

## Properties

### `HasDsoParent`

- **Type:** `bool`
- **Description:** Indicates if this item has a Deep-Sky Object (DSO) parent.
- **Json Property:** `[JsonProperty]`

### `UpdateInterval`

- **Type:** `int`
- **Description:** The interval, in seconds, for checking the altitude.
- **Default Value:** `1`

## Constructors

### Default Constructor

```csharp
[ImportingConstructor]
public WaitUntilAboveHorizon(IProfileService profileService) : base(profileService, useCustomHorizon: true)
```

- **Parameters:**
  - `profileService`: Service providing profile information.

### Clone Constructor

```csharp
private WaitUntilAboveHorizon(WaitUntilAboveHorizon cloneMe) : this(cloneMe.ProfileService)
```

- **Parameters:**
  - `cloneMe`: The instance to clone.

## Methods

### `Clone`

```csharp
public override object Clone()
```

- **Description:** Creates a deep copy of the current `WaitUntilAboveHorizon` instance.
- **Flowchart:**

```mermaid
graph TD;
    A[Clone Method] --> B[Create New WaitUntilAboveHorizon]
    B --> C[Copy Properties]
    C --> D[Return New Instance]
    D --> E[End]
```

### `Execute`

```csharp
public override async Task Execute(IProgress<ApplicationStatus> progress, CancellationToken token)
```

- **Description:** Continuously checks if the current altitude is above the target altitude and waits for the specified interval if not.
- **Flowchart:**

```mermaid
graph TD;
    A[Execute Method] --> B[Set Target Altitude with Horizon]
    B --> C[Transform Coordinates to AltAz]
    C --> D[Check If Current Altitude > Target Altitude]
    D -- Yes --> E[Log Completion]
    D -- No --> F[Delay for Update Interval]
    F --> B
    E --> G[End]
```

### `GetCurrentAltitude`

```csharp
public double GetCurrentAltitude(DateTime time, ObserverInfo observer)
```

- **Description:** Calculates the current altitude for a given time and observer.
- **Flowchart:**

```mermaid
graph TD;
    A[GetCurrentAltitude Method] --> B[Transform Coordinates to AltAz]
    B --> C[Return Altitude]
    C --> D[End]
```

### `CalculateExpectedTime`

```csharp
public override void CalculateExpectedTime()
```

- **Description:** Calculates the expected time for the object to reach the target altitude.
- **Flowchart:**

```mermaid
graph TD;
    A[CalculateExpectedTime Method] --> B[Get Current Altitude]
    B --> C[Calculate Expected Time Common]
    C --> D[End]
```

### `AfterParentChanged`

```csharp
public override void AfterParentChanged()
```

- **Description:** Updates coordinates based on the parent context and validates the data.
- **Flowchart:**

```mermaid
graph TD;
    A[AfterParentChanged Method] --> B[Retrieve Context Coordinates]
    B --> C[Update Coordinates If Available]
    C --> D[Set HasDsoParent]
    D --> E[Validate Data]
    E --> F[End]
```

### `Validate`

```csharp
public bool Validate()
```

- **Description:** Validates if the target altitude can be reached based on the current horizon and coordinates.
- **Flowchart:**

```mermaid
graph TD;
    A[Validate Method] --> B[Calculate Max Altitude]
    B --> C[Calculate Minimum Horizon Altitude]
    C --> D[Check If Max Altitude < Min Horizon Altitude]
    D -- Yes --> E[Add Error Message]
    D -- No --> F[Calculate Expected Time]
    E --> G[Return False]
    F --> H[Return True]
    G --> H
    H --> I[End]
```
