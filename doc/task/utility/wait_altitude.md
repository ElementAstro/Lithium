# `WaitForAltitude` Class

The `WaitForAltitude` class is part of the `NINA.Sequencer.SequenceItem.Utility` namespace and is used to wait until an astronomical object reaches a specified altitude.

## Namespace

```csharp
namespace NINA.Sequencer.SequenceItem.Utility
```

## Class Declaration

```csharp
[ExportMetadata("Name", "Lbl_SequenceItem_Utility_WaitForAltitude_Name")]
[ExportMetadata("Description", "Lbl_SequenceItem_Utility_WaitForAltitude_Description")]
[ExportMetadata("Icon", "WaitForAltitudeSVG")]
[ExportMetadata("Category", "Lbl_SequenceCategory_Utility")]
[Export(typeof(ISequenceItem))]
[JsonObject(MemberSerialization.OptIn)]
public class WaitForAltitude : WaitForAltitudeBase, IValidatable
```

## Properties

### `AboveOrBelow`

- **Type:** `string`
- **Description:** Defines whether the target altitude should be above (`>`) or below (`<`) a certain value.
- **Json Property:** `[JsonProperty]`

### `HasDsoParent`

- **Type:** `bool`
- **Description:** Indicates if this item has a Deep-Sky Object (DSO) parent.
- **Json Property:** `[JsonProperty]`

## Constructors

### Default Constructor

```csharp
[ImportingConstructor]
public WaitForAltitude(IProfileService profileService) : base(profileService, useCustomHorizon: false)
```

- **Parameters:**
  - `profileService`: Service providing profile information.

### Clone Constructor

```csharp
private WaitForAltitude(WaitForAltitude cloneMe) : this(cloneMe.ProfileService)
```

- **Parameters:**
  - `cloneMe`: The instance to clone.

## Methods

### `Clone`

```csharp
public override object Clone()
```

- **Description:** Creates a deep copy of the current `WaitForAltitude` instance.
- **Flowchart:**

```mermaid
graph TD;
    A[Clone Method] --> B[Create New WaitForAltitude]
    B --> C[Copy Properties]
    C --> D[Return New Instance]
    D --> E[End]
```

### `Execute`

```csharp
public override async Task Execute(IProgress<ApplicationStatus> progress, CancellationToken token)
```

- **Description:** Continuously checks if the current altitude is above or below the target altitude and waits for the specified interval if not.
- **Flowchart:**

```mermaid
graph TD;
    A[Execute Method] --> B[Transform Coordinates to AltAz]
    B --> C[Check If Altitude Meets Criteria]
    C -- Yes --> D[Break Loop]
    C -- No --> E[Delay for 1 Second]
    E --> B
    D --> F[End]
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

- **Description:** Validates if the target altitude can be reached based on the current conditions.
- **Flowchart:**

```mermaid
graph TD;
    A[Validate Method] --> B[Calculate Max Altitude]
    B --> C[Calculate Min Altitude]
    C --> D[Check Altitude Validity]
    D -- Invalid --> E[Add Error Message]
    D -- Valid --> F[Calculate Expected Time]
    E --> G[Return False]
    F --> H[Return True]
    G --> H
    H --> I[End]
```
