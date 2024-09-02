# Meridian Flip Process

The Meridian Flip process in N.I.N.A. (Nighttime Imaging 'N' Astronomy) is a critical operation for astrophotography, where the telescope's orientation is adjusted to avoid tracking errors as celestial objects cross the meridian. The following documentation details the flow of operations, providing a step-by-step breakdown accompanied by flowcharts.

---

## **Overall Flowchart**

```mermaid
graph TD
    A[Start Meridian Flip] --> B[Initialize Parameters]
    B --> C[Check if Auto Meridian Flip is Necessary]
    C --> D{Is Exposure Length > Time to Flip?}
    D -- Yes --> E[Wait until Meridian Flip is Possible]
    D -- No --> F[Stop Guider]
    F --> G[Pass Meridian]
    G --> H[Execute Flip]
    H --> I{Recenter Enabled?}
    I -- Yes --> J[Recenter After Flip]
    I -- No --> K[Skip Recenter]
    J --> L[Resume Guider]
    K --> L
    L --> M[Settle Scope]
    M --> N[Rotate Image (if enabled)]
    N --> O[Meridian Flip Completed]
```

---

## **Step-by-Step Flowchart & Details**

1. **Start Meridian Flip Process**

   - The `MeridianFlipVM` class starts the Meridian Flip process when certain conditions are met during astrophotography sessions.

2. **Initialize Parameters**

   - The process begins by initializing various parameters such as the target coordinates, time to flip, and other settings related to the telescope and guider.

   ```mermaid
   graph TD
       A[Start Meridian Flip Process] --> B[Initialize Parameters]
       B --> C[Create `AutomatedWorkflow` Instance]
   ```

3. **Check if Auto Meridian Flip is Necessary**

   - The system compares the exposure length with the time remaining to flip. If the exposure time is longer, the flip is delayed to avoid errors.

   ```mermaid
   graph TD
       A[Check if Auto Meridian Flip is Necessary] --> B{Is Exposure Length > Time to Flip?}
       B -- Yes --> C[Wait until Meridian Flip is Possible]
       B -- No --> D[Proceed to Next Steps]
   ```

4. **Stop Guider**

   - The guider is stopped to avoid interference during the flip.

   ```mermaid
   graph TD
       A[Stop Guider] --> B[Proceed to Pass Meridian]
   ```

5. **Pass Meridian**

   - The system stops the telescope tracking and waits until the object passes the meridian.

   ```mermaid
   graph TD
       A[Pass Meridian] --> B[Resume Tracking]
   ```

6. **Execute Flip**

   - The telescope is instructed to flip to the specified target coordinates after passing the meridian.

   ```mermaid
   graph TD
       A[Execute Flip] --> B[Check Dome Synchronization]
       B --> C[Wait for Dome Synchronization (if needed)]
       C --> D[Flip Completed Successfully?]
   ```

7. **Recenter After Flip (if enabled)**

   - If recentering is enabled in the settings, the system will recenter the telescope to the target coordinates using plate solving.

   ```mermaid
   graph TD
       A{Recenter Enabled?} -- Yes --> B[Recenter Using Plate Solve]
       A -- No --> C[Skip Recenter]
       B --> D[Proceed to Resume Guider]
       C --> D
   ```

8. **Resume Guider**

   - After the flip (and recentering, if enabled), the guider is resumed to continue tracking the object.

   ```mermaid
   graph TD
       A[Resume Guider] --> B[Proceed to Settle Scope]
   ```

9. **Settle Scope**

   - The system allows the telescope to settle for a configured period before resuming the imaging session.

   ```mermaid
   graph TD
       A[Settle Scope] --> B[Proceed to Rotate Image (if enabled)]
   ```

10. **Rotate Image (if enabled)**

    - The captured image is rotated 180 degrees after the flip, if the setting is enabled.

    ```mermaid
    graph TD
        A{Rotate Image After Flip Enabled?} -- Yes --> B[Rotate Image]
        A -- No --> C[Skip Rotation]
        B --> D[Meridian Flip Process Completed]
        C --> D
    ```

11. **Meridian Flip Process Completed**
    - The process concludes, and the system resumes the regular imaging workflow.

---

## **Code Explanation and Flow Integration**

- **Classes & Methods:**

  - `MeridianFlipVM` class is responsible for managing the entire Meridian Flip process.
  - `AutomatedWorkflow` is a collection that holds steps of the flip, such as stopping the guider, executing the flip, and settling the scope.
  - Each `WorkflowStep` is a task that the system needs to execute, like `PassMeridian`, `Recenter`, or `ResumeAutoguider`.

- **Cancellation Handling:**

  - The process can be canceled by the user at any time, handled by the `CancelCommand`.

- **Error Handling:**
  - If any step fails, the system logs the error and attempts to recover by resuming tracking or guiding.
