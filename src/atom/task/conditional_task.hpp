/*
 * conditional_task.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Conditional Task Definition

**************************************************/

#ifndef ATOM_TASK_CONDITIONAL_TASK_HPP
#define ATOM_TASK_CONDITIONAL_TASK_HPP

#include "task.hpp"

class ConditionalTask : public SimpleTask {
public:
    /**
     * @brief Constructor for the ConditionalTask class.
     *
     * @param condition_fn The condition function used to determine whether to
     * execute the task.
     * @param stop_fn An optional stop function. Default is nullptr.
     * @param params_template The template for the task parameters.
     * @param task_fn The task function used to execute the task logic.
     * @param isForce A flag indicating whether to force execution of the task
     * even if the condition is not met. Default is false.
     */
    ConditionalTask(const std::function<json(const json &)> &task_fn,
                    const std::function<bool(const json &)> &condition_fn,
                    const std::function<json(const json &)> &stop_fn = nullptr,
                    const json &params_template = json(), bool isForce = false);

    /**
     * @brief Executes the task.
     *
     * This function is implemented by the subclass to define the specific logic
     * of the task.
     *
     * @return The result of the task execution in JSON format.
     */
    virtual json execute() override;

    /**
     * @brief Serializes the task to a JSON object.
     *
     * @return A JSON object representing the task.
     */
    virtual json toJson() override;

private:
    // The condition function used to determine whether to execute the task.
    std::function<bool(const json &)> m_conditionFunc;

    bool m_isForce;
};

#endif
