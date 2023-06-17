#include <iostream>
#include <lua.hpp>
#include <stdexcept>

// 定义 mul 函数
int mul(lua_State *L)
{
    int a = lua_tonumber(L, 1);
    int b = lua_tonumber(L, 2);
    lua_pushnumber(L, a * b);
    return 1;
}

class LuaScriptManager
{
public:
    LuaScriptManager()
    {
        L = luaL_newstate();
        luaL_openlibs(L);
        // 注册 mul 函数
        lua_register(L, "mul", mul);
    }

    ~LuaScriptManager()
    {
        lua_close(L);
    }

    void LoadScript(const std::string &filename)
    {
        if (luaL_loadfile(L, filename.c_str()))
        {
            const char *error = lua_tostring(L, -1);
            lua_pop(L, 1);
            throw std::runtime_error("Failed to load script '" + filename + "': " + error);
        }
        if (lua_pcall(L, 0, 0, 0))
        {
            const char *error = lua_tostring(L, -1);
            lua_pop(L, 1);
            throw std::runtime_error("Failed to execute script '" + filename + "': " + error);
        }
    }

    void UnloadScript(const std::string &filename)
    {
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "loaded");
        lua_getfield(L, -1, filename.c_str());
        if (!lua_isnil(L, -1))
        {
            lua_pushnil(L);
            lua_setfield(L, -2, filename.c_str());
        }
        lua_pop(L, 3); // 弹出 3 个元素：package、loaded 和 filename
    }

    bool ScriptExists(const std::string &filename)
    {
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "loaded");
        lua_getfield(L, -1, filename.c_str());
        bool exists = !lua_isnil(L, -1);
        lua_pop(L, 3); // 弹出 3 个元素：package、loaded 和 filename
        return exists;
    }

    template <typename T, typename... Args>
    int PushArgs(lua_State *L, T arg, Args... args)
    {
        int nargs = PushArgs(L, args...);
        PushArg(L, arg);
        return nargs + 1;
    }

    int PushArgs(lua_State *L)
    {
        return 0;
    }

    template <typename... Args>
    void CallFunction(const std::string &funcName, const std::string &filename, Args... args)
    {
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);

        if (luaL_loadfile(L, filename.c_str()) || lua_pcall(L, 0, 0, 0))
        {
            printf("Error: %s", lua_tostring(L, -1));
            lua_close(L);
            return;
        }

        lua_getglobal(L, funcName.c_str());
        int nargs = PushArgs(L, args...);

        if (lua_pcall(L, nargs, 0, 0) != 0)
        {
            printf("Error: %s", lua_tostring(L, -1));
        }

        lua_close(L);
    }

    template <typename T>
    void RegisterVariable(const std::string &name, T value)
    {
        PushArg(L, value);
        lua_setglobal(L, name.c_str());
    }

    template <typename T>
    void RegisterClass(const std::string &name)
    {
        // 创建 metatable
        lua_newtable(L);
        // 注册 __index 元方法，使得对象能够访问类的成员函数
        lua_pushstring(L, "__index");
        lua_newtable(L);
        std::size_t size = sizeof(T) / sizeof(void *) - 1;
        for (std::size_t i = 0; i < size; ++i)
        {
            typedef void (*pFunc)(void *, void *);
            pFunc func = ((*reinterpret_cast<pFunc *>(reinterpret_cast<char *>(new T()) + i * sizeof(void *))));
            auto funcName = (*reinterpret_cast<const char **>(reinterpret_cast<char *>(new T()) + size * sizeof(void *) + i * sizeof(char *)));
            lua_pushstring(L, funcName);
            lua_pushlightuserdata(L, reinterpret_cast<void *>(func));
            lua_pushcclosure(L, &LuaScriptManager::ClassMethodCaller<T>, 1);
            lua_settable(L, -3);
        }
        lua_settable(L, -3);
        // 注册 __gc 元方法，使得对象在 Lua 中被回收时调用类的析构函数
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, &LuaScriptManager::ClassGC<T>);
        lua_settable(L, -3);
        // 将 metatable 压入栈中

        // 将 metatable 压入栈中
        lua_setfield(L, LUA_REGISTRYINDEX, name.c_str());
    }

    template <typename T>
    static int ClassMethodCaller(lua_State *L)
    {
        auto func = reinterpret_cast<void (*)(void *, void *)>(lua_touserdata(L, lua_upvalueindex(1)));
        T *obj = reinterpret_cast<T *>(lua_touserdata(L, 1));
        void *arg = reinterpret_cast<void *>(lua_touserdata(L, 2));
        func(obj, arg);
        return 0;
    }

    template <typename T>
    static int ClassGC(lua_State *L)
    {
        T *obj = reinterpret_cast<T *>(lua_touserdata(L, 1));
        obj->~T();
        return 0;
    }

private:
    lua_State *L;

    // 内部方法，将传入的参数推入栈中
    void PushArg(lua_State *L, int arg)
    {
        lua_pushnumber(L, arg);
    }

    void PushArg(lua_State *L, double arg)
    {
        lua_pushnumber(L, arg);
    }

    void PushArg(lua_State *L, const char *arg)
    {
        lua_pushstring(L, arg);
    }

    template <typename T>
    void PushArg(lua_State *L, T *arg, typename std::enable_if<std::is_class<T>::value>::type * = 0)
    {
        // 创建 userdata 类型的值，并将 MyClass 对象的指针存入其中
        void *ud = lua_newuserdata(L, sizeof(T));
        new (ud) T(*arg);

        // 从注册表中取出 MyClass 的 metatable，并将其设置为 userdata 的 metatable
        luaL_getmetatable(L, typeid(T).name());
        lua_setmetatable(L, -2);
    }
};

// 以下是一个例子，演示如何定义一个类，并在 Lua 中使用该类：

class MyClass
{
public:
    MyClass() {}

    void SayHello(const std::string &name) { std::cout << "Hello, " << name << "!" << std::endl; }
    int Add(int x, int y) { return x + y; }
};

int main()
{
    try
    {
        LuaScriptManager lua_mgr;

        MyClass *my_obj;
        // 将 my_obj 注册到 Lua 中
        lua_mgr.RegisterVariable<MyClass *>("my_obj", my_obj);

        // 将 MyClass 类注册到 Lua 中

        lua_mgr.LoadScript("test.lua");

        if (lua_mgr.ScriptExists("hello_world()"))
        {
            lua_mgr.CallFunction("hello_world", "test.lua");
        }
        else
        {
            std::cerr << "Function 'hello_world' does not exist" << std::endl;
        }

        lua_mgr.CallFunction("print_int", "test.lua", 123);
        lua_mgr.CallFunction("print_float", "test.lua", 3.14);
        lua_mgr.CallFunction("print_string", "test.lua", "hello");

        lua_mgr.LoadScript("test2.lua");

        lua_mgr.CallFunction("test_my_class", "test2.lua");
        lua_mgr.CallFunction("test_cpp_func", "test2.lua");

        lua_mgr.UnloadScript("test.lua");
        if (lua_mgr.ScriptExists("hello_world()"))
        { // 前面已经卸载了 test.lua，因此这里应该返回 false
            std::cerr << "Error: script 'test.lua' still exists" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
