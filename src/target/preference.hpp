#ifndef ADVANCED_RECOMMENDATION_ENGINE_H
#define ADVANCED_RECOMMENDATION_ENGINE_H

#include <chrono>
#include <cmath>
#include <unordered_map>
#include <vector>

#include <eigen3/Eigen/Dense>

class AdvancedRecommendationEngine {
private:
    std::unordered_map<std::string, int> userIndex_;
    std::unordered_map<std::string, int> itemIndex_;
    std::vector<
        std::tuple<int, int, double, std::chrono::system_clock::time_point>>
        ratings_;
    Eigen::MatrixXd userFactors_;
    Eigen::MatrixXd itemFactors_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>>
        itemFeatures_;
    std::vector<std::vector<int>> userItemGraph_;

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

    auto getUserId(const std::string& user) -> int;
    auto getItemId(const std::string& item) -> int;
    auto calculateTimeFactor(const std::chrono::system_clock::time_point&
                                 ratingTime) const -> double;
    void updateMatrixFactorization();
    void buildUserItemGraph();
    auto personalizedPageRank(int userId, double alpha = PPR_ALPHA,
                              int numIterations = PPR_ITERATIONS)
        -> std::vector<double>;

public:
    void addRating(const std::string& user, const std::string& item,
                   double rating);
    void addItemFeature(const std::string& item, const std::string& feature,
                        double value);
    void train();
    void updateALS(int numIterations = ALS_ITERATIONS);
    auto recommendItems(const std::string& user, int topN = 5)
        -> std::vector<std::pair<std::string, double>>;
    auto predictRating(const std::string& user,
                       const std::string& item) -> double;
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);
};

#endif  // ADVANCED_RECOMMENDATION_ENGINE_H
