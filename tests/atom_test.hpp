#ifndef ATOM_TEST_HPP
#define ATOM_TEST_HPP

#include <chrono>
#include <cmath>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#else
#include "atom/type/json.hpp"
#endif

#include "macro.hpp"

// 一个简单的测试框架
namespace atom::test {

struct TestCase {
    std::string name;
    std::function<void()> func;
    bool skip = false;                      // 是否跳过测试
    bool async = false;                     // 是否异步运行
    double timeLimit = 0.0;                 // 测试时间阈值
    std::vector<std::string> dependencies;  // 依赖的测试
} ATOM_ALIGNAS(128);

struct TestResult {
    std::string name;
    bool passed;
    bool skipped;
    std::string message;
    double duration;
    bool timedOut;
} ATOM_ALIGNAS(128);

struct TestSuite {
    std::string name;
    std::vector<TestCase> testCases;
} ATOM_ALIGNAS(64);

ATOM_INLINE auto getTestSuites() -> std::vector<TestSuite>& {
    static std::vector<TestSuite> testSuites;
    return testSuites;
}

ATOM_INLINE auto getTestMutex() -> std::mutex& {
    static std::mutex testMutex;
    return testMutex;
}

ATOM_INLINE void registerTest(const std::string& name,
                              std::function<void()> func, bool async = false,
                              double time_limit = 0.0, bool skip = false,
                              std::vector<std::string> dependencies = {}) {
    getTestSuites().push_back({"",
                               {TestCase{name, std::move(func), skip, async,
                                         time_limit, dependencies}}});
}

ATOM_INLINE void registerSuite(const std::string& suite_name,
                               std::vector<TestCase> cases) {
    getTestSuites().push_back({suite_name, std::move(cases)});
}

ATOM_INLINE auto operator""_test(const char* name, std::size_t size) {
    return [name](std::function<void()> func, bool async = false,
                  double time_limit = 0.0, bool skip = false,
                  std::vector<std::string> const& dependencies = {}) {
        return TestCase{name,  std::move(func), skip,
                        async, time_limit,      dependencies};
    };
}

struct TestStats {
    int totalTests = 0;
    int totalAsserts = 0;
    int passedAsserts = 0;
    int failedAsserts = 0;
    int skippedTests = 0;
    std::vector<TestResult> results;
} ATOM_ALIGNAS(64);

ATOM_INLINE auto getTestStats() -> TestStats& {
    static TestStats stats;
    return stats;
}

using Hook = std::function<void()>;

struct Hooks {
    Hook beforeEach;
    Hook afterEach;
    Hook beforeAll;
    Hook afterAll;
} ATOM_ALIGNAS(128);

ATOM_INLINE auto getHooks() -> Hooks& {
    static Hooks hooks;
    return hooks;
}

ATOM_INLINE void printColored(const std::string& text,
                              const std::string& color_code) {
    std::cout << "\033[" << color_code << "m" << text << "\033[0m";
}

struct Timer {
    std::chrono::high_resolution_clock::time_point startTime;

    Timer() { reset(); }

    void reset() { startTime = std::chrono::high_resolution_clock::now(); }

    [[nodiscard]] auto elapsed() const -> double {
        return std::chrono::duration<double, std::milli>(
                   std::chrono::high_resolution_clock::now() - startTime)
            .count();
    }
};

// 支持多种格式的结果导出（JSON, XML, HTML）
ATOM_INLINE void exportResults(const std::string& filename,
                               const std::string& format) {
    auto& stats = getTestStats();
    nlohmann::json jsonReport;

    jsonReport["total_tests"] = stats.totalTests;
    jsonReport["total_asserts"] = stats.totalAsserts;
    jsonReport["passed_asserts"] = stats.passedAsserts;
    jsonReport["failed_asserts"] = stats.failedAsserts;
    jsonReport["skipped_tests"] = stats.skippedTests;
    jsonReport["test_results"] = nlohmann::json::array();

    for (const auto& result : stats.results) {
        nlohmann::json jsonResult;
        jsonResult["name"] = result.name;
        jsonResult["passed"] = result.passed;
        jsonResult["skipped"] = result.skipped;
        jsonResult["message"] = result.message;
        jsonResult["duration"] = result.duration;
        jsonResult["timed_out"] = result.timedOut;
        jsonReport["test_results"].push_back(jsonResult);
    }

    if (format == "json") {
        std::ofstream file(filename + ".json");
        file << jsonReport.dump(4);
        file.close();
        std::cout << "Test report saved to " << filename << ".json\n";
    } else if (format == "xml") {
        std::ofstream file(filename + ".xml");
        file << "<?xml version=\"1.0\"?>\n<testsuite>\n";
        file << "  <total_tests>" << stats.totalTests << "</total_tests>\n";
        file << "  <passed_asserts>" << stats.passedAsserts
             << "</passed_asserts>\n";
        file << "  <failed_asserts>" << stats.failedAsserts
             << "</failed_asserts>\n";
        file << "  <skipped_tests>" << stats.skippedTests
             << "</skipped_tests>\n";
        for (const auto& result : stats.results) {
            file << "  <testcase name=\"" << result.name << "\">\n";
            file << "    <passed>" << (result.passed ? "true" : "false")
                 << "</passed>\n";
            file << "    <message>" << result.message << "</message>\n";
            file << "    <duration>" << result.duration << "</duration>\n";
            file << "    <timed_out>" << (result.timedOut ? "true" : "false")
                 << "</timed_out>\n";
            file << "  </testcase>\n";
        }
        file << "</testsuite>\n";
        file.close();
        std::cout << "Test report saved to " << filename << ".xml\n";
    } else if (format == "html") {
        std::ofstream file(filename + ".html");
        file << "<!DOCTYPE html><html><head><title>Test Report</title></head>"
                "<body>\n";
        file << "<h1>Test Report</h1>\n";
        file << "<p>Total Tests: " << stats.totalTests << "</p>\n";
        file << "<p>Passed Asserts: " << stats.passedAsserts << "</p>\n";
        file << "<p>Failed Asserts: " << stats.failedAsserts << "</p>\n";
        file << "<p>Skipped Tests: " << stats.skippedTests << "</p>\n";
        file << "<ul>\n";
        for (const auto& result : stats.results) {
            file << "  <li><strong>" << result.name << "</strong>: "
                 << (result.passed ? "<span style='color:green;'>PASSED</span>"
                                   : "<span style='color:red;'>FAILED</span>")
                 << " (" << result.duration << " ms)</li>\n";
        }
        file << "</ul>\n";
        file << "</body></html>";
        file.close();
        std::cout << "Test report saved to " << filename << ".html\n";
    }
}

// 执行单个测试用例
ATOM_INLINE void runTestCase(const TestCase& test, int retryCount = 0) {
    auto& stats = getTestStats();
    Timer timer;

    if (test.skip) {
        printColored("SKIPPED\n", "1;33");
        std::lock_guard lock(getTestMutex());
        stats.skippedTests++;
        stats.totalTests++;
        stats.results.push_back(
            {std::string(test.name), false, true, "Test Skipped", 0.0, false});
        return;
    }

    std::string resultMessage;
    bool passed = false;
    bool timedOut = false;

    try {
        timer.reset();
        if (test.async) {
            auto future = std::async(std::launch::async, test.func);
            if (test.timeLimit > 0 && future.wait_for(std::chrono::milliseconds(
                                          static_cast<int>(test.timeLimit))) ==
                                          std::future_status::timeout) {
                timedOut = true;
                throw std::runtime_error("Test timed out");
            }
            future.get();
        } else {
            test.func();
        }
        passed = true;
        resultMessage = "PASSED";
    } catch (const std::exception& e) {
        resultMessage = e.what();
        if (retryCount > 0) {
            printColored("Retrying test...\n", "1;33");
            runTestCase(test, retryCount - 1);
            return;
        }
    }

    std::lock_guard lock(getTestMutex());
    stats.totalTests++;
    stats.results.push_back({std::string(test.name), passed, false,
                             resultMessage, timer.elapsed(), timedOut});

    if (timedOut) {
        printColored(resultMessage + " (TIMEOUT)", "1;31");
    } else {
        printColored(resultMessage, passed ? "1;32" : "1;31");
    }
    std::cout << " (" << timer.elapsed() << " ms)\n";
}

// 支持并行执行测试
ATOM_INLINE void runTestsInParallel(const std::vector<TestCase>& tests,
                                    int numThreads = 4) {
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &tests, numThreads]() {
            for (size_t j = i; j < tests.size(); j += numThreads) {
                runTestCase(tests[j]);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

ATOM_INLINE void runAllTests(int retryCount = 0, bool parallel = false,
                             int numThreads = 4);

ATOM_INLINE void runTests(int argc, char* argv[]) {
    int retryCount = 0;
    bool parallel = false;
    int numThreads = 4;
    std::string exportFormat;
    std::string exportFilename;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--retry" && i + 1 < argc) {
            retryCount = std::stoi(argv[++i]);
        } else if (arg == "--parallel" && i + 1 < argc) {
            parallel = true;
            numThreads = std::stoi(argv[++i]);
        } else if (arg == "--export" && i + 2 < argc) {
            exportFormat = argv[++i];
            exportFilename = argv[++i];
        }
    }

    runAllTests(retryCount, parallel, numThreads);

    if (!exportFormat.empty() && !exportFilename.empty()) {
        exportResults(exportFilename, exportFormat);
    }
}

// 过滤测试用例
ATOM_INLINE auto filterTests(const std::regex& pattern)
    -> std::vector<TestCase> {
    std::vector<TestCase> filtered;
    for (const auto& suite : getTestSuites()) {
        for (const auto& test : suite.testCases) {
            if (std::regex_search(test.name.begin(), test.name.end(),
                                  pattern)) {
                filtered.push_back(test);
            }
        }
    }
    return filtered;
}

// 根据依赖关系排序测试
ATOM_INLINE auto sortTestsByDependencies(const std::vector<TestCase>& tests)
    -> std::vector<TestCase> {
    std::map<std::string, TestCase> testMap;
    std::vector<TestCase> sortedTests;
    std::set<std::string> processed;

    for (const auto& test : tests) {
        testMap[test.name] = test;
    }

    std::function<void(const TestCase&)> resolveDependencies;
    resolveDependencies = [&](const TestCase& test) {
        if (!processed.contains(std::string(test.name))) {
            for (const auto& dep : test.dependencies) {
                if (testMap.find(dep) != testMap.end()) {
                    resolveDependencies(testMap[dep]);
                }
            }
            processed.insert(std::string(test.name));
            sortedTests.push_back(test);
        }
    };

    for (const auto& test : tests) {
        resolveDependencies(test);
    }

    return sortedTests;
}

// 运行所有测试
ATOM_INLINE void runAllTests(int retryCount, bool parallel, int numThreads) {
    auto& stats = getTestStats();
    Timer globalTimer;

    std::vector<TestCase> allTests;
    for (const auto& suite : getTestSuites()) {
        allTests.insert(allTests.end(), suite.testCases.begin(),
                        suite.testCases.end());
    }

    // 按依赖关系排序测试
    allTests = sortTestsByDependencies(allTests);

    if (parallel) {
        runTestsInParallel(allTests, numThreads);
    } else {
        for (const auto& test : allTests) {
            runTestCase(test, retryCount);
        }
    }

    std::cout << "============================================================="
                 "==================\n";
    std::cout << "Total tests: " << stats.totalTests << "\n";
    std::cout << "Total asserts: " << stats.totalAsserts << " | "
              << stats.passedAsserts << " passed | " << stats.failedAsserts
              << " failed | " << stats.skippedTests << " skipped\n";
    std::cout << "Total time: " << globalTimer.elapsed() << " ms\n";
}

// 测试断言
struct alignas(64) Expect {
    bool result;
    const char* file;
    int line;
    std::string message;

    Expect(bool result, const char* file, int line, std::string msg)
        : result(result), file(file), line(line), message(msg) {
        auto& stats = getTestStats();
        stats.totalAsserts++;
        if (!result) {
            stats.failedAsserts++;
            throw std::runtime_error(std::string(file) + ":" +
                                     std::to_string(line) + ": FAILED - " +
                                     std::string(msg));
        }
        stats.passedAsserts++;
    }
};

// 其他断言类型
ATOM_INLINE auto expectApprox(double lhs, double rhs, double epsilon,
                              const char* file, int line) -> Expect {
    bool result = std::abs(lhs - rhs) <= epsilon;
    return {result, file, line,
            "Expected " + std::to_string(lhs) + " approx equal to " +
                std::to_string(rhs)};
}

template <typename T, typename U>
auto expectEq(const T& lhs, const U& rhs, const char* file,
              int line) -> Expect {
    return Expect(lhs == rhs, file, line,
                  std::string("Expected ") + std::to_string(lhs) +
                      " == " + std::to_string(rhs));
}

template <typename T, typename U>
auto expectNe(const T& lhs, const U& rhs, const char* file,
              int line) -> Expect {
    return Expect(lhs != rhs, file, line,
                  std::string("Expected ") + std::to_string(lhs) +
                      " != " + std::to_string(rhs));
}

template <typename T, typename U>
auto expectGt(const T& lhs, const U& rhs, const char* file,
              int line) -> Expect {
    return Expect(lhs > rhs, file, line,
                  std::string("Expected ") + std::to_string(lhs) + " > " +
                      std::to_string(rhs));
}

// 字符串包含断言
ATOM_INLINE auto expectContains(const std::string& str,
                                const std::string& substr, const char* file,
                                int line) -> Expect {
    bool result = str.find(substr) != std::string::npos;
    return {result, file, line,
            "Expected \"" + str + "\" to contain \"" + substr + "\""};
}

// 集合相等断言
template <typename T>
ATOM_INLINE auto expectSetEq(const std::vector<T>& lhs,
                             const std::vector<T>& rhs, const char* file,
                             int line) -> Expect {
    std::set<T> lhsSet(lhs.begin(), lhs.end());
    std::set<T> rhsSet(rhs.begin(), rhs.end());
    bool result = lhsSet == rhsSet;
    return {result, file, line, "Expected sets to be equal"};
}

// 新增的断言类型
template <typename T, typename U>
auto expectLt(const T& lhs, const U& rhs, const char* file,
              int line) -> Expect {
    return Expect(lhs < rhs, file, line,
                  std::string("Expected ") + std::to_string(lhs) + " < " +
                      std::to_string(rhs));
}

template <typename T, typename U>
auto expectGe(const T& lhs, const U& rhs, const char* file,
              int line) -> Expect {
    return Expect(lhs >= rhs, file, line,
                  std::string("Expected ") + std::to_string(lhs) +
                      " >= " + std::to_string(rhs));
}

template <typename T, typename U>
auto expectLe(const T& lhs, const U& rhs, const char* file,
              int line) -> Expect {
    return Expect(lhs <= rhs, file, line,
                  std::string("Expected ") + std::to_string(lhs) +
                      " <= " + std::to_string(rhs));
}

}  // namespace atom::test

#define expect(expr) atom::test::expect(expr, __FILE__, __LINE__, #expr)
#define expect_eq(lhs, rhs) atom::test::expect_eq(lhs, rhs, __FILE__, __LINE__)
#define expect_ne(lhs, rhs) atom::test::expect_ne(lhs, rhs, __FILE__, __LINE__)
#define expect_gt(lhs, rhs) atom::test::expect_gt(lhs, rhs, __FILE__, __LINE__)
#define expect_lt(lhs, rhs) atom::test::expect_lt(lhs, rhs, __FILE__, __LINE__)
#define expect_ge(lhs, rhs) atom::test::expect_ge(lhs, rhs, __FILE__, __LINE__)
#define expect_le(lhs, rhs) atom::test::expect_le(lhs, rhs, __FILE__, __LINE__)
#define expect_approx(lhs, rhs, eps) \
    atom::test::expect_approx(lhs, rhs, eps, __FILE__, __LINE__)
#define expect_contains(str, substr) \
    atom::test::expect_contains(str, substr, __FILE__, __LINE__)
#define expect_set_eq(lhs, rhs) ut::expect_set_eq(lhs, rhs, __FILE__, __LINE__)

#endif
