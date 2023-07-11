#include "../src/module/compiler.hpp"

int main()
{
    // 创建编译器对象
    Compiler compiler;

    // C++ 代码
    std::string code = R"(
        #include <iostream>
        void HelloWorld() {
            std::cout << "Hello, World!" << std::endl;
        }
    )";

    // 模块名和函数名
    std::string moduleName = "myModule";
    std::string functionName = "HelloWorld";

    // 编译并生成共享库
    if (compiler.CompileToSharedLibrary(code, moduleName, functionName))
    {
        std::cout << "Module " << moduleName << "::" << functionName << " compiled successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to compile module." << std::endl;
    }

    return 0;
}
