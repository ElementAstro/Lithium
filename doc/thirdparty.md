# 需要使用到的第三方软件

## 设备控制

### ASCOM：Windows下的设备控制

链接： [官网](https://www.ascom-standards.org/)

由于ASCOM本体使用C#编写，因此不能直接通过C++控制，虽然可以模仿PHD2直接操作串口控制，但是这样的可用性很低。因此使用ASCOM Remote作为中转服务器。

链接： [ASCOM Remote](https://github.com/ASCOMInitiative/ASCOMRemote)

ASCOM Remote的通信基于http协议，消息格式为json，在服务器启动后会暴露指定的端口。

可以模仿的实现：[Python下的ASCOM Remote客户端](https://github.com/ASCOMInitiative/alpyca)


#### 使用流程

用户启动ASCOMRemote服务器并选择需要使用的具体设备 -> 尝试扫描服务器，发现后建立连接 -> 根据API获取所有设备的信息 -> 连接完成

参考文档：
[ASCOM文档中心](https://www.ascom-standards.org/Documentation/Index.htm)
[Alpaca指令定义](https://download.ascom-standards.org/docs/AlpacaIntroduction.pdf)
[Alpyca文档](https://ascom-standards.org/alpyca/alpyca.pdf)
[PHD2中的ASCOM客户端](https://github.com/OpenPHDGuiding/phd2/blob/master/cam_ascom.cpp)

### INDI: Linux下的设备控制

链接： [官网](https://github.com/indilib/)

使用纯正C/C++编写，使用基于XML的tcp通信。__需要得到优先支持__

[核心库](https://github.com/indilib/indi)
[第三方驱动](https://github.com/indilib/indi-3rdparty)

#### 使用流程

有用户选择对应的驱动大类 -> 启动INDI服务器 -> (INDI服务器会自动扫描属于已经选择的驱动大类下的具体设备) -> 连接服务器 -> (第一次连接INDI会返回所有已有设备的信息，此时设备仍然未连接) -> 用户选择具体的设备型号 -> 发送连接指令 -> INDI服务器与具体设备连接 -> 连接流程结束


#### 参考资料

[Kstars中的INDI客户端](https://github.com/KDE/kstars/tree/master/kstars/indi)
[PHD2中的INDI客户端](https://github.com/OpenPHDGuiding/phd2/blob/master/cam_indi.cpp)
[Python下的INDI客户端](https://github.com/indilib/pyindi-client)

## 导星

### PHD2

链接： [官网](https://github.com/OpenPHDGuiding/phd2)

[服务器接口](https://github.com/OpenPHDGuiding/phd2/wiki/EventMonitoring)

## 解析

### Astrometry.net

链接： [官网](https://github.com/dstndstn/astrometry.net)

文档： [官方文档](https://astrometry.net/doc/readme.html)

命令行工具

### Astap

[官网](https://www.hnsky.org/astap.htm)

命令行工具