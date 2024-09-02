# Overview of Stellarium Class

The `Stellarium` class interacts with the Stellarium application to fetch information such as site location, target information, and the current view or rotation angle. It utilizes multiple APIs provided by Stellarium, and the operations involve HTTP requests and JSON parsing.

## Overall Flowchart

```mermaid
flowchart TD
    A[Initialize Stellarium] --> B{Operation Type}
    B --> C[Get Site Location]
    B --> D[Get Target Information]
    B --> E[Get Rotation Angle]

    C --> F["/api/main/status"]
    D --> G{Check Oculars CCD}
    G --> H[Get Center View] --> J["/api/main/view"]
    G --> I[Get Selected Object Info] --> K["/api/objects/info"]
    E --> L["/api/stelproperty/list"]
```

# Detailed Step-by-Step Flowcharts

## Step 1: Get Site Location

The `GetSite` method fetches the site location by querying the `/api/main/status` endpoint.

```mermaid
flowchart TD
    A[GetSite] --> B[Create HttpGetRequest]
    B --> C[Send Request to /api/main/status]
    C --> D{Response Received?}
    D -- Yes --> E[Parse Response as StellariumStatus]
    E --> F[Extract Location Data (Latitude, Longitude, Elevation)]
    D -- No --> G[Throw PlanetariumFailedToConnect]
```

## Step 2: Get Target Information

The `GetTarget` method checks whether the CCD ocular is enabled and either fetches the current view or selected object information.

```mermaid
flowchart TD
    A[GetTarget] --> B[Check Oculars CCD Enabled]
    B --> C{Is CCD Enabled?}
    C -- Yes --> D[Get Center View] --> F[Create HttpGetRequest to /api/main/view]
    C -- No --> E[Get Selected Object Info] --> G[Create HttpGetRequest to /api/objects/info]
    D --> H{Response Valid?}
    E --> I{Response Valid?}
    H -- Yes --> J[Parse StellariumView]
    I -- Yes --> K[Parse StellariumObject]
    H -- No --> L[Throw PlanetariumFailedToConnect]
    I -- No --> M[Try Get Center View]
```

## Step 3: Get Rotation Angle

The `GetRotationAngle` method fetches the rotation angle using the `/api/stelproperty/list` endpoint.

```mermaid
flowchart TD
    A[GetRotationAngle] --> B[Create HttpGetRequest to /api/stelproperty/list]
    B --> C[Send Request]
    C --> D{Response Received?}
    D -- Yes --> E[Parse Response as JObject]
    E --> F[Check Oculars CCD Enabled]
    F --> G{Is CCD Enabled?}
    G -- Yes --> H[Parse Rotation Angle]
    G -- No --> I[Return NaN]
    D -- No --> J[Throw PlanetariumFailedToConnect]
```

# Extracted APIs

1. **/api/main/status**: Used in `GetSite` to retrieve the current site location including latitude, longitude, and elevation.

2. **/api/main/view**: Used in `GetCenterView` to retrieve the center view's celestial coordinates in different epochs.

3. **/api/objects/info**: Used in `GetSelectedObjectInfo` to fetch information about the currently selected object, including right ascension, declination, and object name.

4. **/api/stelproperty/list**: Used in `GetRotationAngle` to obtain properties related to the telescope's ocular settings, including the rotation angle.

These flowcharts and descriptions should give a clear understanding of how the `Stellarium` class operates and interacts with Stellarium's API to retrieve the necessary astronomical data.
