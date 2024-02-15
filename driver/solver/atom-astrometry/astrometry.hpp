#include "atom/driver/solver.hpp"

class AstrometrySolver : public Solver
{
public:
    explicit AstrometrySolver(const std::string &name);
    virtual ~AstrometrySolver();

    virtual bool connect(const json &params) override;

    virtual bool disconnect(const json &params) override;

    virtual bool reconnect(const json &params) override;

    virtual bool isConnected() override;

    virtual bool solveImage(const std::string &image, const int &timeout, const bool &debug);

    virtual bool getSolveResult(const int &timeout, const bool &debug);

    virtual bool getSolveStatus(const int &timeout, const bool &debug);

    virtual bool setSolveParams(const json &params);

    virtual json getSolveParams();

private:

    std::string makeCommand();

    SolveResult parseOutput(const std::string &output);
};