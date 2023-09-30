Hydrogen Server - Next Generation Device Server
===============================================

本项目为INDI Server扩展项目，完全支持原版INDI设备，并增加了对Hydrogen设备的支持。以下是对该项目进行的主要修改和扩展：

+ 拆分单文件：将原先的单一文件拆分成多个文件，这有助于后续的代码维护和管理。通过拆分文件，可以更好地组织代码，使得逻辑结构更清晰。

+ 使用最新的C++特性：在重写老旧代码的过程中，使用了部分最新的C++特性。这可能包括使用更现代化的语法和标准库特性，使代码更加简洁、高效和易读。

+ 增加对Windows的支持：除了原版INDI设备的支持外，Hydrogen Server还增加了对Windows操作系统的支持。这意味着可以在Windows平台上运行Hydrogen Server，并与Hydrogen设备进行交互。

+ 逻辑优化：对部分逻辑进行了优化，以提升程序的性能和效率。通过对代码进行改进和精简，可以改善系统的响应速度，并提供更好的用户体验。

## 文件目录

```
.
├── CMakeLists.txt
├── README.md
├── client_info.cpp
├── client_info.hpp
├── concurrent.cpp
├── concurrent.hpp
├── driver_info.cpp
├── driver_info.hpp
├── fifo_server.cpp
├── fifo_server.hpp
├── hydrogen_server.cpp
├── hydrogen_server.hpp
├── io.cpp
├── io.hpp
├── local_driver.cpp
├── local_driver.hpp
├── message.cpp
├── message.hpp
├── message_queue.cpp
├── message_queue.hpp
├── property.cpp
├── property.hpp
├── remote_driver.cpp
├── remote_driver.hpp
├── serialize.cpp
├── serialize.hpp
├── signal.cpp
├── signal.hpp
├── tcp_server.cpp
├── tcp_server.hpp
├── time.cpp
├── time.hpp
├── xml_util.cpp
└── xml_util.hpp
```
