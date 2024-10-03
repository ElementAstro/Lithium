#ifndef ATOM_ALGORITHM_ANNEALING_HPP
#define ATOM_ALGORITHM_ANNEALING_HPP

#include <algorithm>
#include <atomic>
#include <cmath>
#include <functional>
#include <future>
#include <limits>
#include <mutex>
#include <numeric>
#include <random>
#include <ranges>
#include <vector>


// Define a concept for a problem that Simulated Annealing can solve
template <typename ProblemType, typename SolutionType>
concept AnnealingProblem =
    requires(ProblemType problemInstance, SolutionType solutionInstance) {
        {
            problemInstance.energy(solutionInstance)
        } -> std::convertible_to<double>;
        {
            problemInstance.neighbor(solutionInstance)
        } -> std::same_as<SolutionType>;
        { problemInstance.random_solution() } -> std::same_as<SolutionType>;
    };

// Different cooling strategies for temperature reduction
enum class AnnealingStrategy { LINEAR, EXPONENTIAL, LOGARITHMIC };

// Simulated Annealing algorithm implementation
template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
class SimulatedAnnealing {
private:
    ProblemType& problem_instance_;
    std::function<double(int)> cooling_schedule_;
    int max_iterations_;
    double initial_temperature_;
    AnnealingStrategy cooling_strategy_;
    std::function<void(int, double, const SolutionType&)> progress_callback_;
    std::function<bool(int, double, const SolutionType&)> stop_condition_;
    std::atomic<bool> should_stop_{false};

    std::mutex best_mutex_;
    SolutionType best_solution_;
    double best_energy_ = std::numeric_limits<double>::max();

    static constexpr int K_DEFAULT_MAX_ITERATIONS = 1000;
    static constexpr double K_DEFAULT_INITIAL_TEMPERATURE = 100.0;
    static constexpr double K_COOLING_RATE = 0.95;

    void optimizeThread();

public:
    explicit SimulatedAnnealing(
        ProblemType& problemInstance,
        AnnealingStrategy coolingStrategy = AnnealingStrategy::EXPONENTIAL,
        int maxIterations = K_DEFAULT_MAX_ITERATIONS,
        double initialTemperature = K_DEFAULT_INITIAL_TEMPERATURE);

    void setCoolingSchedule(AnnealingStrategy strategy);

    void setProgressCallback(
        std::function<void(int, double, const SolutionType&)> callback);

    void setStopCondition(
        std::function<bool(int, double, const SolutionType&)> condition);

    auto optimize(int numThreads = 1) -> SolutionType;

    [[nodiscard]] auto getBestEnergy() const -> double;
};

// Example TSP (Traveling Salesman Problem) implementation
class TSP {
private:
    std::vector<std::pair<double, double>> cities_;

public:
    explicit TSP(const std::vector<std::pair<double, double>>& cities);

    [[nodiscard]] auto energy(const std::vector<int>& solution) const -> double;

    [[nodiscard]] static auto neighbor(const std::vector<int>& solution)
        -> std::vector<int>;

    [[nodiscard]] auto random_solution() const -> std::vector<int>;
};

// SimulatedAnnealing class implementation
template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
SimulatedAnnealing<ProblemType, SolutionType>::SimulatedAnnealing(
    ProblemType& problemInstance, AnnealingStrategy coolingStrategy,
    int maxIterations, double initialTemperature)
    : problem_instance_(problemInstance),
      max_iterations_(maxIterations),
      initial_temperature_(initialTemperature),
      cooling_strategy_(coolingStrategy) {
    setCoolingSchedule(coolingStrategy);
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::setCoolingSchedule(
    AnnealingStrategy strategy) {
    cooling_strategy_ = strategy;
    switch (cooling_strategy_) {
        case AnnealingStrategy::LINEAR:
            cooling_schedule_ = [this](int iteration) {
                return initial_temperature_ *
                       (1 - static_cast<double>(iteration) / max_iterations_);
            };
            break;
        case AnnealingStrategy::EXPONENTIAL:
            cooling_schedule_ = [this](int iteration) {
                return initial_temperature_ * std::pow(K_COOLING_RATE, iteration);
            };
            break;
        case AnnealingStrategy::LOGARITHMIC:
            cooling_schedule_ = [this](int iteration) {
                return initial_temperature_ / std::log(iteration + 2);
            };
            break;
    }
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::setProgressCallback(
    std::function<void(int, double, const SolutionType&)> callback) {
    progress_callback_ = callback;
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::setStopCondition(
    std::function<bool(int, double, const SolutionType&)> condition) {
    stop_condition_ = condition;
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::optimizeThread() {
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_real_distribution distribution(0.0, 1.0);

    auto currentSolution = problem_instance_.random_solution();
    double currentEnergy = problem_instance_.energy(currentSolution);

    for (int iteration = 0; iteration < max_iterations_ && !should_stop_.load();
         ++iteration) {
        double temperature = cooling_schedule_(iteration);

        auto neighborSolution = problem_instance_.neighbor(currentSolution);
        double neighborEnergy = problem_instance_.energy(neighborSolution);

        if (double energyDifference = neighborEnergy - currentEnergy;
            energyDifference < 0 ||
            distribution(generator) <
                std::exp(-energyDifference / temperature)) {
            currentSolution = std::move(neighborSolution);
            currentEnergy = neighborEnergy;

            std::lock_guard lock(best_mutex_);
            if (currentEnergy < best_energy_) {
                best_solution_ = currentSolution;
                best_energy_ = currentEnergy;
            }
        }

        if (progress_callback_) {
            progress_callback_(iteration, currentEnergy, currentSolution);
        }

        if (stop_condition_ &&
            stop_condition_(iteration, currentEnergy, currentSolution)) {
            should_stop_.store(true);
            break;
        }
    }
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
auto SimulatedAnnealing<ProblemType, SolutionType>::optimize(int numThreads)
    -> SolutionType {
    std::vector<std::future<void>> futures;

    futures.reserve(numThreads);
    for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex) {
        futures.push_back(
            std::async(std::launch::async, [this]() { optimizeThread(); }));
    }

    for (auto& future : futures) {
        future.wait();
    }

    return best_solution_;
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
auto SimulatedAnnealing<ProblemType, SolutionType>::getBestEnergy() const
    -> double {
    return best_energy_;
}

// TSP class implementation
inline TSP::TSP(const std::vector<std::pair<double, double>>& cities)
    : cities_(cities) {}

inline auto TSP::energy(const std::vector<int>& solution) const -> double {
    double totalDistance = 0.0;
    for (size_t index = 0; index < solution.size(); ++index) {
        auto [x1, y1] = cities_[solution[index]];
        auto [x2, y2] = cities_[solution[(index + 1) % solution.size()]];
        double deltaX = x1 - x2;
        double deltaY = y1 - y2;
        totalDistance += std::sqrt(deltaX * deltaX + deltaY * deltaY);
    }
    return totalDistance;
}

inline auto TSP::neighbor(const std::vector<int>& solution) -> std::vector<int> {
    std::vector<int> newSolution = solution;
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution distribution(
        0, static_cast<int>(solution.size()) - 1);
    int index1 = distribution(generator);
    int index2 = distribution(generator);
    std::swap(newSolution[index1], newSolution[index2]);
    return newSolution;
}

inline auto TSP::random_solution() const -> std::vector<int> {
    std::vector<int> solution(cities_.size());
    std::iota(solution.begin(), solution.end(), 0);
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::ranges::shuffle(solution, generator);
    return solution;
}

#endif  // ATOM_ALGORITHM_ANNEALING_HPP