# ExternalScrip

The `ExternalScript` class is part of the `NINA.Sequencer.SequenceItem.Utility` namespace and is designed to execute an external script or command as part of a sequence item.

## Namespace

```csharp
namespace NINA.Sequencer.SequenceItem.Utility
```

## Class Declaration

```csharp
[ExportMetadata("Name", "Lbl_SequenceItem_Utility_ExternalScript_Name")]
[ExportMetadata("Description", "Lbl_SequenceItem_Utility_ExternalScript_Description")]
[ExportMetadata("Icon", "ScriptSVG")]
[ExportMetadata("Category", "Lbl_SequenceCategory_Utility")]
[Export(typeof(ISequenceItem))]
[JsonObject(MemberSerialization.OptIn)]
public class ExternalScript : SequenceItem, IValidatable
```

## Properties

### `OpenDialogCommand`

- **Type:** `System.Windows.Input.ICommand`
- **Description:** Command to open a file dialog for selecting the script to execute.

### `Issues`

- **Type:** `IList<string>`
- **Description:** List of validation issues.
- **Json Property:** `[JsonProperty]`

### `Script`

- **Type:** `string`
- **Description:** Path to the external script or command to be executed.
- **Json Property:** `[JsonProperty]`

## Constructors

### Default Constructor

```csharp
public ExternalScript()
```

- **Description:** Initializes the command to open a file dialog for selecting the script.

### Clone Constructor

```csharp
private ExternalScript(ExternalScript cloneMe) : this()
```

- **Parameters:**
  - `cloneMe`: The instance to clone.

## Methods

### `Clone`

```csharp
public override object Clone()
```

- **Description:** Creates a deep copy of the current `ExternalScript` instance.
- **Flowchart:**

```mermaid
graph TD;
    A[Clone Method] --> B[Create New ExternalScript]
    B --> C[Copy Properties]
    C --> D[Return New Instance]
    D --> E[End]
```

### `Execute`

```csharp
public override async Task Execute(IProgress<ApplicationStatus> progress, CancellationToken token)
```

- **Description:** Executes the external script or command and reports progress.
- **Flowchart:**

```mermaid
graph TD;
    A[Execute Method] --> B[Prepare External Command]
    B --> C[Run External Command]
    C --> D[Check Success]
    D -- Success --> E[Complete]
    D -- Failure --> F[Throw Exception]
    E --> G[End]
    F --> G
```

### `Validate`

```csharp
public bool Validate()
```

- **Description:** Validates if the script or command exists and is executable.
- **Flowchart:**

```mermaid
graph TD;
    A[Validate Method] --> B[Check Command Exists]
    B -- Exists --> C[Return True]
    B -- Does Not Exist --> D[Add Error Message]
    D --> E[Return False]
    C --> E
    E --> F[End]
```

### `AfterParentChanged`

```csharp
public override void AfterParentChanged()
```

- **Description:** Validates the script after a parent change event.
- **Flowchart:**

```mermaid
graph TD;
    A[AfterParentChanged Method] --> B[Validate Script]
    B --> C[End]
```

### `ToString`

```csharp
public override string ToString()
```

- **Description:** Returns a string representation of the `ExternalScript` instance.
- **Flowchart:**

```mermaid
graph TD;
    A[ToString Method] --> B[Format String]
    B --> C[Return String]
    C --> D[End]
```
