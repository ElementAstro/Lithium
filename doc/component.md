# Lithium & Atom Components

Atom的组件机制是元素天文所有组建的基础，正如其名，组件是最基础的，你可以通过组件增强服务器的功能。

## 加载机制

由于c++的特殊性，导致无法像java那样通过类名加载类，所以组件的加载机制是通过加载动态库后获取对应的共享类指针完成的

## 组件组成

每个组件都是有动态库和对应的package.json组成的，如果后需要有需要的话，可以加入package.xml之类的文件来描述组件的依赖关系。

举一个例子：

```txt
    component-demo
    ├── package.json
    └── component-demo.so
```

在这个例子中，我们的component-demo组件中包含一个component-demo.so（可以与文件夹名称不同，而且后缀名根据平台而定）动态库，以及package.json文件。

### package.json

package.json是组件的配置文件，它包含了组件的基本信息，比如组件的名称，版本，作者等。

```json
{
    "name": "example",
    "version": "0.0.0",
    "type": "shared",
    "description": "An example project",
    "license": "LGPL-3.0-or-later",
    "author": "Max Qian",
    "repository": {
        "type": "git",
        "url": "https://github.com/maxqian/cobalt-example.git"
    },
    "bugs": {
        "url": "https://github.com/maxqian/cobalt-example/issues"
    },
    "homepage": "https://github.com/maxqian/cobalt-example",
    "keywords": [
        "atom",
        "example",
        "component",
    ],
    "scripts": {
        "dev": "run",
    },
    "dependencies": ["atom.core"],
    "main": {
        "example1": {
            "func": "getExample1",
            "type" : "shared"
        }
    }
}
```

其中`main`是最为重要的部分，它定义了组件的入口函数，以及组件的类型，目前支持`shared` 、 `inject`、`alone`、`executable`四种类型。

Warning: 每种组件的加载机制不太一样，请注意区分。

### 动态库

动态库的加载具体加载逻辑请参考`atom/module/module_loader.cpp`，在Windows下平台我们使用了`dl-win`库，因此函数形式与Linux下一致。

你可以自己写一个小函数进行测试，后续也会提供对应的构建工具

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

动态库中必须要存在package.json中声明的函数，否则将无法正常加载。

## 组件注册

组件的注册与管理使用`ComponentManager`完成，下面是Lithium服务器中组件管理的目录架构

```txt
    components
    ├── component-finder.cpp
    ├── component_finder.hpp
    ├── component_info.cpp
    ├── component_info.hpp
    ├── component_manager.cpp
    ├── component_manager.hpp
    ├── package_manager.cpp
    └── package_manager.hpp
    ├── project_info.cpp
    └── project_info.hpp
    ├── project_manager.cpp
    └── project_manager.hpp
    ├── sanbox.cpp
    └── sanbox.hpp
```

其中每个组件的功能：
`component-finder`是组件的查找器，主要负责遍历`modules`目录下所有符合条件的文件夹，具体的条件为存在至少一个动态库和package.json文件
`component_info`是组件信息的定义，主要用来处理package.json中的组件信息
`component_manager`是组件管理器，主要用来管理组件的加载和卸载
`package_manager`是包管理器，主要用来管理组件的包
`project_info`是项目信息的定义，主要用来组件之间的依赖关系和防止循环引用
`project_manager`是项目管理器，主要用来管理项目的加载和卸载，可以更加方便的更新组件，提供Git项目管理和基础的CI构建机制
`sanbox`是沙盒，主要用来隔离组件的运行环境，创建`独立组件`的安全运行环境，在Linux下的运行效果较好

### 设备组件

设备组件不同于普通的组件，它的交互机制类似于INDI，但是协议更加简单，后续会单独作为atom.driver驱动库进行实现。

## 组件通信

### 共享组件

共享组件间的通信是使用`MessageBus(Global)`实现的
