# **Overview of the C2A Class**

The `C2A` class in the `NINA.Equipment.Equipment.MyPlanetarium` namespace implements the `IPlanetarium` interface. This class is designed to interact with the C2A planetarium software via TCP/IP, allowing it to obtain the coordinates of selected objects and the user’s location.

## **Class Members**:

- **Fields**:

  - `string address`: The IP address of the C2A server.
  - `int port`: The port number used to communicate with the C2A server.

- **Constructor**:

  - `C2A(IProfileService profileService)`: Initializes the `C2A` class using the provided profile service to obtain the address and port of the C2A server.

- **Properties**:

  - `string Name`: Returns the name "C2A".
  - `bool CanGetRotationAngle`: Indicates that this implementation cannot get a rotation angle (returns `false`).

- **Methods**:
  - `Task<DeepSkyObject> GetTarget()`: Asynchronously gets the selected object’s coordinates from C2A.
  - `Task<Location> GetSite(CancellationToken token)`: Asynchronously retrieves the user’s location from C2A.
  - `Task<double> GetRotationAngle()`: Returns `NaN` since the rotation angle cannot be retrieved.

---

# 2. **Step-by-Step Flowcharts**

## **Overall Flowchart**

```mermaid
graph TD
    A1[Start] --> B1[Initialize C2A Object]
    B1 --> C1{Choose Action}
    C1 --> |Get Target| D1[Call GetTarget Method]
    C1 --> |Get Site| E1[Call GetSite Method]
    C1 --> |Get Rotation Angle| F1[Call GetRotationAngle Method]
    D1 --> G1[Return DeepSkyObject]
    E1 --> H1[Return Location]
    F1 --> I1[Return NaN]
    G1 --> J1[End]
    H1 --> J1
    I1 --> J1
```

## **Step-by-Step Flowchart for `GetTarget` Method**

```mermaid
graph TD
    A2[Start] --> B2[Create Command String for Coordinates]
    B2 --> C2[Send Command to C2A Server]
    C2 --> D2[Receive Response]
    D2 --> E2{Response Valid?}
    E2 --> |Yes| F2[Parse RA and Dec]
    E2 --> |No| G2[Throw PlanetariumFailedToGetCoordinates Exception]
    F2 --> H2[Create and Return DeepSkyObject]
    G2 --> I2[Log Error and Throw Exception]
    H2 --> J2[End]
    I2 --> J2
```

## **Step-by-Step Flowchart for `GetSite` Method**

```mermaid
graph TD
    A3[Start] --> B3[Create Command String for Location]
    B3 --> C3[Send Command to C2A Server]
    C3 --> D3[Receive Response]
    D3 --> E3{Response Valid?}
    E3 --> |Yes| F3[Parse Latitude and Longitude]
    E3 --> |No| G3[Throw PlanetariumFailedToGetCoordinates Exception]
    F3 --> H3[Create and Return Location Object]
    G3 --> I3[Log Error and Throw Exception]
    H3 --> J3[End]
    I3 --> J3
```

## **Step-by-Step Flowchart for `GetRotationAngle` Method**

```mermaid
graph TD
    A4[Start] --> B4[Return NaN]
    B4 --> C4[End]
```

---

# 3. **API Extraction**

Below is a list of all public APIs extracted from the `C2A` class:

## **Constructor**:

- **`C2A(IProfileService profileService)`**:
  - **Parameters**:
    - `IProfileService profileService`: Provides access to the active profile's settings for the C2A host and port.

## **Properties**:

- **`string Name { get; }`**:
  - **Description**: Returns the name "C2A".
- **`bool CanGetRotationAngle { get; }`**:
  - **Description**: Indicates that the implementation cannot provide a rotation angle.

## **Methods**:

- **`Task<DeepSkyObject> GetTarget()`**:

  - **Description**: Asynchronously retrieves the selected object’s right ascension and declination from C2A.
  - **Returns**: `DeepSkyObject` representing the selected object.

- **`Task<Location> GetSite(CancellationToken token)`**:

  - **Description**: Asynchronously retrieves the user's geographical location (latitude and longitude) from C2A.
  - **Returns**: `Location` object containing latitude, longitude, and elevation.

- **`Task<double> GetRotationAngle()`**:
  - **Description**: Returns `NaN`, as this implementation does not support retrieving a rotation angle.
  - **Returns**: `double` (always `NaN`).

---

This detailed markdown documentation should provide clear insights into the structure, functionality, and API endpoints of the `C2A` class.
