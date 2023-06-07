#include "modloader.hpp"

using namespace std;
using namespace nlohmann;

int main() {
    DynamicLoader loader;
    try {
        // 加载动态库
        loader.Load("my_module", "./libmymodule.so");

        // 获取函数指针
        auto fn_ptr = loader.GetFunctionByName("my_function");

        // 调用函数
        json input = {{"arg1", 123}, {"arg2", "hello"}};
        json output = fn_ptr(input);
        cout << output.dump() << endl;

        // 获取类指针
        auto task_ptr = loader.GetClassByName("my_task");

        task_ptr->executeTask({});

        // 调用类方法

        // 卸载动态库
        loader.Unload("my_module");
    } catch (exception& ex) {
        cerr << ex.what() << endl;
    }

    return 0;
}
