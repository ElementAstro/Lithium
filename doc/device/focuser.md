# `AscomFocuser` Class Overview

The `AscomFocuser` class is a concrete implementation of the `AscomDevice<IFocuserV3>` base class. It represents a focuser device used in astrophotography, controlling the position of the focusing mechanism. The class supports both absolute and relative focusers, provides properties for various device characteristics, and includes methods to move the focuser to a specified position.

## Overall Flowchart

```mermaid
flowchart TD
    A[AscomFocuser Class] --> B[Properties]
    A --> C[Methods]
    A --> D[Initialization]

    B --> B1[IsMoving]
    B --> B2[MaxIncrement]
    B --> B3[MaxStep]
    B --> B4[Position]
    B --> B5[StepSize]
    B --> B6[TempCompAvailable]
    B --> B7[TempComp]
    B --> B8[Temperature]

    C --> C1[Move]
    C --> C2[Halt]

    C1 --> C1a[MoveInternalAbsolute]
    C1 --> C1b[MoveInternalRelative]

    D --> D1[Initialize]
    D --> D2[PostConnect]
    D --> D3[GetInstance]
```

## Step-by-Step Flowchart

## 1. **Initialization (`Initialize` Method)**

The `Initialize` method sets up the focuser based on its characteristics, such as whether it's an absolute or relative focuser.

```mermaid
flowchart TD
    A[Initialize Method] --> B[internalPosition = device.MaxStep / 2]
    B --> C[_isAbsolute = device.Absolute]
    C --> D{Is Absolute?}
    D --> E[Yes] --> F[Absolute Focuser]
    D --> G[No] --> H[Relative Focuser]
    H --> I[Logger.Info: Simulating absolute focuser behavior]
    F --> J[_canHalt = true]
```

## 2. **Move Method (`Move`)**

The `Move` method is responsible for moving the focuser to a specified position. It decides whether to move the focuser absolutely or relatively based on the `_isAbsolute` flag.

```mermaid
flowchart TD
    A[Move Method] --> B{Is Absolute?}
    B --> C[Yes] --> D[MoveInternalAbsolute]
    B --> E[No] --> F[MoveInternalRelative]
```

## 3. **MoveInternalAbsolute Method**

This method moves the focuser to a specific position for absolute focusers. It checks for timeouts and ensures that the focuser is moving as expected.

```mermaid
flowchart TD
    A[MoveInternalAbsolute Method] --> B[Check Connected State]
    B --> C[TempComp = false]
    C --> D{Position Reached?}
    D --> E[Yes] --> F[Exit Loop]
    D --> G[No] --> H{Same Position?}
    H --> I[Yes] --> J[Increase samePositionCount]
    H --> K[No] --> L[Update lastMovementTime]
    I --> M{Timeout Reached?}
    M --> N[Yes] --> O[Throw Exception]
    M --> P[No] --> Q[Wait and Retry]
```

## 4. **MoveInternalRelative Method**

This method handles relative movement by moving the focuser in increments until the desired position is reached.

```mermaid
flowchart TD
    A[MoveInternalRelative Method] --> B[Check Connected State]
    B --> C[TempComp = false]
    C --> D[Calculate relativeOffsetRemaining]
    D --> E{Offset = 0?}
    E --> F[Yes] --> G[Exit Loop]
    E --> H[No] --> I[Move by MinMaxIncrement, relativeOffsetRemaining]
    I --> J[Update internalPosition]
    J --> K[Check IsMoving]
    K --> L{IsMoving?}
    L --> M[Yes] --> N[Wait and Retry]
    L --> O[No] --> P[Update relativeOffsetRemaining]
```

## Methods

1. **`AscomFocuser` Constructor**: Initializes the `AscomFocuser` instance with the given focuser and name.
2. **`IsMoving`**: Returns whether the focuser is currently moving.
3. **`MaxIncrement`**: Returns the maximum step increment the focuser can move.
4. **`MaxStep`**: Returns the maximum step position.
5. **`Position`**: Returns the current position of the focuser.
6. **`StepSize`**: Returns the size of each step in the focuser.
7. **`TempCompAvailable`**: Indicates if temperature compensation is available.
8. **`TempComp`**: Gets or sets the temperature compensation status.
9. **`Temperature`**: Returns the current temperature as reported by the focuser.
10. **`Move`**: Moves the focuser to a specified position, either absolutely or relatively.
11. **`Halt`**: Stops the focuser movement if possible.
12. **`Initialize`**: Sets up the focuser, including handling of relative focusers.
13. **`MoveInternalAbsolute`**: Moves the focuser to an absolute position with timeout handling.
14. **`MoveInternalRelative`**: Moves the focuser relatively, handling incremental movement.
15. **`PostConnect`**: Performs post-connection initialization.
16. **`GetInstance`**: Returns an instance of the `IFocuserV3` interface.
