#ifndef ADVANCED_RECOMMENDATION_ENGINE_H
#define ADVANCED_RECOMMENDATION_ENGINE_H

#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <eigen3/Eigen/Dense>

// 自定义异常类
class RecommendationEngineException : public std::runtime_error {
public:
    explicit RecommendationEngineException(const std::string& message)
        : std::runtime_error(message) {}
};

class DataException : public RecommendationEngineException {
public:
    explicit DataException(const std::string& message)
        : RecommendationEngineException(message) {}
};

class ModelException : public RecommendationEngineException {
public:
    explicit ModelException(const std::string& message)
        : RecommendationEngineException(message) {}
};

class AdvancedRecommendationEngine {
private:
    // 索引映射
    std::unordered_map<std::string, int> userIndex_;
    std::unordered_map<std::string, int> itemIndex_;
    std::unordered_map<std::string, int> featureIndex_;

    // 评分数据和特征数据
    std::vector<
        std::tuple<int, int, double, std::chrono::system_clock::time_point>>
        ratings_;
    std::unordered_map<int, std::unordered_map<int, double>> itemFeatures_;

    // 矩阵分解参数
    Eigen::MatrixXd userFactors_;
    Eigen::MatrixXd itemFactors_;

    // 参数
    static constexpr int LATENT_FACTORS = 20;
    static constexpr double LEARNING_RATE = 0.01;
    static constexpr double REGULARIZATION = 0.02;
    static constexpr int MAX_ITERATIONS = 100;
    static constexpr double TIME_DECAY_FACTOR = 0.1;
    static constexpr double HOURS_IN_A_DAY = 24.0;
    static constexpr double DAYS_IN_A_YEAR = 365.0;
    static constexpr double RANDOM_INIT_RANGE = 0.01;
    static constexpr double CONTENT_BOOST_WEIGHT = 0.2;
    static constexpr double GRAPH_BOOST_WEIGHT = 0.3;
    static constexpr double PPR_ALPHA = 0.85;
    static constexpr int PPR_ITERATIONS = 20;
    static constexpr int ALS_ITERATIONS = 10;

    mutable std::mutex mtx_;  // 互斥锁确保线程安全

    // 内部方法
    auto getUserId(const std::string& user) -> int;
    auto getItemId(const std::string& item) -> int;
    auto getFeatureId(const std::string& feature) -> int;
    auto calculateTimeFactor(const std::chrono::system_clock::time_point&
                                 ratingTime) const -> double;
    void updateMatrixFactorization();
    void normalizeRatings();

public:
    AdvancedRecommendationEngine() = default;
    ~AdvancedRecommendationEngine() = default;

    // 添加数据的方法
    void addRating(const std::string& user, const std::string& item,
                   double rating);
    void addImplicitFeedback(const std::string& user, const std::string& item);
    void addItem(const std::string& item,
                 const std::vector<std::string>& features);
    void addItemFeature(const std::string& item, const std::string& feature,
                        double value);

    // 训练和预测的方法
    void train();
    auto recommendItems(const std::string& user, int topN = 5)
        -> std::vector<std::pair<std::string, double>>;
    auto predictRating(const std::string& user,
                       const std::string& item) -> double;

    // 模型持久化
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);
};

#endif  // ADVANCED_RECOMMENDATION_ENGINE_H