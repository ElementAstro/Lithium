#include "addon/loader.hpp"
#include "gtest/gtest.h"

using json = nlohmann::json;
using namespace lithium;

class ModuleLoaderTest : public ::testing::Test {
protected:
    std::shared_ptr<ModuleLoader> loader;

    void SetUp() override { loader = ModuleLoader::createShared(); }

    void TearDown() override { loader.reset(); }
};

TEST_F(ModuleLoaderTest, LoadAndUnloadModule) {
    std::string modulePath =
        "path/to/your/module.so";  // Replace with actual path
    std::string moduleName = "test_module";

    ASSERT_FALSE(loader->HasModule(moduleName));

    bool loadResult = loader->LoadModule(modulePath, moduleName);
    ASSERT_TRUE(loadResult);
    ASSERT_TRUE(loader->HasModule(moduleName));

    bool unloadResult = loader->UnloadModule(moduleName);
    ASSERT_TRUE(unloadResult);
    ASSERT_FALSE(loader->HasModule(moduleName));
}

TEST_F(ModuleLoaderTest, LoadNonExistentModule) {
    std::string modulePath = "path/to/nonexistent/module.so";
    std::string moduleName = "nonexistent_module";

    bool loadResult = loader->LoadModule(modulePath, moduleName);
    ASSERT_FALSE(loadResult);
    ASSERT_FALSE(loader->HasModule(moduleName));
}

TEST_F(ModuleLoaderTest, EnableAndDisableModule) {
    std::string modulePath =
        "path/to/your/module.so";  // Replace with actual path
    std::string moduleName = "test_module";

    loader->LoadModule(modulePath, moduleName);
    ASSERT_TRUE(loader->EnableModule(moduleName));
    ASSERT_TRUE(loader->IsModuleEnabled(moduleName));

    ASSERT_TRUE(loader->DisableModule(moduleName));
    ASSERT_FALSE(loader->IsModuleEnabled(moduleName));

    loader->UnloadModule(moduleName);
}

TEST_F(ModuleLoaderTest, GetModuleInformation) {
    std::string modulePath =
        "path/to/your/module.so";  // Replace with actual path
    std::string moduleName = "test_module";

    loader->LoadModule(modulePath, moduleName);

    std::string version = loader->GetModuleVersion(moduleName);
    std::string description = loader->GetModuleDescription(moduleName);
    std::string author = loader->GetModuleAuthor(moduleName);
    std::string license = loader->GetModuleLicense(moduleName);

    ASSERT_FALSE(version.empty());
    ASSERT_FALSE(description.empty());
    ASSERT_FALSE(author.empty());
    ASSERT_FALSE(license.empty());

    loader->UnloadModule(moduleName);
}

TEST_F(ModuleLoaderTest, GetFunction) {
    std::string modulePath =
        "path/to/your/module.so";  // Replace with actual path
    std::string moduleName = "test_module";
    std::string functionName =
        "YourFunction";  // Replace with actual function name

    loader->LoadModule(modulePath, moduleName);

    auto func = loader->GetFunction<int (*)()>(moduleName, functionName);
    ASSERT_NE(func, nullptr);

    int result = func();
    ASSERT_EQ(result, 42);  // Replace with expected result

    loader->UnloadModule(moduleName);
}

TEST_F(ModuleLoaderTest, GetInstance) {
    std::string modulePath =
        "path/to/your/module.so";  // Replace with actual path
    std::string moduleName = "test_module";
    std::string instanceFunctionName =
        "GetInstance";  // Replace with actual instance function name

    loader->LoadModule(modulePath, moduleName);

    json config = {{"key", "value"}};  // Replace with actual config
    auto instance = loader->GetInstance<std::shared_ptr<int>>(
        moduleName, config, instanceFunctionName);
    ASSERT_NE(instance, nullptr);

    loader->UnloadModule(moduleName);
}

TEST_F(ModuleLoaderTest, UnloadAllModules) {
    std::string modulePath1 =
        "path/to/your/module1.so";  // Replace with actual path
    std::string moduleName1 = "test_module1";
    std::string modulePath2 =
        "path/to/your/module2.so";  // Replace with actual path
    std::string moduleName2 = "test_module2";

    loader->LoadModule(modulePath1, moduleName1);
    loader->LoadModule(modulePath2, moduleName2);

    ASSERT_TRUE(loader->HasModule(moduleName1));
    ASSERT_TRUE(loader->HasModule(moduleName2));

    ASSERT_TRUE(loader->UnloadAllModules());
    ASSERT_FALSE(loader->HasModule(moduleName1));
    ASSERT_FALSE(loader->HasModule(moduleName2));
}

TEST_F(ModuleLoaderTest, GetAllExistedModules) {
    std::string modulePath1 =
        "path/to/your/module1.so";  // Replace with actual path
    std::string moduleName1 = "test_module1";
    std::string modulePath2 =
        "path/to/your/module2.so";  // Replace with actual path
    std::string moduleName2 = "test_module2";

    loader->LoadModule(modulePath1, moduleName1);
    loader->LoadModule(modulePath2, moduleName2);

    auto modules = loader->GetAllExistedModules();
    ASSERT_EQ(modules.size(), 2);
    ASSERT_NE(std::find(modules.begin(), modules.end(), moduleName1),
              modules.end());
    ASSERT_NE(std::find(modules.begin(), modules.end(), moduleName2),
              modules.end());

    loader->UnloadAllModules();
}
