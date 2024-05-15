/*
 * task.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Basic and Simple Task Definition

**************************************************/

#ifndef ATOM_TASK_TASK_HPP
#define ATOM_TASK_TASK_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "atom/type/json.hpp"
using json = nlohmann::json;

/**
 * @brief The SimpleTask class represents a task that can be merged with other
 * tasks.
 *
 * A SimpleTask object can only be executed after being merged with other tasks.
 * It is used to group multiple tasks into a single unit of work, which can then
 * be executed as a single task. A SimpleTask object can have a stop function
 * that is called when the user requests to stop the task. The stop function
 * should stop the task immediately and return a JSON object with the current
 * status of the task.
 */
class SimpleTask {
public:
    /**
     * @brief Constructs a SimpleTask object with a stop function and a flag
     * indicating whether the task can be stopped.
     *
     * @param func A function that is called when the task is executed.
     * @param stop_fn A function that is called when the user requests to stop
     * the task.
     * @param params_template A JSON object that is used to generate the
     * parameters of the task.
     * @note The stop function should stop the task immediately and return a
     * JSON object with the current status of the task.
     */
    SimpleTask(const std::function<json(const json &)> &func,
               const std::function<json(const json &)> &stop_fn,
               const json &params_template);

    /**
     * @brief Destructs the SimpleTask object.
     *
     * If the task has been stopped, the stop function will be called before the
     * object is destructed.
     */
    ~SimpleTask();

    /**
     * @brief Returns a JSON object representing the SimpleTask object.
     *
     * The returned JSON object contains the following fields:
     * - "type": "merged"
     * - "name": The name of the task.
     * - "id": The ID of the task.
     * - "description": The description of the task.
     * - "can_stop": A flag indicating whether the task can be stopped.
     */
    virtual json toJson();

    /**
     * @brief Returns a JSON object representing the result of the SimpleTask
     * object.
     *
     * For a SimpleTask object, the result is always an empty JSON object.
     */
    json getResult();

    /**
     * @brief Returns a JSON object representing the template of the parameters
     * required by the SimpleTask object.
     *
     * A SimpleTask object does not require any parameters, so the returned JSON
     * object is always an empty JSON object.
     */
    json getParamsTemplate();

    /**
     * @brief Sets the parameters of the SimpleTask object.
     *
     * A SimpleTask object does not require any parameters, so this function
     * does nothing.
     *
     * @param params The parameters to set.
     */
    void setParams(const json &params);

    /**
     * @brief Returns the ID of the SimpleTask object.
     *
     * @return The ID of the SimpleTask object.
     */
    int getId() const;

    /**
     * @brief Sets the ID of the SimpleTask object.
     *
     * @param id The ID to set.
     */
    void setId(int id);

    /**
     * @brief Returns the name of the SimpleTask object.
     *
     * @return The name of the SimpleTask object.
     */
    const std::string &getName() const;

    /**
     * @brief Sets the name of the SimpleTask object.
     *
     * @param name The name to set.
     */
    void setName(const std::string &name);

    /**
     * @brief Returns the description of the SimpleTask object.
     *
     * @return The description of the SimpleTask object.
     */
    const std::string &getDescription() const;

    /**
     * @brief Sets the description of the SimpleTask object.
     *
     * @param description The description to set.
     */
    void setDescription(const std::string &description);

    /**
     * @brief Sets whether the SimpleTask object can be executed.
     *
     * @param can_execute Whether the SimpleTask object can be executed.
     */
    void setCanExecute(bool can_execute);

    /**
     * @brief Returns whether the SimpleTask object can be executed.
     *
     * @return Whether the SimpleTask object can be executed.
     */
    bool isExecutable() const;

    /**
     * @brief Sets the stop function of the SimpleTask object.
     *
     * The stop function is called when the user requests to stop the task. It
     * should stop the task immediately and return a JSON object with the
     * current status of the task.
     *
     * @param stop_fn The stop function to set.
     */
    void setStopFunction(const std::function<json(const json &)> &stop_fn);

    /**
     * @brief Returns whether the stop flag of the SimpleTask object is set.
     *
     * @return Whether the stop flag of the SimpleTask object is set.
     */
    bool getStopFlag() const;

    /**
     * @brief Sets the stop flag of the SimpleTask object.
     *
     * The stop flag is set when the user requests to stop the task. It is used
     * to indicate that the task should be stopped as soon as possible.
     *
     * @param flag The value to set the stop flag to.
     */
    void setStopFlag(bool flag);

    /**
     * @brief Stops the SimpleTask object.
     *
     * If the stop function has been set, it will be called before the task is
     * stopped.
     */
    void stop();

    /**
     * @brief Validates whether a JSON value matches a template.
     *
     * This function is used to validate the parameters passed to the SimpleTask
     * object. It compares a JSON value with a template and returns true if they
     * match, false otherwise.
     *
     * @param data The JSON value to validate.
     * @param templateValue The template to compare the JSON value with.
     * @return Whether the JSON value matches the template.
     */
    bool validateJsonValue(const json &data, const json &templateValue);

    /**
     * @brief Validates whether a JSON string matches a template.
     *
     * This function is used to validate the parameters passed to the SimpleTask
     * object. It compares a JSON string with a template and returns true if
     * they match, false otherwise.
     *
     * @param jsonString The JSON string to validate.
     * @param templateString The template to compare the JSON string with.
     * @return Whether the JSON string matches the template.
     */
    bool validateJsonString(const std::string &jsonString,
                            const std::string &templateString);

    /**
     * @brief Executes the SimpleTask object.
     *
     * This function executes the SimpleTask object and returns a JSON object
     * with the current status of the task. For a SimpleTask object, the
     * returned JSON object is always the JSON object returned by the toJson()
     * function.
     *
     * @return A JSON object with the current status of the task.
     */
    virtual json execute();

    int getPriority() const { return priority; }
    bool getStatus() const { return status; }

    void setPriority(int pri) { priority = pri; }
    void setStatus(bool sta) { status = sta; }

    std::function<json(const json &)> m_function;
    json m_paramsTemplate;
    json m_params;
    json m_returns;
    std::function<json(const json &)> m_stopFn;
    bool m_canStop;
    std::atomic_bool m_stopFlag;
    std::atomic_bool m_isExecuting;
    int m_id;
    std::string m_name;
    std::string m_description;
    bool m_canExecute;
    int priority;
    bool status;
};

#endif
