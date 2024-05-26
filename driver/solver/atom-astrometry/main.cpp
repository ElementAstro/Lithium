#include "astrometry.hpp"

static std::shared_ptr<AstrometrySolver> createShared(const std::string &name) {
    return std::make_shared<AstrometrySolver>(name);
}

int main(int argc, char *argv[]) {
    std::shared_ptr<AstrometrySolver> solver = createShared("Astrometry");
    solver->solveImage("test.fits", 1000, false);
    return 0;
}
