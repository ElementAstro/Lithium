#include "rotator.hpp"
#include <stdexcept>

AlpacaRotator::AlpacaRotator(const std::string& address, int device_number,
                             const std::string& protocol)
    : AlpacaDevice(address, "rotator", device_number, protocol) {}

bool AlpacaRotator::GetCanReverse() {
    return GetNumericProperty<bool>("canreverse");
}

bool AlpacaRotator::GetIsMoving() {
    return GetNumericProperty<bool>("ismoving");
}

double AlpacaRotator::GetMechanicalPosition() {
    return GetNumericProperty<double>("mechanicalposition");
}

double AlpacaRotator::GetPosition() {
    return GetNumericProperty<double>("position");
}

bool AlpacaRotator::GetReverse() { return GetNumericProperty<bool>("reverse"); }

void AlpacaRotator::SetReverse(bool ReverseState) {
    Put("reverse", {{"Reverse", ReverseState ? "true" : "false"}});
}

std::optional<double> AlpacaRotator::GetStepSize() {
    try {
        return GetNumericProperty<double>("stepsize");
    } catch (const std::runtime_error& e) {
        return std::nullopt;
    }
}

double AlpacaRotator::GetTargetPosition() {
    return GetNumericProperty<double>("targetposition");
}

void AlpacaRotator::Halt() { Put("halt"); }

std::future<void> AlpacaRotator::Move(double Position) {
    return AsyncMove("move", Position);
}

std::future<void> AlpacaRotator::MoveAbsolute(double Position) {
    return AsyncMove("moveabsolute", Position);
}

std::future<void> AlpacaRotator::MoveMechanical(double Position) {
    return AsyncMove("movemechanical", Position);
}

void AlpacaRotator::Sync(double Position) {
    Put("sync", {{"Position", std::to_string(Position)}});
}