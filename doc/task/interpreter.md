# TaskInterpreter Class Overview

The TaskInterpreter class is part of the lithium namespace and is designed to load, manage, and execute scripts in JSON format. It provides a variety of methods to control script execution, manage variables, and handle exceptions. Below is an overview of the class and its key components.

## Overall Flowchart

```mermaid
flowchart TD
    A[TaskInterpreter]
    A1(TaskInterpreterImpl)
    A2(Scripts Management)
    A3(Variables Management)
    A4(Functions Management)
    A5(Execution Control)
    A6(Event Queue)

    A --> A1
    A1 --> A2
    A1 --> A3
    A1 --> A4
    A1 --> A5
    A1 --> A6

    A2 --> B1[loadScript]
    A2 --> B2[unloadScript]
    A2 --> B3[hasScript]
    A2 --> B4[getScript]
    A3 --> C1[setVariable]
    A3 --> C2[getVariable]
    A4 --> D1[registerFunction]
    A4 --> D2[registerExceptionHandler]
    A5 --> E1[execute]
    A5 --> E2[stop]
    A5 --> E3[pause]
    A5 --> E4[resume]
    A5 --> E5[executeStep]
    A6 --> F1[queueEvent]
```

### Explanation

TaskInterpreterImpl: This is a private implementation class that contains the actual data structures and methods used by TaskInterpreter. It includes:
Scripts Management: Manages loading, unloading, and checking of scripts.
Variables Management: Manages setting and getting of variables.
Functions Management: Handles function registration and invocation.
Execution Control: Handles the execution flow of the scripts, including pause, stop, and resume functionalities.
Event Queue: Manages the event queue for the interpreter.

## Key Functions

### `loadScript` Function

```mermaid
flowchart TD
    A[loadScript -name, script]
    B[Lock Mutex]
    C[Store script in scripts_]
    D[Unlock Mutex]
    E[Call prepareScript - script]
    F[Call parseLabels - script]
    G[Exception Handling]

    A --> B
    B --> C
    C --> D
    D --> E
    E --> F
    E --> G[Exception?]
    G -->|Yes| THROW_RUNTIME_ERROR
    G -->|No| F
```

The loadScript function loads a script into the interpreter. It locks the mutex, stores the script in the scripts\_ map, prepares the script by processing it, and then parses any labels. If any errors occur during preparation, an exception is thrown.

### `execute` Function

```mermaid
flowchart TD
    A[execute - scriptName]
    B[Check stopRequested_ flag]
    C[Set isRunning_ to true]
    D[Join existing executionThread_]
    E[Check if script exists]
    F[Start executionThread_]
    G[Execute steps sequentially]
    H[Catch exceptions]
    I[Set isRunning_ to false]
    J[Notify all waiting threads]

    A --> B
    B --> C
    C --> D
    D --> E
    E -->|No| THROW_RUNTIME_ERROR
    E -->|Yes| F
    F --> G
    G --> H[Exception?]
    H -->|Yes| HandleException
    H -->|No| I
    I --> J
```

The execute function starts the execution of a specified script. It checks if the script exists, sets the isRunning\_ flag, and runs the script in a separate thread. If any exception occurs during execution, it is caught and handled appropriately. After the execution is complete, it notifies any waiting threads.

### `executeStep` Function

```mermaid
flowchart TD
    A[executeStep - step, idx, script]
    B[Check stopRequested_ flag]
    C[Determine step type]
    D[Call corresponding executeX function]
    E[Return true]
    F[Catch exceptions]
    G[Log error and handle exception]
    H[Return false]

    A --> B
    B -->|Stopped| H
    B -->|Not Stopped| C
    C --> D
    D --> E
    E -->|Success| E
    E -->|Failure| F
    F --> G
    G --> H
```

The executeStep function handles the execution of individual steps within a script. It checks if execution should stop, determines the step type, and calls the appropriate function to handle the step. If an exception occurs, it logs the error, handles the exception, and returns false to indicate failure.

### `setVariable` Function

```mermaid
flowchart TD
    A[setVariable - name, value, type]
    B[Lock Mutex]
    C[Wait for isRunning_ to be false]
    D[Determine the type of value]
    E[Check if type matches]
    F[Store variable in variables_]
    G[Unlock Mutex]
    H[Exception Handling]

    A --> B
    B --> C
    C --> D
    D --> E[Type Mismatch?]
    E -->|Yes| THROW_RUNTIME_ERROR
    E -->|No| F
    F --> G
    G --> H[Exception?]
    H -->|Yes| THROW_RUNTIME_ERROR
    H -->|No| G
```

The setVariable function sets a variable's value and type in the interpreter. It first locks the mutex and waits until isRunning* is false. It then checks if the provided type matches the value's determined type. If the types match, it stores the variable in variables*. If any exception occurs during the process, it is thrown as a runtime error.

### `executeCall` Function

```mermaid
flowchart TD
    A[executeCall - step]
    B[Extract functionName and params]
    C[Evaluate params]
    D[Check if function exists]
    E[Call function with params]
    F[Store result in targetVariable if specified]
    G[Handle exceptions]

    A --> B
    B --> C
    C --> D[Function exists?]
    D -->|No| THROW_RUNTIME_ERROR
    D -->|Yes| E
    E --> F
    F --> G
```

The executeCall function is used to execute a registered function. It extracts the function name and parameters from the step, looks up the corresponding function in the functions\_ map, and calls it. If a target variable is specified, the return value is stored in that variable.

### `executeCondition` Function

```mermaid
flowchart TD
    A[executeCondition - step, idx, script]
    B[Evaluate condition]
    C[Condition is true?]
    D[Execute true block]
    E[Execute false block if exists]

    A --> B
    B --> C
    C -->|Yes| D
    C -->|No| E
```

The executeCondition function evaluates a condition and executes the appropriate block of code. If the condition is true, the true block is executed; otherwise, the false block is executed (if it exists).

### `executeGoto` Function

```mermaid
flowchart TD
    A[executeGoto - step, idx, script]
    B[Extract label]
    C[Find label in labels_ map]
    D[Update idx to label position]
    E[Handle label not found]

    A --> B
    B --> C
    C -->|Found| D
    C -->|Not Found| E
    E --> THROW_RUNTIME_ERROR
```

The executeGoto function allows jumping to a specific point in the script based on a label. It finds the label in the script and updates the execution index to that position.

### `executeParallel` Function

```mermaid
flowchart TD
    A[executeParallel - step, idx, script]
    B[Create task for each step]
    C[Enqueue tasks in taskPool]
    D[Wait for all tasks to complete]
    E[Return to main execution flow]

    A --> B
    B --> C
    C --> D
    D --> E
```

The executeParallel function executes multiple steps in parallel. It creates a task for each step, enqueues these tasks to the task pool, and waits for all tasks to complete before proceeding.

### `executeTryCatch` Function

```mermaid
flowchart TD
    A[executeTryCatch - step, idx, script]
    B[Execute try block]
    C[Exception occurred?]
    D[Execute catch block]
    E[Continue execution]

    A --> B
    B --> C
    C -->|Yes| D
    C -->|No| E
    D --> E
```

The executeTryCatch function implements error handling. It executes the steps in the try block, and if an exception occurs, it handles the error by executing the steps in the catch block.

### `executeAssign` Function

```mermaid
flowchart TD
    A[executeAssign - step]
    B[Extract variable name]
    C[Evaluate value]
    D[Assign value to variable]
    E[Update variables_ map]

    A --> B
    B --> C
    C --> D
    D --> E
```

The executeAssign function assigns a value to a variable. It evaluates the value specified in the step and stores it in the designated variable.

### `executeLoop` Function

```mermaid
flowchart TD
    A[executeLoop - step, idx, script]
    B[Evaluate loop count]
    C[Start loop iteration]
    D[Execute steps in loop]
    E[Increment iteration counter]
    F[Check if loop is complete]
    G[Return to main execution flow]

    A --> B
    B --> C
    C --> D
    D --> E
    E --> F
    F -->|Complete| G
    F -->|Not Complete| C
```

The executeLoop function handles looping in scripts. It repeats the execution of a block of steps for a specified number of iterations.

### `executeSwitch` Function

```mermaid
flowchart TD
    A[executeSwitch - step, idx, script]
    B[Evaluate switch variable]
    C[Match variable with case blocks]
    D[Execute matched case block]
    E[Execute default block if no match]

    A --> B
    B --> C
    C -->|Matched| D
    C -->|No Match| E
```

The executeSwitch function implements multi-branch conditional logic. It evaluates a variable and executes the corresponding case block or a default block if no cases match.

### `handleException` Function

```mermaid
flowchart TD
    A[handleException - scriptName, e]
    B[Check if exception handler exists for script]
    C[Call exception handler]
    D[Log unhandled exception]

    A --> B
    B -->|Exists| C
    B -->|Not Exists| D
```

The handleException function is called when an unhandled exception occurs during script execution. It checks for a registered exception handler and invokes it if available; otherwise, it logs the error.
