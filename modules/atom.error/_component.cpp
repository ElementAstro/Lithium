/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-System

**************************************************/

#include "_component.hpp"

#include "atom/log/loguru.hpp"

#include "atom/error/error_stack.hpp"

using namespace atom::error;

ErrorComponent::ErrorComponent(const std::string &name) : Component(name) {
    DLOG_F(INFO, "ErrorComponent::ErrorComponent");

    def("insert_error", &ErrorStack::insertError, errorStack_, "error",
        "Insert an error into the error stack.");
    def("set_filters", &ErrorStack::setFilteredModules, errorStack_, "error",
        "Set filters.");
    def("clear_filters", &ErrorStack::clearFilteredModules, errorStack_,
        "error", " Clear filters.");
    def("get_filtered_errors", &ErrorStack::getFilteredErrorsByModule,
        errorStack_, "error", "Get filtered errors by module.");
    def("print_filtered_error_stack", &ErrorStack::printFilteredErrorStack,
        errorStack_, "error", "Print filtered error stack.");
    def("get_compressed_errors", &ErrorStack::getCompressedErrors, errorStack_,
        "error", "Get compressed errors.");

    addVariable("error_stack.instance", errorStack_);
}

ErrorComponent::~ErrorComponent() {
    DLOG_F(INFO, "ErrorComponent::~ErrorComponent");
}

auto ErrorComponent::initialize() -> bool { return true; }

auto ErrorComponent::destroy() -> bool { return true; }
