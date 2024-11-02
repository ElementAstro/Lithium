#include "preference.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>

#include "atom/log/loguru.hpp"

auto AdvancedRecommendationEngine::getUserId(const std::string& user) -> int {
    std::lock_guard<std::mutex> lock(mtx_);
    if (userIndex_.find(user) == userIndex_.end()) {
        int newIndex = static_cast<int>(userIndex_.size());
        userIndex_[user] = newIndex;
    }
    return userIndex_[user];
}

auto AdvancedRecommendationEngine::getItemId(const std::string& item) -> int {
    std::lock_guard<std::mutex> lock(mtx_);
    if (itemIndex_.find(item) == itemIndex_.end()) {
        int newIndex = static_cast<int>(itemIndex_.size());
        itemIndex_[item] = newIndex;
    }
    return itemIndex_[item];
}

auto AdvancedRecommendationEngine::calculateTimeFactor(
    const std::chrono::system_clock::time_point& ratingTime) const -> double {
    auto now = std::chrono::system_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::hours>(now - ratingTime);
    return std::exp(-TIME_DECAY_FACTOR * static_cast<double>(duration.count()) /
                    (HOURS_IN_A_DAY * DAYS_IN_A_YEAR));  // Decay over years
}

void AdvancedRecommendationEngine::normalizeRatings() {
    double mean = 0.0;
    if (!ratings_.empty()) {
        mean = std::accumulate(ratings_.begin(), ratings_.end(), 0.0,
                               [&](double sum, const auto& tup) {
                                   return sum + std::get<2>(tup);
                               }) /
               ratings_.size();
    }
    for (auto& tup : ratings_) {
        std::get<2>(tup) -= mean;
    }
}

void AdvancedRecommendationEngine::updateMatrixFactorization() {
    std::lock_guard<std::mutex> lock(mtx_);
    normalizeRatings();
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_real_distribution<> distribution(-RANDOM_INIT_RANGE,
                                                  RANDOM_INIT_RANGE);

    int numUsers = static_cast<int>(userIndex_.size());
    int numItems = static_cast<int>(itemIndex_.size());

    userFactors_ =
        Eigen::MatrixXd::Random(numUsers, LATENT_FACTORS) * RANDOM_INIT_RANGE;
    itemFactors_ =
        Eigen::MatrixXd::Random(numItems, LATENT_FACTORS) * RANDOM_INIT_RANGE;

    for (int iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
        for (const auto& [userId, itemId, rating, timestamp] : ratings_) {
            double timeFactor = calculateTimeFactor(timestamp);
            Eigen::VectorXd userVec = userFactors_.row(userId);
            Eigen::VectorXd itemVec = itemFactors_.row(itemId);

            double prediction = userVec.dot(itemVec);
            double error = timeFactor * (rating - prediction);

            userFactors_.row(userId) +=
                LEARNING_RATE * (error * itemVec - REGULARIZATION * userVec);
            itemFactors_.row(itemId) +=
                LEARNING_RATE * (error * userVec - REGULARIZATION * itemVec);
        }
    }
}

void AdvancedRecommendationEngine::buildUserItemGraph() {
    std::lock_guard<std::mutex> lock(mtx_);
    int numUsers = static_cast<int>(userIndex_.size());
    int numItems = static_cast<int>(itemIndex_.size());
    userItemGraph_.clear();
    userItemGraph_.resize(numUsers + numItems);

    for (const auto& [userId, itemId, rating, _] : ratings_) {
        userItemGraph_[userId].push_back(numUsers + itemId);
        userItemGraph_[numUsers + itemId].push_back(userId);
    }
}

auto AdvancedRecommendationEngine::personalizedPageRank(
    int userId, double alpha, int numIterations) -> std::vector<double> {
    std::lock_guard<std::mutex> lock(mtx_);
    int numNodes = static_cast<int>(userItemGraph_.size());
    std::vector<double> ppr(numNodes, 0.0);
    std::vector<double> nextPpr(numNodes, 0.0);
    ppr[userId] = 1.0;

    for (int i = 0; i < numIterations; ++i) {
        for (int node = 0; node < numNodes; ++node) {
            if (!userItemGraph_[node].empty()) {
                double contribution =
                    ppr[node] /
                    static_cast<double>(userItemGraph_[node].size());
                for (int neighbor : userItemGraph_[node]) {
                    nextPpr[neighbor] += alpha * contribution;
                }
            }
        }
        for (int node = 0; node < numNodes; ++node) {
            nextPpr[node] += (1 - alpha) * (node == userId ? 1.0 : 0.0);
            ppr[node] = nextPpr[node];
            nextPpr[node] = 0.0;
        }
    }

    return ppr;
}

void AdvancedRecommendationEngine::addRating(const std::string& user,
                                             const std::string& item,
                                             double rating) {
    if (rating < 0.0 || rating > 5.0) {
        throw DataException("Rating must be between 0 and 5.");
    }
    std::lock_guard<std::mutex> lock(mtx_);
    int userId = getUserId(user);
    int itemId = getItemId(item);
    ratings_.emplace_back(userId, itemId, rating,
                          std::chrono::system_clock::now());
}

void AdvancedRecommendationEngine::addImplicitFeedback(
    const std::string& user, const std::string& item) {
    std::lock_guard<std::mutex> lock(mtx_);
    int userId = getUserId(user);
    int itemId = getItemId(item);
    // Using a default high implicit rating
    ratings_.emplace_back(userId, itemId, 4.5,
                          std::chrono::system_clock::now());
}

void AdvancedRecommendationEngine::addItemFeature(const std::string& item,
                                                  const std::string& feature,
                                                  double value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (value < 0.0 || value > 1.0) {
        throw DataException("Feature value must be between 0 and 1.");
    }
    itemFeatures_[item][feature] = value;
}

void AdvancedRecommendationEngine::train() {
    try {
        updateMatrixFactorization();
        buildUserItemGraph();
    } catch (const std::exception& e) {
        throw ModelException(std::string("Training failed: ") + e.what());
    }
}

void AdvancedRecommendationEngine::incrementTrain(int numIterations) {
    std::lock_guard<std::mutex> lock(mtx_);
    try {
        int numUsers = static_cast<int>(userIndex_.size());
        int numItems = static_cast<int>(itemIndex_.size());

        Eigen::MatrixXd ratingMatrix =
            Eigen::MatrixXd::Zero(numUsers, numItems);
        for (const auto& [userId, itemId, rating, _] : ratings_) {
            ratingMatrix(userId, itemId) = rating;
        }

        for (int iteration = 0; iteration < numIterations; ++iteration) {
            // Update user factors
#pragma omp parallel for
            for (int userIdx = 0; userIdx < numUsers; ++userIdx) {
                Eigen::MatrixXd A =
                    itemFactors_.transpose() * itemFactors_ +
                    REGULARIZATION * Eigen::MatrixXd::Identity(LATENT_FACTORS,
                                                               LATENT_FACTORS);
                Eigen::VectorXd b = itemFactors_.transpose() *
                                    ratingMatrix.row(userIdx).transpose();
                userFactors_.row(userIdx) = A.ldlt().solve(b);
            }

            // Update item factors
#pragma omp parallel for
            for (int itemIdx = 0; itemIdx < numItems; ++itemIdx) {
                Eigen::MatrixXd A =
                    userFactors_.transpose() * userFactors_ +
                    REGULARIZATION * Eigen::MatrixXd::Identity(LATENT_FACTORS,
                                                               LATENT_FACTORS);
                Eigen::VectorXd b =
                    userFactors_.transpose() * ratingMatrix.col(itemIdx);
                itemFactors_.row(itemIdx) = A.ldlt().solve(b);
            }
        }
    } catch (const std::exception& e) {
        throw ModelException(std::string("Incremental training failed: ") +
                             e.what());
    }
}

auto AdvancedRecommendationEngine::evaluate(
    const std::vector<std::tuple<std::string, std::string, double>>&
        testRatings) -> std::pair<double, double> {
    if (testRatings.empty()) {
        throw DataException("Test ratings are empty.");
    }

    double total = 0.0;
    double correct = 0.0;
    double recall = 0.0;

    for (const auto& [user, item, actualRating] : testRatings) {
        double predictedRating = predictRating(user, item);
        total += 1.0;
        if (std::abs(predictedRating - actualRating) < 0.5) {  // 简单准确率定义
            correct += 1.0;
        }
        if (actualRating >= 4.0 && predictedRating >= 4.0) {  // 简单召回率定义
            recall += 1.0;
        }
    }

    double precision = (total > 0) ? (correct / total) : 0.0;
    double recallRate =
        (testRatings.size() > 0) ? (recall / testRatings.size()) : 0.0;

    return {precision, recallRate};
}

auto AdvancedRecommendationEngine::recommendItems(const std::string& user,
                                                  int topN)
    -> std::vector<std::pair<std::string, double>> {
    std::lock_guard<std::mutex> lock(mtx_);
    int userId = getUserId(user);
    std::unordered_map<int, double> scores;

    // Matrix Factorization
    Eigen::VectorXd userVec = userFactors_.row(userId);
    for (const auto& [item, id] : itemIndex_) {
        Eigen::VectorXd itemVec = itemFactors_.row(id);
        scores[id] += userVec.dot(itemVec);
    }

    // Content-Boosted CF
    for (const auto& [item, features] : itemFeatures_) {
        int itemId = getItemId(item);
        double featureScore = 0.0;
        for (const auto& [feature, value] : features) {
            featureScore += value;
        }
        scores[itemId] += CONTENT_BOOST_WEIGHT * featureScore;
    }

    // Graph-based Recommendation
    std::vector<double> ppr = personalizedPageRank(userId);
    int numUsers = static_cast<int>(userIndex_.size());
    for (int itemId = 0; itemId < static_cast<int>(ppr.size()) - numUsers;
         ++itemId) {
        scores[itemId] += GRAPH_BOOST_WEIGHT * ppr[numUsers + itemId];
    }

    // Convert scores to vector of pairs for sorting
    std::vector<std::pair<std::string, double>> recommendations;
    for (const auto& [item, id] : itemIndex_) {
        if (scores.find(id) != scores.end()) {
            recommendations.emplace_back(item, scores[id]);
        }
    }

    // Sort and get top N recommendations
    std::partial_sort(
        recommendations.begin(),
        recommendations.begin() +
            std::min(topN, static_cast<int>(recommendations.size())),
        recommendations.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.second > rhs.second;
        });

    recommendations.resize(
        std::min(topN, static_cast<int>(recommendations.size())));
    return recommendations;
}

auto AdvancedRecommendationEngine::predictRating(
    const std::string& user, const std::string& item) -> double {
    std::lock_guard<std::mutex> lock(mtx_);
    int userId = getUserId(user);
    int itemId = getItemId(item);

    Eigen::VectorXd userVec = userFactors_.row(userId);
    Eigen::VectorXd itemVec = itemFactors_.row(itemId);

    return userVec.dot(itemVec);
}

void AdvancedRecommendationEngine::saveModel(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw ModelException("Unable to open file for writing: " + filename);
    }

    try {
        // Save user and item indices
        size_t userSize = userIndex_.size();
        size_t itemSize = itemIndex_.size();
        file.write(reinterpret_cast<const char*>(&userSize), sizeof(userSize));
        file.write(reinterpret_cast<const char*>(&itemSize), sizeof(itemSize));

        for (const auto& [user, id] : userIndex_) {
            size_t len = user.length();
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(user.data(), len);
            file.write(reinterpret_cast<const char*>(&id), sizeof(id));
        }

        for (const auto& [item, id] : itemIndex_) {
            size_t len = item.length();
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(item.data(), len);
            file.write(reinterpret_cast<const char*>(&id), sizeof(id));
        }

        // Save matrix factors
        int numUsers = static_cast<int>(userIndex_.size());
        int numItems = static_cast<int>(itemIndex_.size());

        file.write(reinterpret_cast<const char*>(userFactors_.data()),
                   userFactors_.size() * sizeof(double));
        file.write(reinterpret_cast<const char*>(itemFactors_.data()),
                   itemFactors_.size() * sizeof(double));

        // Save item features
        size_t featureSize = itemFeatures_.size();
        file.write(reinterpret_cast<const char*>(&featureSize),
                   sizeof(featureSize));
        for (const auto& [item, features] : itemFeatures_) {
            size_t itemLen = item.length();
            file.write(reinterpret_cast<const char*>(&itemLen),
                       sizeof(itemLen));
            file.write(item.data(), itemLen);

            size_t numFeatures = features.size();
            file.write(reinterpret_cast<const char*>(&numFeatures),
                       sizeof(numFeatures));
            for (const auto& [feature, value] : features) {
                size_t featureLen = feature.length();
                file.write(reinterpret_cast<const char*>(&featureLen),
                           sizeof(featureLen));
                file.write(feature.data(), featureLen);
                file.write(reinterpret_cast<const char*>(&value),
                           sizeof(value));
            }
        }

    } catch (const std::exception& e) {
        throw ModelException(std::string("Error during model saving: ") +
                             e.what());
    }
}

void AdvancedRecommendationEngine::loadModel(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw ModelException("Unable to open file for reading: " + filename);
    }

    try {
        // Load user and item indices
        size_t userSize, itemSize;
        file.read(reinterpret_cast<char*>(&userSize), sizeof(userSize));
        file.read(reinterpret_cast<char*>(&itemSize), sizeof(itemSize));

        userIndex_.clear();
        itemIndex_.clear();

        for (size_t i = 0; i < userSize; ++i) {
            size_t len;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string user(len, '\0');
            file.read(&user[0], len);
            int id;
            file.read(reinterpret_cast<char*>(&id), sizeof(id));
            userIndex_[user] = id;
        }

        for (size_t i = 0; i < itemSize; ++i) {
            size_t len;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string item(len, '\0');
            file.read(&item[0], len);
            int id;
            file.read(reinterpret_cast<char*>(&id), sizeof(id));
            itemIndex_[item] = id;
        }

        // Load matrix factors
        int numUsers = static_cast<int>(userIndex_.size());
        int numItems = static_cast<int>(itemIndex_.size());

        userFactors_.resize(numUsers, LATENT_FACTORS);
        itemFactors_.resize(numItems, LATENT_FACTORS);

        file.read(reinterpret_cast<char*>(userFactors_.data()),
                  userFactors_.size() * sizeof(double));
        file.read(reinterpret_cast<char*>(itemFactors_.data()),
                  itemFactors_.size() * sizeof(double));

        // Load item features
        size_t featureSize;
        file.read(reinterpret_cast<char*>(&featureSize), sizeof(featureSize));
        itemFeatures_.clear();
        for (size_t i = 0; i < featureSize; ++i) {
            size_t itemLen;
            file.read(reinterpret_cast<char*>(&itemLen), sizeof(itemLen));
            std::string item(itemLen, '\0');
            file.read(&item[0], itemLen);

            size_t numFeatures;
            file.read(reinterpret_cast<char*>(&numFeatures),
                      sizeof(numFeatures));
            for (size_t j = 0; j < numFeatures; ++j) {
                size_t featureLen;
                file.read(reinterpret_cast<char*>(&featureLen),
                          sizeof(featureLen));
                std::string feature(featureLen, '\0');
                file.read(&feature[0], featureLen);
                double value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                itemFeatures_[item][feature] = value;
            }
        }

    } catch (const std::exception& e) {
        throw ModelException(std::string("Error during model loading: ") +
                             e.what());
    }
}