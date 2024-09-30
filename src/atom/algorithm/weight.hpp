#ifndef ATOM_ALGORITHM_WEIGHT_HPP
#define ATOM_ALGORITHM_WEIGHT_HPP

#include <algorithm>
#include <concepts>
#include <format>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <span>
#include <vector>

#include "atom/utils/random.hpp"
#include "error/exception.hpp"
#include "function/concept.hpp"

namespace atom::algorithm {
template <Arithmetic T>
class WeightSelector {
public:
    class SelectionStrategy {
    public:
        virtual ~SelectionStrategy() = default;
        virtual auto select(std::span<const T> cumulative_weights,
                            T total_weight) -> size_t = 0;
    };

    class DefaultSelectionStrategy : public SelectionStrategy {
    private:
        utils::Random<std::mt19937, std::uniform_real_distribution<>> random_;

    public:
        DefaultSelectionStrategy() : random_(0.0, 1.0) {}

        auto select(std::span<const T> cumulative_weights,
                    T total_weight) -> size_t override {
            T randomValue = random_() * total_weight;
            auto it = std::ranges::upper_bound(cumulative_weights, randomValue);
            return std::distance(cumulative_weights.begin(), it);
        }
    };

    class BottomHeavySelectionStrategy : public SelectionStrategy {
    private:
        utils::Random<std::mt19937, std::uniform_real_distribution<>> random_;

    public:
        BottomHeavySelectionStrategy() : random_(0.0, 1.0) {}

        auto select(std::span<const T> cumulative_weights,
                      T total_weight) -> size_t override {
            T randomValue = std::sqrt(random_()) * total_weight;
            auto it = std::ranges::upper_bound(cumulative_weights, randomValue);
            return std::distance(cumulative_weights.begin(), it);
        }
    };

    class RandomSelectionStrategy : public SelectionStrategy {
    private:
        utils::Random<std::mt19937, std::uniform_int_distribution<>>
            random_index_;

    public:
        explicit RandomSelectionStrategy(size_t max_index)
            : random_index_(0, max_index - 1) {}

        auto select(std::span<const T>  /*cumulative_weights*/,
                      T /*total_weight*/) -> size_t override {
            return random_index_();
        }
    };

    class WeightedRandomSampler {
    public:
        auto sample(std::span<const T> weights, size_t n) -> std::vector<size_t> {
            std::vector<size_t> indices(weights.size());
            std::iota(indices.begin(), indices.end(), 0);

            utils::Random<std::mt19937, std::discrete_distribution<>> random(
                weights);
            std::vector<size_t> results(n);
            std::generate(results.begin(), results.end(),
                          [&]() { return random(); });

            return results;
        }
    };

private:
    std::vector<T> weights_;
    std::vector<T> cumulative_weights_;
    std::unique_ptr<SelectionStrategy> strategy_;

    void updateCumulativeWeights() {
        cumulative_weights_.resize(weights_.size());
        std::exclusive_scan(weights_.begin(), weights_.end(),
                            cumulative_weights_.begin(), T{0});
    }

public:
    explicit WeightSelector(std::span<const T> input_weights,
                            std::unique_ptr<SelectionStrategy> custom_strategy =
                                std::make_unique<DefaultSelectionStrategy>())
        : weights_(input_weights.begin(), input_weights.end()),
          strategy_(std::move(custom_strategy)) {
        updateCumulativeWeights();
    }

    void setSelectionStrategy(std::unique_ptr<SelectionStrategy> new_strategy) {
        strategy_ = std::move(new_strategy);
    }

    auto select() -> size_t {
        T totalWeight = std::reduce(weights_.begin(), weights_.end());
        return strategy_->select(cumulative_weights_, totalWeight);
    }

    auto selectMultiple(size_t n) -> std::vector<size_t> {
        std::vector<size_t> results;
        results.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            results.push_back(select());
        }
        return results;
    }

    void updateWeight(size_t index, T new_weight) {
        if (index >= weights_.size()) {
            THROW_OUT_OF_RANGE("Index out of range");
        }
        weights_[index] = new_weight;
        updateCumulativeWeights();
    }

    void addWeight(T new_weight) {
        weights_.push_back(new_weight);
        updateCumulativeWeights();
    }

    void removeWeight(size_t index) {
        if (index >= weights_.size()) {
            THROW_OUT_OF_RANGE("Index out of range");
        }
        weights_.erase(weights_.begin() + index);
        updateCumulativeWeights();
    }

    void normalizeWeights() {
        T sum = std::reduce(weights_.begin(), weights_.end());
        if (sum > T{0}) {
            std::ranges::transform(weights_, weights_.begin(),
                                   [sum](T w) { return w / sum; });
            updateCumulativeWeights();
        }
    }

    void applyFunctionToWeights(std::invocable<T> auto&& func) {
        std::ranges::transform(weights_, weights_.begin(),
                               std::forward<decltype(func)>(func));
        updateCumulativeWeights();
    }

    void batchUpdateWeights(const std::vector<std::pair<size_t, T>>& updates) {
        for (const auto& [index, new_weight] : updates) {
            if (index >= weights_.size()) {
                THROW_OUT_OF_RANGE("Index out of range");
            }
            weights_[index] = new_weight;
        }
        updateCumulativeWeights();
    }

    [[nodiscard]] auto getWeight(size_t index) const -> std::optional<T> {
        if (index >= weights_.size()) {
            return std::nullopt;
        }
        return weights_[index];
    }

    [[nodiscard]] auto getMaxWeightIndex() const -> size_t {
        return std::distance(weights_.begin(),
                             std::ranges::max_element(weights_));
    }

    [[nodiscard]] auto getMinWeightIndex() const -> size_t {
        return std::distance(weights_.begin(),
                             std::ranges::min_element(weights_));
    }

    [[nodiscard]] auto size() const -> size_t { return weights_.size(); }

    [[nodiscard]] auto getWeights() const -> std::span<const T> {
        return weights_;
    }

    [[nodiscard]] auto getTotalWeight() const -> T {
        return std::reduce(weights_.begin(), weights_.end());
    }

    void resetWeights(const std::vector<T>& new_weights) {
        weights_ = new_weights;
        updateCumulativeWeights();
    }

    void scaleWeights(T factor) {
        std::ranges::transform(weights_, weights_.begin(),
                               [factor](T w) { return w * factor; });
        updateCumulativeWeights();
    }

    [[nodiscard]] auto getAverageWeight() const -> T {
        return getTotalWeight() / static_cast<T>(weights_.size());
    }

    void printWeights(std::ostream& oss) const {
        oss << std::format("[{:.2f}", weights_.front());
        for (auto it = weights_.begin() + 1; it != weights_.end(); ++it) {
            oss << std::format(", {:.2f}", *it);
        }
        oss << "]\n";
    }
};

template <Arithmetic T>
class TopHeavySelectionStrategy : public WeightSelector<T>::SelectionStrategy {
private:
    utils::Random<std::mt19937, std::uniform_real_distribution<>> random_;

public:
    TopHeavySelectionStrategy() : random_(0.0, 1.0) {}

    auto select(std::span<const T> cumulative_weights,
                  T total_weight) -> size_t override {
        T randomValue = std::pow(random_(), 2) * total_weight;
        auto it = std::ranges::upper_bound(cumulative_weights, randomValue);
        return std::distance(cumulative_weights.begin(), it);
    }
};

}  // namespace atom::algorithm

#endif
