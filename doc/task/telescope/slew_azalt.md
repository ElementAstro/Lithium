# SlewScopeToAltAz

The `SlewScopeToAltAz` class is designed to move a telescope to a specific altitude and azimuth position. It handles the calculation of the telescope's new position based on input coordinates and interacts with both the telescope and guider systems.

## Class Overview

### Namespace

- **Namespace:** `NINA.Sequencer.SequenceItem.Telescope`
- **Dependencies:**
  - `NINA.Core.Model`
  - `NINA.Profile.Interfaces`
  - `NINA.Sequencer.Validations`
  - `NINA.Astrometry`
  - `NINA.Equipment.Interfaces.Mediator`
  - `NINA.Core.Locale`
  - `NINA.Core.Utility.Notification`

### Class Declaration

```csharp
[ExportMetadata("Name", "Lbl_SequenceItem_Telescope_SlewScopeToAltAz_Name")]
[ExportMetadata("Description", "Lbl_SequenceItem_Telescope_SlewScopeToAltAz_Description")]
[ExportMetadata("Icon", "SlewToAltAzSVG")]
[ExportMetadata("Category", "Lbl_SequenceCategory_Telescope")]
[Export(typeof(ISequenceItem))]
[JsonObject(MemberSerialization.OptIn)]
public class SlewScopeToAltAz : SequenceItem, IValidatable
```

### Class Properties

- **profileService**: Service for accessing and managing profile-related data.
- **telescopeMediator**: Interface for communicating with the telescope hardware.
- **guiderMediator**: Interface for managing the guiding system.
- **Coordinates**: The target altitude and azimuth coordinates for the telescope.
- **issues**: List of validation issues.

### Constructor

The constructor initializes the `SlewScopeToAltAz` class with the provided services and sets up an event handler for profile location changes.

```csharp
[ImportingConstructor]
public SlewScopeToAltAz(IProfileService profileService, ITelescopeMediator telescopeMediator, IGuiderMediator guiderMediator)
```

### Key Methods

- **Clone()**: Creates a copy of the `SlewScopeToAltAz` instance with the same configuration.
- **Execute(IProgress<ApplicationStatus> progress, CancellationToken token)**: Moves the telescope to the specified coordinates, handling any necessary guiding operations.
- **AfterParentChanged()**: Revalidates the state when the parent changes.
- **Validate()**: Checks if the telescope is connected and ready.
- **ToString()**: Provides a string representation of the class instance.

### Flowchart: Execution Process

Below is a flowchart illustrating the key steps in the `Execute` method of the `SlewScopeToAltAz` class.

```mermaid
flowchart TD
    A[Start Execute Method] --> B{Telescope Parked?}
    B -- Yes --> C[Show Error and Abort]
    B -- No --> D[Stop Guiding]
    D --> E[Slew to Coordinates]
    E --> F{Guiding Stopped?}
    F -- Yes --> G[Start Guiding]
    G --> H[End Execution]
    F -- No --> H
```

### Flowchart Explanation

1. **Telescope Parked?**: Checks if the telescope is parked. If it is, an error message is shown, and the operation is aborted.
2. **Stop Guiding**: Stops the guiding process to prepare for the slewing operation.
3. **Slew to Coordinates**: Moves the telescope to the specified altitude and azimuth coordinates.
4. **Guiding Stopped?**: Checks if guiding was stopped. If it was, guiding is restarted.

### Detailed Method Descriptions

#### `Clone`

Creates a new instance of the `SlewScopeToAltAz` class with the same coordinates and configuration as the current instance.

#### `Execute`

1. **Check Telescope Status**: If the telescope is parked, an error is shown, and the method throws an exception.
2. **Stop Guiding**: Stops the guiding process to prevent conflicts during the slewing operation.
3. **Slew to Coordinates**: Uses `telescopeMediator.SlewToCoordinatesAsync()` to move the telescope to the target coordinates.
4. **Restart Guiding**: If guiding was stopped, it restarts guiding to resume normal operations.

#### `AfterParentChanged`

Revalidates the instance whenever its parent changes to ensure it remains in a valid state.

#### `Validate`

Checks if the telescope is connected. If it is not, an error is added to the issues list.

#### `ToString`

Provides a string representation of the `SlewScopeToAltAz` instance, including the category, item name, and the target coordinates.

### Coordinates

- **Coordinates**: Represents the altitude and azimuth coordinates to which the telescope should move.
