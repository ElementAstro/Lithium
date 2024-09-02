#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>

class CartesDuCiel {
public:
    CartesDuCiel(const std::string& addr, int prt);
    ~CartesDuCiel();

    std::optional<std::pair<std::string, std::pair<double, double>>>
    getTarget();
    std::optional<std::pair<double, double>> getSite();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
