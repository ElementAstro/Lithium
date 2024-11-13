// FILE: src/task/simple/test_sequencer.hpp

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "task/simple/sequencer.hpp"
#include "task/simple/target.hpp"

using namespace lithium::sequencer;
using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

class MockTarget : public Target {
public:
    MOCK_METHOD(void, execute, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, pause, (), (override));
    MOCK_METHOD(void, resume, (), (override));
    MOCK_METHOD(TargetStatus, getStatus, (), (const, override));
};

class ExposureSequenceTest : public ::testing::Test {
protected:
    ExposureSequence sequence;
};

TEST_F(ExposureSequenceTest, ConstructorDestructor) {
    ExposureSequence* seq = new ExposureSequence();
    delete seq;
}

TEST_F(ExposureSequenceTest, AddTarget) {
    auto target = std::make_unique<MockTarget>();
    sequence.addTarget(std::move(target));
    EXPECT_EQ(sequence.getTargetNames().size(), 1);
}

TEST_F(ExposureSequenceTest, RemoveTarget) {
    auto target = std::make_unique<MockTarget>();
    std::string targetName = "target1";
    sequence.addTarget(std::move(target));
    sequence.removeTarget(targetName);
    EXPECT_EQ(sequence.getTargetNames().size(), 0);
}

TEST_F(ExposureSequenceTest, ModifyTarget) {
    auto target = std::make_unique<MockTarget>();
    std::string targetName = "target1";
    sequence.addTarget(std::move(target));
    TargetModifier modifier = [](Target& target) { /* modify target */ };
    sequence.modifyTarget(targetName, modifier);
    // Add assertions to verify modification if possible
}

TEST_F(ExposureSequenceTest, ExecuteAll) {
    auto target = std::make_unique<MockTarget>();
    MockTarget* targetPtr = target.get();
    EXPECT_CALL(*targetPtr, execute()).Times(1);
    sequence.addTarget(std::move(target));
    sequence.executeAll();
}

TEST_F(ExposureSequenceTest, Stop) {
    auto target = std::make_unique<MockTarget>();
    MockTarget* targetPtr = target.get();
    EXPECT_CALL(*targetPtr, stop()).Times(1);
    sequence.addTarget(std::move(target));
    sequence.stop();
}

TEST_F(ExposureSequenceTest, Pause) {
    auto target = std::make_unique<MockTarget>();
    MockTarget* targetPtr = target.get();
    EXPECT_CALL(*targetPtr, pause()).Times(1);
    sequence.addTarget(std::move(target));
    sequence.pause();
}

TEST_F(ExposureSequenceTest, Resume) {
    auto target = std::make_unique<MockTarget>();
    MockTarget* targetPtr = target.get();
    EXPECT_CALL(*targetPtr, resume()).Times(1);
    sequence.addTarget(std::move(target));
    sequence.resume();
}

TEST_F(ExposureSequenceTest, SaveLoadSequence) {
    std::string filename = "test_sequence.json";
    sequence.saveSequence(filename);
    sequence.loadSequence(filename);
    // Add assertions to verify save/load functionality
}

TEST_F(ExposureSequenceTest, GetTargetNames) {
    auto target = std::make_unique<MockTarget>();
    sequence.addTarget(std::move(target));
    auto names = sequence.getTargetNames();
    EXPECT_EQ(names.size(), 1);
}

TEST_F(ExposureSequenceTest, GetTargetStatus) {
    auto target = std::make_unique<MockTarget>();
    MockTarget* targetPtr = target.get();
    EXPECT_CALL(*targetPtr, getStatus()).WillOnce(Return(TargetStatus::Pending));
    sequence.addTarget(std::move(target));
    auto status = sequence.getTargetStatus("target1");
    EXPECT_EQ(status, TargetStatus::Pending);
}

TEST_F(ExposureSequenceTest, GetProgress) {
    auto target = std::make_unique<MockTarget>();
    sequence.addTarget(std::move(target));
    double progress = sequence.getProgress();
    EXPECT_EQ(progress, 0.0);
}

TEST_F(ExposureSequenceTest, SetCallbacks) {
    sequence.setOnSequenceStart([]() {});
    sequence.setOnSequenceEnd([]() {});
    sequence.setOnTargetStart([](const std::string&, TargetStatus) {});
    sequence.setOnTargetEnd([](const std::string&, TargetStatus) {});
    sequence.setOnError([](const std::string&, const std::exception&) {});
    // Add assertions to verify callback setting if possible
}
