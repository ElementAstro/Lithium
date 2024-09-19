#pragma once

#include <future>
#include <memory>
#include <string>
#include "atom/type/json.hpp"

class TheSkyX {
public:
    TheSkyX(const std::string& addr, int prt, bool useObj);
    ~TheSkyX();

    TheSkyX(const TheSkyX&) = delete;
    TheSkyX& operator=(const TheSkyX&) = delete;
    TheSkyX(TheSkyX&&) noexcept = default;
    TheSkyX& operator=(TheSkyX&&) noexcept = default;

    [[nodiscard]] auto name() const -> std::string;
    [[nodiscard]] auto canGetRotationAngle() const -> bool;

    auto getTarget() -> std::future<nlohmann::json>;
    auto getSite() -> std::future<nlohmann::json>;
    auto getRotationAngle() -> std::future<double>;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
