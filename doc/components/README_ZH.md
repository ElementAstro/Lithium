# Lithium & Atom Components

Atom的组件机制是元素天文所有组件的基础，正如其名，组件是最基础的,是一切功能的核心，通过组件可以个性化扩展服务器功能。

## 加载机制

由于C++的特殊性，无法像Java那样通过类名加载类，更加不能像python那样导入文件即可。目前采用的组件加载机制是加载动态库后获取指定共享类指针。

## 组件组成

每个组件由动态库和对应的`package.json`组成，如果需要的话，可以加入`package.xml`等文件来描述组件的依赖关系。

> [!IMPORTANT]
> 优先支持`package.json`，不一定会兼容xml格式

举一个例子：

> [!NOTE]
> 一个文件夹中可以有多个不同名称的动态，只需要在`package.json`中指定即可

```txt
    component-demo
    ├── package.json
    └── component-demo.so
    └── component-demo-2.so
```

在这个例子中，`component-demo`组件包含一个或多个动态库（后缀名根据平台而定，__需要特别注意的是，Mingw64环境需要使用so作为后缀，虽然也是Windows环境__）和`package.json`文件。

### package.json

`package.json`是组件的配置文件，包含组件的基本信息，比如组件的名称、版本、作者等。

```json
{
    "name": "atom.io",
    "version": "1.0.0",
    "type": "shared",
    "description": "Atom IO Module",
    "license": "GPL-3.0-or-later",
    "author": "Max Qian",
    "repository": {
        "type": "git",
        "url": "https://github.com/ElementAstro/Lithium"
    },
    "bugs": {
        "type": "git",
        "url": "https://github.com/ElementAstro/Lithium/issues"
    },
    "homepage": {
        "type": "git",
        "url": "https://github.com/ElementAstro/Lithium"
    },
    "keywords": [
        "lithium",
        "config"
    ],
    "scripts": {
        "build": "cmake --build-type=Release -- -j 4",
        "lint": "clang-format -i src/*.cpp src/*.h"
    },
    "modules": [
        {
            "name": "io",
            "entry": "getInstance"
        }
    ]
}

```

+ `type`声明组件类型，分为`shared`和`standalone`两种，`shared`表示共享组件，`standalone`表示独立组件，加载机制完全不同！
+ `scripts`是脚本，用于构建和格式化代码。
+ `modules`是一个json数组，其中包括若干组件信息，包括组件的名称（是注册在模块管理器中的名称）、和入口函数（需要与动态中完全相同，可以是C++翻译之后的名称）。

__Warning__: 整个`package.json`文件必须包含`modules`字段，否则无法正常加载。

### 动态库

加载逻辑请参考`atom/module/module_loader.cpp`，在Windows平台我们使用了`dl-win`库（后续会添加Windows原生API的支持），因此函数形式与Linux下保持一致。

你可以自己写一个小函数进行测试，后续也会提供对应的构建工具。

```cpp
// example.cpp
#include <iostream>

// 这个函数将被导出到动态库
extern "C" void helloWorld() {
    std::cout << "Hello, World!" << std::endl;
}

// 这个类将不会被导出，因为它是C++特定的
class MyClass {
public:
    void sayHello() {
        std::cout << "MyClass says hello!" << std::endl;
    }
};

// 这个函数将被导出，它将创建一个MyClass的实例并调用其sayHello方法
extern "C" void callMyClass() {
    MyClass myObj;
    myObj.sayHello();
}
```

## 组件注册

组件的注册与管理使用`ComponentManager`完成，下面是Lithium服务器中组件管理的目录架构：

```txt
    components
    ├── addons.cpp
    ├── addons.hpp
    ├── compiler.cpp
    ├── compiler.hpp
    ├── component.hpp
    ├── dependency.cpp
    ├── dependency.hpp
    ├── loader.cpp
    ├── loader.hpp
    ├── manager.cpp
    ├── manager.hpp
    ├── module.cpp
    ├── sandbox.cpp
    ├── sandbox.hpp
    ├── sort.cpp
    ├── sort.hpp
```
