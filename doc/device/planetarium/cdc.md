# CartesDuCiel Class

## Overview of the Code

The `CartesDuCiel` class implements the `IPlanetarium` interface and is responsible for communicating with the Cartes Du Ciel (CdC) planetarium software to retrieve astronomical data. The key methods include:

1. **`GetTarget()`**: Retrieves the selected object or the current view in CdC.
2. **`GetView()`**: Retrieves the coordinates of the current view in CdC.
3. **`GetSite()`**: Retrieves the observer's site location from CdC.
4. **`GetRotationAngle()`**: Returns a constant rotation angle, which is not implemented and always returns `NaN`.

## Overall Flowchart

Below is an overall flowchart representing the process flow in the `CartesDuCiel` class.

```mermaid
flowchart TD
    A[Start] --> B{Is Target Selected?}
    B -->|Yes| C[Get Selected Object Data]
    B -->|No| D[Get Current View Data]
    C --> E[Return DeepSkyObject]
    D --> F[Return DeepSkyObject]
    E --> G[End]
    F --> G[End]
```

## Step-by-Step Flowchart for `GetTarget` Method

The `GetTarget` method is more complex, involving multiple steps to retrieve and process data from CdC.

```mermaid
flowchart TD
    A[Start GetTarget] --> B[Send 'GETSELECTEDOBJECT' Command]
    B --> C{Response Starts with 'OK!'?}
    C -->|No| D[Call GetView Method]
    C -->|Yes| E{Response Has More than 2 Columns?}
    E -->|No| D
    E -->|Yes| F[Extract RA and Dec from Response]
    F --> G{Match RA Format?}
    G -->|No| H[Throw PlanetariumObjectNotSelectedException]
    G -->|Yes| I[Convert RA to Degrees]
    I --> J{Match Dec Format?}
    J -->|No| H
    J -->|Yes| K[Convert Dec to Degrees]
    K --> L[Extract Equinox from Response]
    L --> M{Equinox Contains '2000'?}
    M -->|Yes| N[Use J2000 Epoch]
    M -->|No| O[Use JNOW Epoch]
    O --> P[Create DeepSkyObject]
    N --> P
    P --> Q[Return DeepSkyObject]
    D --> Q
```

## Step-by-Step Flowchart for `GetView` Method

This method is called when no object is selected in CdC and it retrieves the current view's RA and Dec.

```mermaid
flowchart TD
    A[Start GetView] --> B[Send 'GETRA F' Command]
    B --> C[Send 'GETDEC F' Command]
    C --> D[Send 'GETCHARTEQSYS F' Command]
    D --> E{Responses Start with 'OK!'?}
    E -->|No| F[Throw PlanetariumObjectNotSelectedException]
    E -->|Yes| G[Parse RA and Dec from Responses]
    G --> H[Convert RA and Dec to Degrees]
    H --> I[Check Equinox Response]
    I --> J{Equinox Contains '2000'?}
    J -->|Yes| K[Use J2000 Epoch]
    J -->|No| L[Use JNOW Epoch]
    K --> M[Create DeepSkyObject]
    L --> M
    M --> N[Return DeepSkyObject]
```

## Step-by-Step Flowchart for `GetSite` Method

This method retrieves the observer's site information from CdC.

```mermaid
flowchart TD
    A[Start GetSite] --> B[Send 'GETOBS' Command]
    B --> C{Response Starts with 'OK!'?}
    C -->|No| D[Throw PlanetariumFailedToGetCoordinates]
    C -->|Yes| E[Extract Latitude, Longitude, and Altitude]
    E --> F[Convert Latitude and Longitude to Degrees]
    F --> G[Create Location Object]
    G --> H[Return Location]
```

## Extracted APIs

Here is a list of all the key API calls and utility methods used in the `CartesDuCiel` class:

1. **`BasicQuery.SendQuery()`**: Sends a command to the CdC application and returns the response.
2. **`AstroUtil.HMSToDegrees()`**: Converts Right Ascension (RA) from hours/minutes/seconds to degrees.
3. **`AstroUtil.DMSToDegrees()`**: Converts Declination (Dec) from degrees/minutes/seconds to degrees.
4. **`Coordinates.Transform()`**: Transforms coordinates based on the epoch.
5. **`Logger.Error()`**: Logs errors encountered during execution.
6. **`CancellationToken`**: Manages task cancellation.

These flowcharts and API extractions provide a comprehensive view of the class's functionality and internal processes.
