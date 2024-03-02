/*
 * loop_task.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Loop Task Definition

**************************************************/

#ifndef ATOM_TASK_LOOP_TASK_HPP
#define ATOM_TASK_LOOP_TASK_HPP

#include "task.hpp"

class LoopTask : public SimpleTask {
public:
    /**
     * @brief Constructor for the LoopTask class.
     *
     * @param item_fn The function that will be executed for each item.
     * @param stop_fn An optional stop function. Default is nullptr.
     * @param params_template The template for the task parameters.
     * @param loop_count The number of times the task will be executed.
     */
    LoopTask(const std::function<json(const json &)> &func,
             const std::function<json(const json &)> &stop_fn,
             const json &params_template, int loop_count);

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
    int m_loopCount;
};

#endif
