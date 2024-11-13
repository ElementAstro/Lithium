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
#include <vector>

#ifdef USE_SIMD
#ifdef __x86_64__
#include <immintrin.h>
#elif __aarch64__
#include <arm_neon.h>
#endif
#endif

#include "atom/log/loguru.hpp"

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

    [[nodiscard]] auto randomSolution() const -> std::vector<int>;
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
    LOG_F(INFO,
          "SimulatedAnnealing initialized with max_iterations: {}, "
          "initial_temperature: %.2f, cooling_strategy: {}",
          maxIterations, initialTemperature, static_cast<int>(coolingStrategy));
    setCoolingSchedule(coolingStrategy);
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::setCoolingSchedule(
    AnnealingStrategy strategy) {
    cooling_strategy_ = strategy;
    LOG_F(INFO, "Setting cooling schedule to strategy: {}",
          static_cast<int>(strategy));
    switch (cooling_strategy_) {
        case AnnealingStrategy::LINEAR:
            cooling_schedule_ = [this](int iteration) {
                return initial_temperature_ *
                       (1 - static_cast<double>(iteration) / max_iterations_);
            };
            break;
        case AnnealingStrategy::EXPONENTIAL:
            cooling_schedule_ = [this](int iteration) {
                return initial_temperature_ *
                       std::pow(K_COOLING_RATE, iteration);
            };
            break;
        case AnnealingStrategy::LOGARITHMIC:
            cooling_schedule_ = [this](int iteration) {
                if (iteration == 0)
                    return initial_temperature_;
                return initial_temperature_ / std::log(iteration + 2);
            };
            break;
        default:
            LOG_F(WARNING,
                  "Unknown cooling strategy. Defaulting to EXPONENTIAL.");
            cooling_schedule_ = [this](int iteration) {
                return initial_temperature_ *
                       std::pow(K_COOLING_RATE, iteration);
            };
            break;
    }
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::setProgressCallback(
    std::function<void(int, double, const SolutionType&)> callback) {
    progress_callback_ = callback;
    LOG_F(INFO, "Progress callback has been set.");
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::setStopCondition(
    std::function<bool(int, double, const SolutionType&)> condition) {
    stop_condition_ = condition;
    LOG_F(INFO, "Stop condition has been set.");
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
void SimulatedAnnealing<ProblemType, SolutionType>::optimizeThread() {
    try {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_real_distribution<double> distribution(0.0, 1.0);

        auto currentSolution = problem_instance_.random_solution();
        double currentEnergy = problem_instance_.energy(currentSolution);
        LOG_F(INFO, "Thread %ld started with initial energy: {}",
              std::this_thread::get_id(), currentEnergy);

        {
            std::lock_guard lock(best_mutex_);
            if (currentEnergy < best_energy_) {
                best_solution_ = currentSolution;
                best_energy_ = currentEnergy;
                LOG_F(INFO, "New best energy found: {}", best_energy_);
            }
        }

        for (int iteration = 0;
             iteration < max_iterations_ && !should_stop_.load(); ++iteration) {
            double temperature = cooling_schedule_(iteration);
            if (temperature <= 0) {
                LOG_F(WARNING,
                      "Temperature has reached zero or below at iteration {}.",
                      iteration);
                break;
            }

            auto neighborSolution = problem_instance_.neighbor(currentSolution);
            double neighborEnergy = problem_instance_.energy(neighborSolution);

            double energyDifference = neighborEnergy - currentEnergy;
            LOG_F(INFO,
                  "Iteration {}: Current Energy = {}, Neighbor Energy = "
                  "{}, Energy Difference = {}, Temperature = {}",
                  iteration, currentEnergy, neighborEnergy, energyDifference,
                  temperature);

            if (energyDifference < 0 ||
                distribution(generator) <
                    std::exp(-energyDifference / temperature)) {
                currentSolution = std::move(neighborSolution);
                currentEnergy = neighborEnergy;
                LOG_F(INFO, "Solution accepted at iteration {} with energy: {}",
                      iteration, currentEnergy);

                std::lock_guard lock(best_mutex_);
                if (currentEnergy < best_energy_) {
                    best_solution_ = currentSolution;
                    best_energy_ = currentEnergy;
                    LOG_F(INFO, "New best energy updated to: {}", best_energy_);
                }
            }

            if (progress_callback_) {
                try {
                    progress_callback_(iteration, currentEnergy,
                                       currentSolution);
                } catch (const std::exception& e) {
                    LOG_F(ERROR, "Exception in progress_callback_: {}",
                          e.what());
                }
            }

            if (stop_condition_ &&
                stop_condition_(iteration, currentEnergy, currentSolution)) {
                should_stop_.store(true);
                LOG_F(INFO, "Stop condition met at iteration {}.", iteration);
                break;
            }
        }
        LOG_F(INFO, "Thread %ld completed optimization with best energy: {}",
              std::this_thread::get_id(), best_energy_);
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in optimizeThread: {}", e.what());
    }
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
auto SimulatedAnnealing<ProblemType, SolutionType>::optimize(int numThreads)
    -> SolutionType {
    LOG_F(INFO, "Starting optimization with {} threads.", numThreads);
    if (numThreads < 1) {
        LOG_F(WARNING, "Invalid number of threads ({}). Defaulting to 1.",
              numThreads);
        numThreads = 1;
    }

    std::vector<std::future<void>> futures;
    futures.reserve(numThreads);
    for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex) {
        futures.emplace_back(
            std::async(std::launch::async, [this]() { optimizeThread(); }));
        LOG_F(INFO, "Launched optimization thread {}.", threadIndex + 1);
    }

    for (auto& future : futures) {
        try {
            future.wait();
            future.get();
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in optimization thread: {}", e.what());
        }
    }

    LOG_F(INFO, "Optimization completed with best energy: {}", best_energy_);
    return best_solution_;
}

template <typename ProblemType, typename SolutionType>
    requires AnnealingProblem<ProblemType, SolutionType>
auto SimulatedAnnealing<ProblemType, SolutionType>::getBestEnergy() const
    -> double {
    std::lock_guard lock(best_mutex_);
    return best_energy_;
}

// TSP class implementation
inline TSP::TSP(const std::vector<std::pair<double, double>>& cities)
    : cities_(cities) {
    LOG_F(INFO, "TSP instance created with %zu cities.", cities_.size());
}

inline auto TSP::energy(const std::vector<int>& solution) const -> double {
    double totalDistance = 0.0;
    size_t numCities = solution.size();

#ifdef USE_SIMD
    __m256d totalDistanceVec = _mm256_setzero_pd();
    size_t i = 0;
    for (; i + 3 < numCities; i += 4) {
        __m256d x1 = _mm256_set_pd(
            cities_[solution[i]].first, cities_[solution[i + 1]].first,
            cities_[solution[i + 2]].first, cities_[solution[i + 3]].first);
        __m256d y1 = _mm256_set_pd(
            cities_[solution[i]].second, cities_[solution[i + 1]].second,
            cities_[solution[i + 2]].second, cities_[solution[i + 3]].second);

        __m256d x2 =
            _mm256_set_pd(cities_[solution[(i + 1) % numCities]].first,
                          cities_[solution[(i + 2) % numCities]].first,
                          cities_[solution[(i + 3) % numCities]].first,
                          cities_[solution[(i + 4) % numCities]].first);
        __m256d y2 =
            _mm256_set_pd(cities_[solution[(i + 1) % numCities]].second,
                          cities_[solution[(i + 2) % numCities]].second,
                          cities_[solution[(i + 3) % numCities]].second,
                          cities_[solution[(i + 4) % numCities]].second);

        __m256d deltaX = _mm256_sub_pd(x1, x2);
        __m256d deltaY = _mm256_sub_pd(y1, y2);

        __m256d distance = _mm256_sqrt_pd(_mm256_add_pd(
            _mm256_mul_pd(deltaX, deltaX), _mm256_mul_pd(deltaY, deltaY)));
        totalDistanceVec = _mm256_add_pd(totalDistanceVec, distance);
    }

    // Horizontal addition to sum up the total distance in vector
    double distances[4];
    _mm256_storeu_pd(distances, totalDistanceVec);
    for (double d : distances) {
        totalDistance += d;
    }
#endif

    // Handle leftover cities that couldn't be processed in sets of 4
    for (size_t index = numCities - numCities % 4; index < numCities; ++index) {
        auto [x1, y1] = cities_[solution[index]];
        auto [x2, y2] = cities_[solution[(index + 1) % numCities]];
        double deltaX = x1 - x2;
        double deltaY = y1 - y2;
        totalDistance += std::sqrt(deltaX * deltaX + deltaY * deltaY);
    }

    LOG_F(INFO, "Computed energy (total distance): {}", totalDistance);
    return totalDistance;
}

inline auto TSP::neighbor(const std::vector<int>& solution)
    -> std::vector<int> {
    std::vector<int> newSolution = solution;
    try {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<int> distribution(
            0, static_cast<int>(solution.size()) - 1);
        int index1 = distribution(generator);
        int index2 = distribution(generator);
        std::swap(newSolution[index1], newSolution[index2]);
        LOG_F(INFO,
              "Generated neighbor solution by swapping indices {} and {}.",
              index1, index2);
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in TSP::neighbor: {}", e.what());
        throw;
    }
    return newSolution;
}

inline auto TSP::randomSolution() const -> std::vector<int> {
    std::vector<int> solution(cities_.size());
    std::iota(solution.begin(), solution.end(), 0);
    try {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::ranges::shuffle(solution, generator);
        LOG_F(INFO, "Generated random solution.");
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in TSP::randomSolution: {}", e.what());
        throw;
    }
    return solution;
}

#endif  // ATOM_ALGORITHM_ANNEALING_HPP