# ParkScope

The `ParkScope` class is designed for parking a telescope and stopping any ongoing guiding operations. It integrates with both telescope and guider mediators to ensure the telescope is safely parked.

## Class Overview

### Namespace

- **Namespace:** `NINA.Sequencer.SequenceItem.Telescope`
- **Dependencies:**
  - `NINA.Core.Model`
  - `NINA.Sequencer.Validations`
  - `NINA.Equipment.Interfaces.Mediator`
  - `NINA.Core.Locale`

### Class Declaration

```csharp
[ExportMetadata("Name", "Lbl_SequenceItem_Telescope_ParkScope_Name")]
[ExportMetadata("Description", "Lbl_SequenceItem_Telescope_ParkScope_Description")]
[ExportMetadata("Icon", "ParkSVG")]
[ExportMetadata("Category", "Lbl_SequenceCategory_Telescope")]
[Export(typeof(ISequenceItem))]
[JsonObject(MemberSerialization.OptIn)]
public class ParkScope : SequenceItem, IValidatable
```

### Class Properties

- **telescopeMediator**: Handles communication with the telescope hardware.
- **guiderMediator**: Manages communication with the guider hardware.
- **issues**: A list to record any validation issues.

### Constructor

The constructor initializes the `ParkScope` class with `telescopeMediator` and `guiderMediator`.

```csharp
[ImportingConstructor]
public ParkScope(ITelescopeMediator telescopeMediator, IGuiderMediator guiderMediator)
```

### Key Methods

- **Clone()**: Creates a copy of the `ParkScope` instance.
- **Execute(IProgress<ApplicationStatus> progress, CancellationToken token)**: Commands the telescope to park and stops guiding.
- **Validate()**: Checks if the telescope is connected.
- **AfterParentChanged()**: Revalidates the state when the parent changes.
- **ToString()**: Returns a string representation of the class instance.

### Flowchart: Execution Process

Below is a flowchart illustrating the key steps in the `Execute` method of the `ParkScope` class.

```mermaid
flowchart TD
    A[Start Execute Method] --> B[Stop Guiding]
    B --> C[Park Telescope]
    C --> D[Handle Failure]
    D --> E[End Execution]
```

### Flowchart Explanation

1. **Stop Guiding**: Stops any ongoing guiding operation using `guiderMediator`.
2. **Park Telescope**: Commands the telescope to park using `telescopeMediator`. If this operation fails, an exception is thrown.
3. **Handle Failure**: If the telescope fails to park, a `SequenceEntityFailedException` is thrown.
4. **End Execution**: Marks the end of the execution process.

### Detailed Method Descriptions

#### `Clone`

Creates a new instance of the `ParkScope` class with the same configuration as the current instance.

#### `Execute`

1. **Stop Guiding**: Calls `guiderMediator.StopGuiding()` to halt any ongoing guiding operations.
2. **Park Telescope**: Uses `telescopeMediator.ParkTelescope()` to park the telescope. If the parking operation is unsuccessful, an exception is thrown.

#### `Validate`

Checks the current state of the telescope:

- If the telescope is not connected, adds an error to the issues list.
- Updates the `Issues` property with any validation errors.

#### `AfterParentChanged`

Revalidates the state of the `ParkScope` instance whenever its parent changes to ensure it remains valid.

#### `ToString`

Provides a string representation of the `ParkScope` instance, including the category and item name.
