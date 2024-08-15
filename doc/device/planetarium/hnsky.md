# HNSKY Class Overview

The `HNSKY` class is designed to interface with HNSKY planetarium software. It allows the user to:

1. **Retrieve the selected astronomical object** from the planetarium.
2. **Retrieve the configured user location** from the planetarium.

## Key Properties

- **address**: The IP address of the HNSKY instance.
- **port**: The port on which HNSKY is listening.

## Key Methods

- `GetTarget()`: Retrieves the currently selected deep-sky object from HNSKY.
- `GetSite(CancellationToken token)`: Retrieves the user’s configured location from HNSKY.
- `GetRotationAngle()`: Returns the rotation angle, although this method is not supported by HNSKY and will return NaN.

## Overall Flowchart

```mermaid
graph TD;
    A[Start] --> B{Initialize HNSKY Instance};
    B --> C[GetTarget Method];
    B --> D[GetSite Method];
    C --> E[Handle Response];
    D --> F[Handle Response];
    E --> G[Return DeepSkyObject];
    F --> H[Return Location];
    G --> I[End];
    H --> I[End];
```

---

# Method Details and Flowcharts

## 1. GetTarget Method

## Purpose:

Fetches the currently selected deep-sky object from HNSKY.

## Step-by-Step Flowchart

```mermaid
graph TD;
    A[Start GetTarget] --> B[Prepare GET_TARGET Command];
    B --> C[Send Command to HNSKY];
    C --> D[Receive Response];
    D --> E{Valid Response?};
    E --> |Yes| F[Parse Response];
    E --> |No| G[Throw Exception];
    F --> H[Create DeepSkyObject Instance];
    H --> I[Return DeepSkyObject];
    G --> J[Throw PlanetariumObjectNotSelectedException];
    I --> K[End GetTarget];
    J --> K[End GetTarget];
```

## API Extraction:

- **Command Sent**: `GET_TARGET`
- **Response Format**: `RA Dec Name Position_angle`
- **Key Fields Extracted**:
  - **RA**: Right Ascension in radians
  - **Dec**: Declination in radians
  - **Name**: Name of the astronomical object

## 2. GetSite Method

## Purpose:

Retrieves the user’s configured location from HNSKY.

## Step-by-Step Flowchart

```mermaid
graph TD;
    A[Start GetSite] --> B[Prepare GET_LOCATION Command];
    B --> C[Send Command to HNSKY];
    C --> D[Receive Response];
    D --> E{Valid Response?};
    E --> |Yes| F[Parse Response];
    E --> |No| G[Throw Exception];
    F --> H[Create Location Instance];
    H --> I[Return Location];
    G --> J[Throw PlanetariumFailedToGetCoordinates];
    I --> K[End GetSite];
    J --> K[End GetSite];
```

## API Extraction:

- **Command Sent**: `GET_LOCATION`
- **Response Format**: `Latitude Longitude Julian_Date`
- **Key Fields Extracted**:
  - **Latitude**: Latitude in radians
  - **Longitude**: Longitude in radians (flipped sign)
  - **Julian_Date**: The Julian date

## 3. GetRotationAngle Method

## Purpose:

This method is a placeholder and does not perform any action related to rotation angle in HNSKY. It returns NaN.

## Simple Flowchart

```mermaid
graph TD;
    A[Start GetRotationAngle] --> B[Return NaN];
    B --> C[End GetRotationAngle];
```
