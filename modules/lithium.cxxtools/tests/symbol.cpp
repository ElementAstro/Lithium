#include <gmock/gmock.h>
#include <gtest/gtest.h>


#include "symbol.cpp"  // Include the implementation file

using ::testing::_;
using ::testing::Return;

// Mock for exec function
std::string mock_exec(const char* cmd) {
    return "  1: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND \n"
           "  2: 0000000000000000     0 FUNC    GLOBAL DEFAULT  UND printf\n";
}

// Mock for demangle function
namespace atom {
namespace meta {
class DemangleHelper {
public:
    static std::string demangle(const std::string& name) {
        return "demangled_" + name;
    }
};
}  // namespace meta
}  // namespace atom

class AnalyzeLibraryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Redirect exec to mock_exec
        exec = mock_exec;
    }

    void TearDown() override {
        // Reset exec to original function if needed
    }
};

TEST_F(AnalyzeLibraryTest, ValidInputCSV) {
    std::string libraryPath = "dummy_path";
    std::string outputFormat = "csv";
    int threadCount = 2;

    analyzeLibrary(libraryPath, outputFormat, threadCount);

    std::ifstream file("symbols.csv");
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);  // Skip header
    std::getline(file, line);
    EXPECT_EQ(line, "0000000000000000,NOTYPE,LOCAL,DEFAULT,UND,demangled_");

    file.close();
}

TEST_F(AnalyzeLibraryTest, ValidInputJSON) {
    std::string libraryPath = "dummy_path";
    std::string outputFormat = "json";
    int threadCount = 2;

    analyzeLibrary(libraryPath, outputFormat, threadCount);

    std::ifstream file("symbols.json");
    ASSERT_TRUE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("\"demangled_name\": \"demangled_\""),
              std::string::npos);

    file.close();
}

TEST_F(AnalyzeLibraryTest, ValidInputYAML) {
    std::string libraryPath = "dummy_path";
    std::string outputFormat = "yaml";
    int threadCount = 2;

    analyzeLibrary(libraryPath, outputFormat, threadCount);

    std::ifstream file("symbols.yaml");
    ASSERT_TRUE(file.is_open());

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_NE(content.find("demangled_name: demangled_"), std::string::npos);

    file.close();
}

TEST_F(AnalyzeLibraryTest, InvalidThreadCount) {
    std::string libraryPath = "dummy_path";
    std::string outputFormat = "csv";
    int threadCount = -1;

    EXPECT_THROW(analyzeLibrary(libraryPath, outputFormat, threadCount),
                 std::invalid_argument);
}

TEST_F(AnalyzeLibraryTest, UnsupportedOutputFormat) {
    std::string libraryPath = "dummy_path";
    std::string outputFormat = "xml";
    int threadCount = 2;

    EXPECT_THROW(analyzeLibrary(libraryPath, outputFormat, threadCount),
                 std::invalid_argument);
}
