#include "task.hpp"

// StateMachine method implementations

void StateMachine::addState(StatePtr state) {
    states[state->getName()] = state;
}

void StateMachine::setInitialState(const std::string& stateName) {
    auto it = states.find(stateName);
    if (it != states.end()) {
        currentState = it->second;
        currentState->onEnter();
    }
}

void StateMachine::transitionTo(const std::string& stateName) {
    auto it = states.find(stateName);
    if (it != states.end() && currentState != it->second) {
        currentState->onExit();
        currentState = it->second;
        currentState->onEnter();
    }
}

void StateMachine::handleEvent(std::shared_ptr<Event> event) {
    if (currentState) {
        currentState->handleEvent(event);
    }
}

StateMachine::StatePtr StateMachine::getCurrentState() const {
    return currentState;
}

// TaskEvent method implementations

TaskEvent::TaskEvent(Type type) : eventType_(type) {}

auto TaskEvent::getType() const -> TaskEvent::Type { return eventType_; }

// TaskState method implementations

TaskState::TaskState(Task& task) : task(task) {}

// PendingState method implementations

auto PendingState::getName() const -> std::string { return "Pending"; }

void PendingState::handleEvent(std::shared_ptr<Event> event) {
    auto taskEvent = std::dynamic_pointer_cast<TaskEvent>(event);
    if (taskEvent && taskEvent->getType() == TaskEvent::Start) {
        task.executeCustomFunctions(Task::Status::Pending);
        task.getStateMachine().transitionTo("Running");
        task.run();
    }
}

void PendingState::onEnter() {}

void PendingState::onExit() {}

// RunningState method implementations

auto RunningState::getName() const -> std::string { return "Running"; }

void RunningState::handleEvent(std::shared_ptr<Event> event) {
    auto taskEvent = std::dynamic_pointer_cast<TaskEvent>(event);
    if (taskEvent && taskEvent->getType() == TaskEvent::Complete) {
        task.executeCustomFunctions(Task::Status::Running);
        task.getStateMachine().transitionTo("Completed");
    } else if (taskEvent && taskEvent->getType() == TaskEvent::Fail) {
        task.executeCustomFunctions(Task::Status::Running);
        task.getStateMachine().transitionTo("Failed");
    } else if (task.isTimeout()) {
        task.fail(std::runtime_error("Task timed out"));
    }
}

void RunningState::onEnter() {}

void RunningState::onExit() {}

// CompletedState method implementations

auto CompletedState::getName() const -> std::string { return "Completed"; }

void CompletedState::onEnter() {}

void CompletedState::onExit() {}

// FailedState method implementations

auto FailedState::getName() const -> std::string { return "Failed"; }

void FailedState::onEnter() {}

void FailedState::onExit() {}

Task::Task(
    const std::string& name, const json& params,
    std::function<json(const json&)> function,
    std::optional<std::function<void(const std::exception&)>> onTerminate)
    : name(name),
      params(params),
      function(std::move(function)),
      onTerminate(std::move(onTerminate)) {
    statusMachine.addState(std::make_shared<PendingState>(*this));
    statusMachine.addState(std::make_shared<RunningState>(*this));
    statusMachine.addState(std::make_shared<CompletedState>(*this));
    statusMachine.addState(std::make_shared<FailedState>(*this));
    statusMachine.setInitialState("Pending");
}

void Task::start() {
    statusMachine.handleEvent(std::make_shared<TaskEvent>(TaskEvent::Start));
}

void Task::run() {
    try {
        result = function(params);
        complete();
    } catch (const std::exception& e) {
        fail(e);
    }
}

void Task::complete() {
    setStatus(Status::Completed);
    statusMachine.handleEvent(std::make_shared<TaskEvent>(TaskEvent::Complete));
}

void Task::fail(const std::exception& e) {
    setStatus(Status::Failed);
    if (onTerminate) {
        (*onTerminate)(e);
    }
    statusMachine.handleEvent(std::make_shared<TaskEvent>(TaskEvent::Fail));
}

auto Task::getName() const -> const std::string& { return name; }

auto Task::getParams() const -> const json& { return params; }

auto Task::getResult() const -> const std::optional<json>& { return result; }

auto Task::getStatus() const -> Task::Status { return status; }

void Task::setStatus(Status status) { this->status = status; }

auto Task::getStateMachine() -> StateMachine& { return statusMachine; }

void Task::registerCustomFunction(Status status, CustomFunction function) {
    customFunctions[status].push_back(std::move(function));
}

void Task::executeCustomFunctions(Status status) {
    auto it = customFunctions.find(status);
    if (it != customFunctions.end()) {
        for (const auto& function : it->second) {
            function(*this);
        }
    }
}

void Task::cancel() {
    if (status == Status::Running) {
        status = Status::Failed;
        result = std::nullopt;
        statusMachine.handleEvent(std::make_shared<TaskEvent>(TaskEvent::Fail));
        THROW_TASK_CANCELED("");
    }
}

void Task::setProgress(double progress) {
    this->progress = progress;
    executeCustomFunctions(Status::Running);
}

auto Task::getProgress() const -> double { return progress; }

void Task::setTimeout(std::chrono::milliseconds timeout) {
    this->timeout = timeout;
}

auto Task::isTimeout() const -> bool {
    if (timeout.has_value()) {
        auto now = std::chrono::steady_clock::now();
        return now - startTime >= timeout.value();
    }
    return false;
}