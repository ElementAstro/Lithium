2023-6-30 xml.cpp 读取和处理XML文件完全异常，但是由于主协议中并不涉及XML的操作，所以暂时搁置
2023-7-3 compress.cpp 解压缩和压缩zip还是有问题，无法正确的处理文件夹架构
2023-7-3 system.cpp 获取cpu占用在windows环境下失效，获取cpu温度数据不知道该如何转化
2023-7-3 compiler.cpp 编译出现重大问题，似乎是RunShellCommand函数出现问题，暂时放弃
2023-7-3 runner.cpp 任务管理器需要根据最新的task定义进行重写
2023-7-3 device_manager.cpp 需要根据最新的device定义进行重写，虚函数的继承问题尚未解决