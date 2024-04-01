# Lithium & Atom Components: Advanced Concepts

Atom's component mechanism forms the foundation of all elements within the ecosystem. Components, as the name suggests, are fundamental building blocks that enhance the server's functionality.

## Loading Mechanism

Due to the unique nature of C++, unlike Java where classes can be loaded by name, the loading mechanism for components involves loading dynamic libraries and obtaining corresponding shared class pointers.

## Component Structure

Each component consists of a dynamic library and a corresponding `package.json` file. Additional files like `package.xml` can be included to describe component dependencies if necessary.

Example structure:

```plaintext
component-demo
├── package.json
└── component-demo.so
```

In this example, the `component-demo` component includes a `component-demo.so` dynamic library (the filename can differ from the folder name) and a `package.json` file.

## `package.json`

The `package.json` serves as the configuration file for components, containing essential information such as component name, version, author, etc. The `main` section is crucial as it defines the entry function of the component and its type.

## Dynamic Library

The loading logic of dynamic libraries can be found in `atom/module/module_loader.cpp`. On Windows, we utilize the `dl-win` library, so the function form is consistent with Linux platforms.

A simple test function can be written to load the dynamic library:

```cpp
#include <dlfcn.h>
#include <iostream>

int main()
{
    void* handle = dlopen("./component-demo.so", RTLD_LAZY);
    if (handle == nullptr)
    {
        std::cout << dlerror() << std::endl;
        return -1;
    }

    return 0;
}
```

It's essential for the dynamic library to contain the functions declared in `package.json`, or else it won't load correctly.

## Component Registration

Component registration and management are handled by `ComponentManager`. The directory structure for component management in the Lithium server is as follows:

```plaintext
components
├── component-finder.cpp
├── component_finder.hpp
├── component_info.cpp
├── component_info.hpp
├── component_manager.cpp
├── component_manager.hpp
├── package_manager.cpp
├── package_manager.hpp
├── project_info.cpp
└── project_info.hpp
├── project_manager.cpp
└── project_manager.hpp
├── sandbox.cpp
└── sandbox.hpp
```

Each component's function:

- `component-finder`: Responsible for locating components by traversing folders in the `modules` directory that meet specific criteria (at least one dynamic library and one `package.json` file).
- `component_info`: Defines component information to handle data from `package.json`.
- `component_manager`: Manages component loading and unloading.
- `package_manager`: Manages component packages.
- `project_info`: Defines project information, managing dependencies between components and preventing circular references.
- `project_manager`: Manages projects, facilitating component updates, providing Git project management, and basic CI build mechanisms.
- `sandbox`: Isolates component runtime environments, creating a secure execution environment for `standalone components`, particularly effective on Linux.

## Device Components

Device components, unlike regular components, follow a protocol similar to INDI but with a simpler protocol. They will be implemented separately as `atom.driver` for driver library functionality.

## Component Communication

### Shared Components

Communication between shared components is achieved using the `MessageBus (Global)` implementation.

This advanced documentation elaborates on the precise terminology and provides a more detailed explanation of module concepts for better understanding and implementation within the Atom framework.
