#include "preference.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>

#include "atom/log/loguru.hpp"

// 获取或创建用户 ID
auto AdvancedRecommendationEngine::getUserId(const std::string& user) -> int {
    std::lock_guard<std::mutex> lock(mtx_);
    auto userIterator = userIndex_.find(user);
    if (userIterator == userIndex_.end()) {
        int newIndex = static_cast<int>(userIndex_.size());
        userIndex_[user] = newIndex;
        LOG_F(INFO, "New user added: {} with ID: {}", user, newIndex);
        return newIndex;
    }
    return userIterator->second;
}

// 获取或创建物品 ID
auto AdvancedRecommendationEngine::getItemId(const std::string& item) -> int {
    std::lock_guard<std::mutex> lock(mtx_);
    auto itemIterator = itemIndex_.find(item);
    if (itemIterator == itemIndex_.end()) {
        int newIndex = static_cast<int>(itemIndex_.size());
        itemIndex_[item] = newIndex;
        LOG_F(INFO, "New item added: {} with ID: {}", item, newIndex);
        return newIndex;
    }
    return itemIterator->second;
}

// 获取或创建特征 ID
auto AdvancedRecommendationEngine::getFeatureId(const std::string& feature)
    -> int {
    std::lock_guard<std::mutex> lock(mtx_);
    auto featureIterator = featureIndex_.find(feature);
    if (featureIterator == featureIndex_.end()) {
        int newIndex = static_cast<int>(featureIndex_.size());
        featureIndex_[feature] = newIndex;
        LOG_F(INFO, "New feature added: {} with ID: {}", feature, newIndex);
        return newIndex;
    }
    return featureIterator->second;
}

// 计算时间衰减因子
auto AdvancedRecommendationEngine::calculateTimeFactor(
    const std::chrono::system_clock::time_point& ratingTime) const -> double {
    auto now = std::chrono::system_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::hours>(now - ratingTime);
    double timeFactor =
        std::exp(-TIME_DECAY_FACTOR * static_cast<double>(duration.count()) /
                 (HOURS_IN_A_DAY * DAYS_IN_A_YEAR));  // 按年衰减
    return timeFactor;
}

// 规范化评分
void AdvancedRecommendationEngine::normalizeRatings() {
    std::lock_guard<std::mutex> lock(mtx_);
    LOG_F(INFO, "Starting normalization of ratings.");
    double mean = 0.0;
    if (!ratings_.empty()) {
        mean = std::accumulate(ratings_.begin(), ratings_.end(), 0.0,
                               [&](double sum, const auto& tup) {
                                   return sum + std::get<2>(tup);
                               }) /
               ratings_.size();
    }
    for (auto& tup : ratings_) {
        std::get<2>(tup) -= mean;  // 减去平均值
    }
    LOG_F(INFO, "Ratings normalization completed.");
}

// 更新矩阵分解
void AdvancedRecommendationEngine::updateMatrixFactorization() {
    std::lock_guard<std::mutex> lock(mtx_);
    LOG_F(INFO, "Starting matrix factorization update.");

    size_t numUsers = userIndex_.size();
    size_t numItems = itemIndex_.size();

    // 初始化用户和物品因子矩阵
    if (userFactors_.rows() != static_cast<int>(numUsers) ||
        userFactors_.cols() != LATENT_FACTORS) {
        userFactors_ = Eigen::MatrixXd::Random(numUsers, LATENT_FACTORS) *
                       RANDOM_INIT_RANGE;
    }
    if (itemFactors_.rows() != static_cast<int>(numItems) ||
        itemFactors_.cols() != LATENT_FACTORS) {
        itemFactors_ = Eigen::MatrixXd::Random(numItems, LATENT_FACTORS) *
                       RANDOM_INIT_RANGE;
    }

    // 开始迭代
    for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
        for (const auto& [userId, itemId, rating, timestamp] : ratings_) {
            double pred =
                userFactors_.row(userId).dot(itemFactors_.row(itemId));
            double err = rating - pred;
            Eigen::VectorXd userVec = userFactors_.row(userId);
            Eigen::VectorXd itemVec = itemFactors_.row(itemId);

            userFactors_.row(userId) +=
                LEARNING_RATE * (err * itemVec - REGULARIZATION * userVec);
            itemFactors_.row(itemId) +=
                LEARNING_RATE * (err * userVec - REGULARIZATION * itemVec);
        }
    }
    LOG_F(INFO, "Matrix factorization update completed.");
}

// 添加评分
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

// 添加物品
void AdvancedRecommendationEngine::addItem(
    const std::string& item, const std::vector<std::string>& features) {
    std::lock_guard<std::mutex> lock(mtx_);
    int itemId = getItemId(item);
    for (const auto& feature : features) {
        int featureId = getFeatureId(feature);
        itemFeatures_[itemId][featureId] = 1.0;  // 二进制特征，存在即为 1.0
    }
}

// 添加物品特征
void AdvancedRecommendationEngine::addItemFeature(const std::string& item,
                                                  const std::string& feature,
                                                  double value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (value < 0.0 || value > 1.0) {
        throw DataException("Feature value must be between 0 and 1.");
    }
    int itemId = getItemId(item);
    int featureId = getFeatureId(feature);
    itemFeatures_[itemId][featureId] = value;
}

// 训练模型
void AdvancedRecommendationEngine::train() {
    LOG_F(INFO, "Starting model training.");
    normalizeRatings();
    updateMatrixFactorization();
    LOG_F(INFO, "Model training completed.");
}

// 生成推荐
auto AdvancedRecommendationEngine::recommendItems(const std::string& user,
                                                  int topN)
    -> std::vector<std::pair<std::string, double>> {
    std::lock_guard<std::mutex> lock(mtx_);
    int userId = getUserId(user);
    std::unordered_map<int, double> scores;

    // 矩阵分解评分
    Eigen::VectorXd userVec = userFactors_.row(userId);
    for (int itemId = 0; itemId < itemFactors_.rows(); ++itemId) {
        double score = userVec.dot(itemFactors_.row(itemId));
        scores[itemId] = score;
    }

    // 排序并获取前 topN 个物品
    std::vector<std::pair<int, double>> scoredItems(scores.begin(),
                                                    scores.end());
    std::sort(scoredItems.begin(), scoredItems.end(),
              [](const auto& itemA, const auto& itemB) {
                  return itemA.second > itemB.second;
              });

    std::vector<std::pair<std::string, double>> recommendations;
    for (const auto& [itemId, score] : scoredItems) {
        const auto& itemNameIt = std::find_if(
            itemIndex_.begin(), itemIndex_.end(),
            [itemId](const auto& pair) { return pair.second == itemId; });
        if (itemNameIt != itemIndex_.end()) {
            recommendations.emplace_back(itemNameIt->first, score);
        }
        if (recommendations.size() >= static_cast<size_t>(topN)) {
            break;
        }
    }

    return recommendations;
}

// 预测评分
auto AdvancedRecommendationEngine::predictRating(
    const std::string& user, const std::string& item) -> double {
    std::lock_guard<std::mutex> lock(mtx_);
    int userId = getUserId(user);
    int itemId = getItemId(item);
    return userFactors_.row(userId).dot(itemFactors_.row(itemId));
}

// 保存模型到文件
void AdvancedRecommendationEngine::saveModel(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw ModelException("Failed to open file for saving: " + filename);
    }

    // 保存用户索引
    size_t userIndexSize = userIndex_.size();
    file.write(reinterpret_cast<const char*>(&userIndexSize), sizeof(size_t));
    for (const auto& [user, userId] : userIndex_) {
        size_t length = user.size();
        file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
        file.write(user.data(), static_cast<std::streamsize>(length));
        file.write(reinterpret_cast<const char*>(&userId), sizeof(int));
    }

    // 保存物品索引
    size_t itemIndexSize = itemIndex_.size();
    file.write(reinterpret_cast<const char*>(&itemIndexSize), sizeof(size_t));
    for (const auto& [item, itemId] : itemIndex_) {
        size_t length = item.size();
        file.write(reinterpret_cast<const char*>(&length), sizeof(size_t));
        file.write(item.data(), static_cast<std::streamsize>(length));
        file.write(reinterpret_cast<const char*>(&itemId), sizeof(int));
    }

    // 保存用户因子矩阵
    int rows = userFactors_.rows();
    int cols = userFactors_.cols();
    file.write(reinterpret_cast<const char*>(&rows), sizeof(int));
    file.write(reinterpret_cast<const char*>(&cols), sizeof(int));
    file.write(reinterpret_cast<const char*>(userFactors_.data()),
               static_cast<std::streamsize>(sizeof(double) * rows * cols));

    // 保存物品因子矩阵
    rows = itemFactors_.rows();
    cols = itemFactors_.cols();
    file.write(reinterpret_cast<const char*>(&rows), sizeof(int));
    file.write(reinterpret_cast<const char*>(&cols), sizeof(int));
    file.write(reinterpret_cast<const char*>(itemFactors_.data()),
               static_cast<std::streamsize>(sizeof(double) * rows * cols));

    file.close();
    LOG_F(INFO, "Model saved successfully to {}", filename);
}

// 从文件加载模型
void AdvancedRecommendationEngine::loadModel(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw ModelException("Failed to open file for loading: " + filename);
    }

    // 加载用户索引
    size_t userIndexSize;
    file.read(reinterpret_cast<char*>(&userIndexSize), sizeof(size_t));
    for (size_t i = 0; i < userIndexSize; ++i) {
        size_t length;
        file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
        std::string user(length, '\0');
        file.read(user.data(), static_cast<std::streamsize>(length));
        int userId;
        file.read(reinterpret_cast<char*>(&userId), sizeof(int));
        userIndex_[user] = userId;
    }

    // 加载物品索引
    size_t itemIndexSize;
    file.read(reinterpret_cast<char*>(&itemIndexSize), sizeof(size_t));
    for (size_t i = 0; i < itemIndexSize; ++i) {
        size_t length;
        file.read(reinterpret_cast<char*>(&length), sizeof(size_t));
        std::string item(length, '\0');
        file.read(item.data(), static_cast<std::streamsize>(length));
        int itemId;
        file.read(reinterpret_cast<char*>(&itemId), sizeof(int));
        itemIndex_[item] = itemId;
    }

    // 加载用户因子矩阵
    int rows, cols;
    file.read(reinterpret_cast<char*>(&rows), sizeof(int));
    file.read(reinterpret_cast<char*>(&cols), sizeof(int));
    userFactors_.resize(rows, cols);
    file.read(reinterpret_cast<char*>(userFactors_.data()),
              static_cast<std::streamsize>(sizeof(double) * rows * cols));

    // 加载物品因子矩阵
    file.read(reinterpret_cast<char*>(&rows), sizeof(int));
    file.read(reinterpret_cast<char*>(&cols), sizeof(int));
    itemFactors_.resize(rows, cols);
    file.read(reinterpret_cast<char*>(itemFactors_.data()),
              static_cast<std::streamsize>(sizeof(double) * rows * cols));

    file.close();
    LOG_F(INFO, "Model loaded successfully from {}", filename);
}