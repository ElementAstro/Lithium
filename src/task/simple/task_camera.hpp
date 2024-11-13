#ifndef LITHIUM_TASK_SIMPLE_TASK_CAMERA_HPP
#define LITHIUM_TASK_SIMPLE_TASK_CAMERA_HPP

#include "task.hpp"

#include <format>

#include "atom/type/json.hpp"

namespace lithium::sequencer::task {

enum ExposureType { LIGHT, DARK, BIAS, FLAT, SNAPSHOT };

NLOHMANN_JSON_SERIALIZE_ENUM(ExposureType, {
                                               {LIGHT, "light"},
                                               {DARK, "dark"},
                                               {BIAS, "bias"},
                                               {FLAT, "flat"},
                                               {SNAPSHOT, "snapshot"},
                                           })

/**
 * @brief Derived class for creating TakeExposure tasks.
 */
class TakeExposureTask : public TaskCreator<TakeExposureTask> {
public:
    static auto taskName() -> std::string;

    static void execute(const json& params);
};

/**
 * @brief Derived class for creating TakeManyExposure tasks.
 */
class TakeManyExposureTask : public TaskCreator<TakeManyExposureTask> {
public:
    static auto taskName() -> std::string;

    static void execute(const json& params);
};

/**
 * @brief Derived class for creating SubframeExposure tasks.
 */
class SubframeExposureTask : public TaskCreator<SubframeExposureTask> {
public:
    static auto taskName() -> std::string;

    static void execute(const json& params);
};

/**
 * @brief Derived class for creating SmartExposure tasks.
 */
class SmartExposureTask : public TaskCreator<SmartExposureTask> {
public:
    static auto taskName() -> std::string;

    static void execute(const json& params);
};

}  // namespace lithium::sequencer::task

#endif  // LITHIUM_TASK_SIMPLE_TASK_CAMERA_HPP