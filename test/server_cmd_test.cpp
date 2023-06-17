#include <string>
#include <unordered_map>
#include <iostream>

class CommandDispatcher {
 public:
  using HandlerFunc = void (*)(const std::string& data);

  void RegisterHandler(const std::string& name, HandlerFunc handler) {
    auto hash_value = Djb2Hash(name.c_str());
    handlers_[hash_value] = handler;
  }

  bool HasHandler(const std::string& name) {
    auto hash_value = Djb2Hash(name.c_str());
    return handlers_.find(hash_value) != handlers_.end();
  }

  void Dispatch(const std::string& name, const std::string& data) {
    auto hash_value = Djb2Hash(name.c_str());
    auto it = handlers_.find(hash_value);
    if (it != handlers_.end()) {
      it->second(data);
    } else {
      std::cerr << "Error: Unknown command " << name << std::endl;
    }
  }

 private:
  std::unordered_map<std::size_t, HandlerFunc> handlers_;

  static std::size_t Djb2Hash(const char* str) {
    std::size_t hash = 5381;
    int c;
    while ((c = *str++) != 0) {
      hash = ((hash << 5) + hash) + c;
    }
    return hash;
  }
};

void HandleFoo(const std::string& data) {
  // 处理 foo 指令
  std::cout << "Foo() is called" << std::endl;
}

void HandleBar(const std::string& data) {
    std::cout << "Bar() is called" << std::endl;
  // 处理 bar 指令
}

int main(int argc, char* argv[]) {
  CommandDispatcher dispatcher;

  dispatcher.RegisterHandler("foo", &HandleFoo);
  dispatcher.RegisterHandler("bar", &HandleBar);

  // 检查是否存在指令 "foo"
  if (dispatcher.HasHandler("foo")) {
    // 接收到 "foo" 指令
    dispatcher.Dispatch("foo", "foo data");
  } else {
    std::cerr << "Error: Cannot find command \"foo\"" << std::endl;
  }

  if (dispatcher.HasHandler("bar")) {
    // 接收到 "foo" 指令
    dispatcher.Dispatch("bar", "foo data");
  } else {
    std::cerr << "Error: Cannot find command \"foo\"" << std::endl;
  }

  // 接收到未知指令
  dispatcher.Dispatch("unknown", "");
  
  return 0;
}
