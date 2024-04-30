/*
 * argsview.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-19

Description: ArgsView Class for C++

**************************************************/

#include "argsview.hpp"

namespace atom::utils {
ArgsView::ArgsView(int argc, char** argv) : m_argc(argc), m_argv(argv) {
    for (int i = 1; i < m_argc; ++i) {
        std::string_view arg(m_argv[i]);
        if (arg.substr(0, 2) == "--") {
            auto pos = arg.find('=');
            if (pos != std::string_view::npos) {
                auto key = arg.substr(2, pos - 2);
                if (!has(key)) {
                    m_args.emplace(key, arg.substr(pos + 1));
                }
            } else {
                auto flag = arg.substr(2);
                if (!hasFlag(flag)) {
                    m_flags.emplace_back(flag);
                }
            }
        } else {
            for (const auto& rule : m_rules) {
                if (arg.substr(0, rule.first.size()) == rule.first) {
                    rule.second(arg.substr(rule.first.size()));
                    break;
                }
            }
        }
    }
}

std::optional<std::string_view> ArgsView::get(std::string_view key) const {
    if (auto it = m_args.find(key); it != m_args.end()) {
        return it->second;
    }
    throw std::runtime_error(std::string("Key not found: ") + std::string(key));
}

bool ArgsView::has(std::string_view key) const { return m_args.count(key) > 0; }

bool ArgsView::hasFlag(std::string_view flag) const {
    return std::find(m_flags.begin(), m_flags.end(), flag) != m_flags.end();
}

std::vector<std::string_view> ArgsView::getFlags() const { return m_flags; }

std::unordered_map<std::string_view, std::string_view> ArgsView::getArgs()
    const {
    return m_args;
}

void ArgsView::addRule(std::string_view prefix,
                       std::function<void(std::string_view)> handler) {
    m_rules.emplace_back(prefix, handler);
}

}  // namespace atom::utils
