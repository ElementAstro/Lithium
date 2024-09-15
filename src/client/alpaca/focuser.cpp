#include "focuser.hpp"

#include <chrono>
#include <format>
#include <stdexcept>
#include <thread>

AlpacaFocuser::AlpacaFocuser(const std::string& address, int device_number,
                             const std::string& protocol)
    : AlpacaDevice(address, "focuser", device_number, protocol) {}

AlpacaFocuser::~AlpacaFocuser() {
    if (m_move_thread.joinable()) {
        m_move_thread.join();
    }
}

bool AlpacaFocuser::GetAbsolute() {
    return GetNumericProperty<bool>("absolute");
}

bool AlpacaFocuser::GetIsMoving() { return m_is_moving.load(); }

int AlpacaFocuser::GetMaxIncrement() {
    return GetNumericProperty<int>("maxincrement");
}

int AlpacaFocuser::GetMaxStep() { return GetNumericProperty<int>("maxstep"); }

int AlpacaFocuser::GetPosition() { return GetNumericProperty<int>("position"); }

float AlpacaFocuser::GetStepSize() {
    return GetNumericProperty<float>("stepsize");
}

bool AlpacaFocuser::GetTempComp() {
    return GetNumericProperty<bool>("tempcomp");
}

void AlpacaFocuser::SetTempComp(bool TempCompState) {
    Put("tempcomp", {{"TempComp", TempCompState ? "true" : "false"}});
}

bool AlpacaFocuser::GetTempCompAvailable() {
    return GetNumericProperty<bool>("tempcompavailable");
}

std::optional<float> AlpacaFocuser::GetTemperature() {
    try {
        return GetNumericProperty<float>("temperature");
    } catch (const std::runtime_error& e) {
        // If temperature is not implemented, return nullopt
        return std::nullopt;
    }
}

void AlpacaFocuser::Halt() {
    Put("halt");
    m_is_moving.store(false);
}

void AlpacaFocuser::StartMove(int Position) {
    Put("move", {{"Position", std::to_string(Position)}});
}

void AlpacaFocuser::MoveThread(int Position) {
    m_is_moving.store(true);
    StartMove(Position);

    // Poll the IsMoving property until the move is complete
    while (GetNumericProperty<bool>("ismoving")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    m_is_moving.store(false);
}

std::future<void> AlpacaFocuser::Move(int Position) {
    // If a move is already in progress, wait for it to complete
    if (m_move_thread.joinable()) {
        m_move_thread.join();
    }

    // Start a new move thread
    m_move_thread = std::thread(&AlpacaFocuser::MoveThread, this, Position);

    // Return a future that will be ready when the move is complete
    return std::async(std::launch::deferred, [this]() {
        if (m_move_thread.joinable()) {
            m_move_thread.join();
        }
    });
}
