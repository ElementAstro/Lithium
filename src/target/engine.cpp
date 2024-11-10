#include "engine.hpp"
#include "preference.hpp"  // 引入偏好引擎

#include <algorithm>
#include <fstream>
#include <shared_mutex>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "atom/log/loguru.hpp"
#include "atom/search/lru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium::target {

// --------------------- CelestialObject Implementation ---------------------

auto CelestialObject::from_json(const json& j) -> CelestialObject {
    LOG_F(INFO, "Deserializing CelestialObject from JSON.");
    try {
        CelestialObject obj(
            j.at("ID").get<std::string>(), j.at("标识").get<std::string>(),
            j.at("M标识").get<std::string>(), j.at("拓展名").get<std::string>(),
            j.at("组件").get<std::string>(), j.at("Class").get<std::string>(),
            j.at("业余排名").get<std::string>(),
            j.at("中文名").get<std::string>(), j.at("类型").get<std::string>(),
            j.at("含重复类型").get<std::string>(),
            j.at("形态").get<std::string>(),
            j.at("星座(Zh)").get<std::string>(),
            j.at("星座(En)").get<std::string>(),
            j.at("赤经(J2000)").get<std::string>(),
            j.at("赤经D(J2000)").get<double>(),
            j.at("赤纬(J2000)").get<std::string>(),
            j.at("赤纬D(J2000)").get<double>(),
            j.at("可见光星等V").get<double>(),
            j.at("摄影(蓝光)星等B").get<double>(), j.at("B-V").get<double>(),
            j.at("表面亮度(mag/arcmin2)").get<double>(),
            j.at("长轴(分)").get<double>(), j.at("短轴(分)").get<double>(),
            j.at("方位角").get<int>(), j.at("详细描述").get<std::string>(),
            j.at("简略描述").get<std::string>());
        LOG_F(INFO, "Successfully deserialized CelestialObject with ID: {}",
              obj.ID);
        return obj;
    } catch (const json::exception& e) {
        LOG_F(ERROR,
              "JSON deserialization error in CelestialObject::from_json: {}",
              e.what());
        throw;
    }
}

// JSON序列化函数
auto CelestialObject::to_json() const -> json {
    LOG_F(INFO, "Serializing CelestialObject with ID: {}", ID);
    try {
        return json{{"ID", ID},
                    {"标识", Identifier},
                    {"M标识", MIdentifier},
                    {"拓展名", ExtensionName},
                    {"组件", Component},
                    {"Class", ClassName},
                    {"业余排名", AmateurRank},
                    {"中文名", ChineseName},
                    {"类型", Type},
                    {"含重复类型", DuplicateType},
                    {"形态", Morphology},
                    {"星座(Zh)", ConstellationZh},
                    {"星座(En)", ConstellationEn},
                    {"赤经(J2000)", RAJ2000},
                    {"赤经D(J2000)", RADJ2000},
                    {"赤纬(J2000)", DecJ2000},
                    {"赤纬D(J2000)", DecDJ2000},
                    {"可见光星等V", VisualMagnitudeV},
                    {"摄影(蓝光)星等B", PhotographicMagnitudeB},
                    {"B-V", BMinusV},
                    {"表面亮度(mag/arcmin2)", SurfaceBrightness},
                    {"长轴(分)", MajorAxis},
                    {"短轴(分)", MinorAxis},
                    {"方位角", PositionAngle},
                    {"详细描述", DetailedDescription},
                    {"简略描述", BriefDescription}};
    } catch (const json::exception& e) {
        LOG_F(ERROR, "JSON serialization error in CelestialObject::to_json: {}",
              e.what());
        throw;
    }
}

// --------------------- StarObject Implementation ---------------------

StarObject::StarObject(std::string name, std::vector<std::string> aliases,
                       int clickCount)
    : name_(std::move(name)),
      aliases_(std::move(aliases)),
      clickCount_(clickCount) {
    LOG_F(INFO, "Constructed StarObject with name: {}", name_);
}

// 访问器方法实现
const std::string& StarObject::getName() const {
    LOG_F(INFO, "Accessing StarObject::getName for {}", name_);
    return name_;
}

const std::vector<std::string>& StarObject::getAliases() const {
    LOG_F(INFO, "Accessing StarObject::getAliases for {}", name_);
    return aliases_;
}

int StarObject::getClickCount() const {
    LOG_F(INFO, "Accessing StarObject::getClickCount for {}", name_);
    return clickCount_;
}

// 修改器方法实现
void StarObject::setName(const std::string& name) {
    LOG_F(INFO, "Setting name from {} to {}", name_, name);
    name_ = name;
}

void StarObject::setAliases(const std::vector<std::string>& aliases) {
    LOG_F(INFO, "Setting aliases for {}: {}", name_, [&aliases]() {
        std::stringstream ss;
        for (const auto& alias : aliases) {
            ss << alias << " ";
        }
        return ss.str();
    }());
    aliases_ = aliases;
}

void StarObject::setClickCount(int clickCount) {
    LOG_F(INFO, "Setting clickCount for {} to {}", name_, clickCount);
    clickCount_ = clickCount;
}

// JSON序列化函数实现
void StarObject::setCelestialObject(const CelestialObject& celestialObject) {
    LOG_F(INFO, "Associating CelestialObject with ID: {} to StarObject: {}",
          celestialObject.ID, name_);
    celestialObject_ = celestialObject;
}

CelestialObject StarObject::getCelestialObject() const {
    LOG_F(INFO, "Accessing CelestialObject for StarObject: {}", name_);
    return celestialObject_;
}

auto StarObject::to_json() const -> json {
    LOG_F(INFO, "Serializing StarObject: {}", name_);
    try {
        return json{{"name", name_},
                    {"aliases", aliases_},
                    {"clickCount", clickCount_},
                    {"celestialObject", celestialObject_.to_json()}};
    } catch (const json::exception& e) {
        LOG_F(ERROR,
              "JSON serialization error in StarObject::to_json for {}: {}",
              name_, e.what());
        throw;
    }
}

// --------------------- Trie Implementation ---------------------

Trie::Trie() {
    root_ = new TrieNode();
    LOG_F(INFO, "Initialized Trie.");
}

Trie::~Trie() {
    clear(root_);
    LOG_F(INFO, "Destroyed Trie.");
}

void Trie::insert(const std::string& word) {
    LOG_F(INFO, "Inserting word into Trie: {}", word);
    TrieNode* current = root_;
    for (const char& ch : word) {
        if (current->children.find(ch) == current->children.end()) {
            LOG_F(INFO, "Creating new TrieNode for character: {}", ch);
            current->children[ch] = new TrieNode();
        }
        current = current->children[ch];
    }
    current->isEndOfWord = true;
    LOG_F(INFO, "Finished inserting word into Trie: {}", word);
}

auto Trie::autoComplete(const std::string& prefix) const
    -> std::vector<std::string> {
    LOG_F(INFO, "Auto-completing prefix: {}", prefix);
    std::vector<std::string> suggestions;
    TrieNode* current = root_;
    for (const char& ch : prefix) {
        if (current->children.find(ch) == current->children.end()) {
            LOG_F(INFO, "Prefix '{}' not found in Trie.", prefix);
            return suggestions;  // Prefix not found
        }
        current = current->children[ch];
    }
    LOG_F(INFO, "Prefix '{}' found. Performing DFS for suggestions.", prefix);
    dfs(current, prefix, suggestions);
    LOG_F(INFO, "Auto-complete found {} suggestions for prefix: {}",
          suggestions.size(), prefix);
    return suggestions;
}

void Trie::dfs(TrieNode* node, const std::string& prefix,
               std::vector<std::string>& suggestions) const {
    if (node->isEndOfWord) {
        suggestions.emplace_back(prefix);
        LOG_F(INFO, "Found word in Trie during DFS: {}", prefix);
    }
    for (const auto& [ch, child] : node->children) {
        dfs(child, prefix + ch, suggestions);
    }
}

void Trie::clear(TrieNode* node) {
    if (node == nullptr) {
        return;
    }
    for (auto& [ch, child] : node->children) {
        clear(child);
    }
    LOG_F(INFO, "Deleting TrieNode.");
    delete node;
}

// --------------------- SearchEngine::Impl Implementation ---------------------

class SearchEngine::Impl {
public:
    static constexpr size_t CACHE_CAPACITY = 100;  // 定义缓存容量

    Impl() : queryCache_(CACHE_CAPACITY) {
        LOG_F(INFO, "SearchEngine initialized with cache capacity {}",
              CACHE_CAPACITY);
    }

    ~Impl() { LOG_F(INFO, "SearchEngine destroyed."); }

    // 初始化推荐引擎
    bool initializeRecommendationEngine(const std::string& modelFilename) {
        try {
            recommendationEngine_.loadModel(modelFilename);
            LOG_F(INFO, "Recommendation Engine loaded model from '{}'",
                  modelFilename);
            return true;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to load Recommendation Engine model: {}",
                  e.what());
            return false;
        }
    }

    // 添加星体对象
    void addStarObject(const StarObject& starObject) {
        std::unique_lock lock(indexMutex_);
        LOG_F(INFO, "Adding StarObject: {}", starObject.getName());
        try {
            auto result =
                starObjectIndex_.emplace(starObject.getName(), starObject);
            if (!result.second) {
                LOG_F(WARNING,
                      "StarObject with name '{}' already exists. Overwriting.",
                      starObject.getName());
                starObjectIndex_[starObject.getName()] = starObject;
            }
            trie_.insert(starObject.getName());
            for (const auto& alias : starObject.getAliases()) {
                trie_.insert(alias);
                aliasIndex_.emplace(alias, starObject.getName());
                LOG_F(INFO, "Added alias '{}' for StarObject '{}'.", alias,
                      starObject.getName());
            }
            LOG_F(INFO, "Successfully added StarObject: {}",
                  starObject.getName());

            // 添加到推荐引擎
            recommendationEngine_.addItem(starObject.getName(),
                                          starObject.getAliases());

        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in addStarObject for {}: {}",
                  starObject.getName(), e.what());
        }
    }

    // 添加用户评分
    void addUserRating(const std::string& user, const std::string& item,
                       double rating) {
        try {
            recommendationEngine_.addRating(user, item, rating);
            LOG_F(INFO, "Added rating: User '{}', Item '{}', Rating {}", user,
                  item, rating);
        } catch (const std::exception& e) {
            LOG_F(ERROR,
                  "Exception in addUserRating for user '{}', item '{}': {}",
                  user, item, e.what());
        }
    }

    // 搜索星体对象
    std::vector<StarObject> searchStarObject(const std::string& query) const {
        LOG_F(INFO, "Searching for StarObject with query: {}", query);
        std::shared_lock lock(indexMutex_);
        try {
            if (auto cached = queryCache_.get(query)) {
                LOG_F(INFO, "Cache hit for query: {}", query);
                return *cached;
            }

            std::vector<StarObject> results;
            auto it = starObjectIndex_.find(query);
            if (it != starObjectIndex_.end()) {
                results.emplace_back(it->second);
                LOG_F(INFO, "Found StarObject by name: {}", query);
            }

            // 通过别名搜索
            auto aliasRange = aliasIndex_.equal_range(query);
            for (auto itr = aliasRange.first; itr != aliasRange.second; ++itr) {
                auto starIt = starObjectIndex_.find(itr->second);
                if (starIt != starObjectIndex_.end()) {
                    results.emplace_back(starIt->second);
                    LOG_F(INFO, "Found StarObject '{}' by alias '{}'.",
                          starIt->second.getName(), query);
                }
            }

            if (!results.empty()) {
                queryCache_.put(query, results);
                LOG_F(INFO, "Search completed for query: {} with {} results.",
                      query, results.size());
            } else {
                LOG_F(INFO, "No results found for query: {}", query);
            }

            return results;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in searchStarObject for query '{}': {}",
                  query, e.what());
            return {};
        }
    }

    // 模糊搜索星体对象
    std::vector<StarObject> fuzzySearchStarObject(const std::string& query,
                                                  int tolerance) const {
        LOG_F(INFO,
              "Performing fuzzy search for query: '{}' with tolerance: {}",
              query, tolerance);
        std::shared_lock lock(indexMutex_);
        std::vector<StarObject> results;
        try {
            for (const auto& [name, starObject] : starObjectIndex_) {
                if (levenshteinDistance(query, name) <= tolerance) {
                    results.emplace_back(starObject);
                    LOG_F(INFO, "Fuzzy match found: {}", name);
                } else {
                    for (const auto& alias : starObject.getAliases()) {
                        if (levenshteinDistance(query, alias) <= tolerance) {
                            results.emplace_back(starObject);
                            LOG_F(INFO, "Fuzzy match found by alias: {}",
                                  alias);
                            break;
                        }
                    }
                }
            }
            LOG_F(INFO,
                  "Fuzzy search completed for query: '{}' with {} results.",
                  query, results.size());
            return results;
        } catch (const std::exception& e) {
            LOG_F(ERROR,
                  "Exception in fuzzySearchStarObject for query '{}': {}",
                  query, e.what());
            return {};
        }
    }

    // 自动完成星体对象名称
    std::vector<std::string> autoCompleteStarObject(
        const std::string& prefix) const {
        LOG_F(INFO, "Auto-completing StarObject with prefix: {}", prefix);
        try {
            auto suggestions = trie_.autoComplete(prefix);
            LOG_F(INFO, "Auto-complete retrieved {} suggestions for prefix: {}",
                  suggestions.size(), prefix);
            return suggestions;
        } catch (const std::exception& e) {
            LOG_F(ERROR,
                  "Exception in autoCompleteStarObject for prefix '{}': {}",
                  prefix, e.what());
            return {};
        }
    }

    // 获取排序后的结果
    static std::vector<StarObject> getRankedResultsStatic(
        std::vector<StarObject>& results) {
        LOG_F(INFO, "Ranking search results by click count.");
        std::sort(results.begin(), results.end(),
                  [](const StarObject& a, const StarObject& b) {
                      return a.getClickCount() > b.getClickCount();
                  });
        LOG_F(INFO, "Ranking completed. Top result click count: {}",
              results.empty() ? 0 : results[0].getClickCount());
        return results;
    }

    // 高级搜索 - 过滤功能
    std::vector<StarObject> filterSearch(const std::string& type,
                                         const std::string& morphology,
                                         double minMagnitude,
                                         double maxMagnitude) const {
        LOG_F(INFO,
              "Performing filtered search with type: '{}', morphology: '{}', "
              "magnitude range: {}-{}",
              type, morphology, minMagnitude, maxMagnitude);
        std::shared_lock lock(indexMutex_);
        std::vector<StarObject> results;
        try {
            for (const auto& [name, starObject] : starObjectIndex_) {
                const auto& celestial = starObject.getCelestialObject();
                bool matches = true;

                if (!type.empty() && celestial.Type != type) {
                    matches = false;
                }

                if (!morphology.empty() && celestial.Morphology != morphology) {
                    matches = false;
                }

                if (celestial.VisualMagnitudeV < minMagnitude ||
                    celestial.VisualMagnitudeV > maxMagnitude) {
                    matches = false;
                }

                if (matches) {
                    results.emplace_back(starObject);
                    LOG_F(INFO, "StarObject '{}' matches filter criteria.",
                          name);
                }
            }
            LOG_F(INFO, "Filtered search completed with {} results.",
                  results.size());
            return results;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in filterSearch: {}", e.what());
            return {};
        }
    }

    // 从 name.json 加载星体对象
    bool loadFromNameJson(const std::string& filename) {
        LOG_F(INFO, "Loading StarObjects from file: {}", filename);
        std::ifstream file(filename);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open file: {}", filename);
            return false;
        }

        json jsonData;
        try {
            file >> jsonData;
            LOG_F(INFO, "Successfully read JSON data from {}", filename);
        } catch (const json::parse_error& e) {
            LOG_F(ERROR, "JSON parsing error while reading {}: {}", filename,
                  e.what());
            return false;
        }

        size_t initialSize = starObjectIndex_.size();

        for (const auto& item : jsonData) {
            if (!item.is_array() || item.size() < 1) {
                LOG_F(WARNING, "Invalid entry in {}: {}", filename,
                      item.dump());
                continue;  // Skip invalid entries
            }
            std::string name = item[0].get<std::string>();
            std::vector<std::string> aliases;
            if (item.size() >= 2 && !item[1].is_null()) {
                // Assume aliases are comma-separated
                std::string aliasesStr = item[1].get<std::string>();
                std::stringstream ss(aliasesStr);
                std::string alias;
                while (std::getline(ss, alias, ',')) {
                    // Trim whitespace
                    size_t start = alias.find_first_not_of(" \t");
                    size_t end = alias.find_last_not_of(" \t");
                    if (start != std::string::npos &&
                        end != std::string::npos) {
                        aliases.emplace_back(
                            alias.substr(start, end - start + 1));
                        LOG_F(INFO, "Parsed alias '{}' for StarObject '{}'.",
                              alias.substr(start, end - start + 1), name);
                    }
                }
            }
            StarObject star(name, aliases,
                            0);  // Assuming default clickCount is 0
            addStarObject(star);
        }

        size_t loadedCount = starObjectIndex_.size() - initialSize;
        LOG_F(INFO, "Loaded {} StarObjects from {}", loadedCount, filename);
        return true;
    }

    // 从 ngc2019.json 加载 CelestialObjects 并与 StarObjects 关联
    bool loadFromCelestialJson(const std::string& filename) {
        LOG_F(INFO, "Loading CelestialObjects from file: {}", filename);
        std::ifstream file(filename);
        if (!file.is_open()) {
            LOG_F(ERROR, "Failed to open file: {}", filename);
            return false;
        }

        json jsonData;
        try {
            file >> jsonData;
            LOG_F(INFO, "Successfully read JSON data from {}", filename);
        } catch (const json::parse_error& e) {
            LOG_F(ERROR, "JSON parsing error while reading {}: {}", filename,
                  e.what());
            return false;
        }

        int matched = 0;
        int unmatched = 0;

        for (const auto& item : jsonData) {
            try {
                CelestialObject celestialObject =
                    CelestialObject::from_json(item);
                std::string name =
                    celestialObject
                        .getName();  // Assuming Identifier is the name

                // 查找对应的 StarObject
                auto it = starObjectIndex_.find(name);
                if (it != starObjectIndex_.end()) {
                    it->second.setCelestialObject(celestialObject);
                    matched++;
                    LOG_F(INFO,
                          "Associated CelestialObject with StarObject '{}'.",
                          name);

                    // 更新推荐引擎的物品特征
                    recommendationEngine_.addItemFeature(
                        name, "Type", 1.0);  // 示例，具体特征可根据需求添加

                } else {
                    unmatched++;
                    LOG_F(WARNING,
                          "No matching StarObject found for CelestialObject "
                          "'{}'.",
                          name);
                }
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Error associating CelestialObject: {}", e.what());
            }
        }

        LOG_F(INFO, "Loaded CelestialObjects from {}: Matched {}, Unmatched {}",
              filename, matched, unmatched);
        return true;
    }

    // 推荐方法
    std::vector<std::pair<std::string, double>> recommendItems(
        const std::string& user, int topN = 5) {
        LOG_F(INFO, "Requesting top {} recommendations for user '{}'.", topN,
              user);
        try {
            return recommendationEngine_.recommendItems(user, topN);
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in recommendItems for user '{}': {}", user,
                  e.what());
            return {};
        }
    }

    // 保存和加载推荐模型
    bool saveRecommendationModel(const std::string& filename) {
        LOG_F(INFO, "Saving Recommendation Engine model to '{}'.", filename);
        try {
            recommendationEngine_.saveModel(filename);
            LOG_F(INFO,
                  "Successfully saved Recommendation Engine model to '{}'.",
                  filename);
            return true;
        } catch (const std::exception& e) {
            LOG_F(ERROR,
                  "Failed to save Recommendation Engine model to '{}': {}",
                  filename, e.what());
            return false;
        }
    }

    bool loadRecommendationModel(const std::string& filename) {
        LOG_F(INFO, "Loading Recommendation Engine model from '{}'.", filename);
        try {
            recommendationEngine_.loadModel(filename);
            LOG_F(INFO,
                  "Successfully loaded Recommendation Engine model from '{}'.",
                  filename);
            return true;
        } catch (const std::exception& e) {
            LOG_F(ERROR,
                  "Failed to load Recommendation Engine model from '{}': {}",
                  filename, e.what());
            return false;
        }
    }

    // 训练推荐引擎
    void trainRecommendationEngine() {
        LOG_F(INFO, "Starting training of Recommendation Engine.");
        try {
            recommendationEngine_.train();
            LOG_F(INFO, "Recommendation Engine training completed.");
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception during Recommendation Engine training: {}",
                  e.what());
        }
    }

private:
    std::unordered_map<std::string, StarObject>
        starObjectIndex_;  // Key: Star name
    std::unordered_multimap<std::string, std::string>
        aliasIndex_;  // Key: Alias, Value: Star name
    Trie trie_;
    mutable atom::search::ThreadSafeLRUCache<std::string,
                                             std::vector<StarObject>>
        queryCache_;
    mutable std::shared_mutex indexMutex_;

    // 推荐引擎
    AdvancedRecommendationEngine recommendationEngine_;

    // 编辑距离实现
    static int levenshteinDistance(const std::string& str1,
                                   const std::string& str2) {
        LOG_F(INFO, "Calculating Levenshtein distance between '{}' and '{}'.",
              str1, str2);
        const size_t len1 = str1.size();
        const size_t len2 = str2.size();

        // 初始化距离矩阵
        std::vector<std::vector<int>> distanceMatrix(
            len1 + 1, std::vector<int>(len2 + 1));

        for (size_t i = 0; i <= len1; ++i) {
            distanceMatrix[i][0] = static_cast<int>(i);
        }
        for (size_t j = 0; j <= len2; ++j) {
            distanceMatrix[0][j] = static_cast<int>(j);
        }

        // 计算编辑距离
        for (size_t i = 1; i <= len1; ++i) {
            for (size_t j = 1; j <= len2; ++j) {
                if (str1[i - 1] == str2[j - 1]) {
                    distanceMatrix[i][j] = distanceMatrix[i - 1][j - 1];
                } else {
                    distanceMatrix[i][j] = std::min({
                        distanceMatrix[i - 1][j] + 1,     // 删除
                        distanceMatrix[i][j - 1] + 1,     // 插入
                        distanceMatrix[i - 1][j - 1] + 1  // 替换
                    });
                }
            }
        }

        LOG_F(INFO, "Levenshtein distance between '{}' and '{}' is {}", str1,
              str2, distanceMatrix[len1][len2]);
        return distanceMatrix[len1][len2];
    }
};

// --------------------- SearchEngine Implementation ---------------------

SearchEngine::SearchEngine() : pImpl_(std::make_unique<Impl>()) {
    LOG_F(INFO, "SearchEngine instance created.");
}

SearchEngine::~SearchEngine() {
    LOG_F(INFO, "SearchEngine instance destroyed.");
}

bool SearchEngine::initializeRecommendationEngine(
    const std::string& modelFilename) {
    LOG_F(INFO, "Initializing Recommendation Engine with model file '{}'.",
          modelFilename);
    return pImpl_->initializeRecommendationEngine(modelFilename);
}

void SearchEngine::addStarObject(const StarObject& starObject) {
    LOG_F(INFO, "Request to add StarObject: {}", starObject.getName());
    pImpl_->addStarObject(starObject);
}

void SearchEngine::addUserRating(const std::string& user,
                                 const std::string& item, double rating) {
    LOG_F(INFO, "Request to add rating: User '{}', Item '{}', Rating {}", user,
          item, rating);
    pImpl_->addUserRating(user, item, rating);
}

auto SearchEngine::searchStarObject(const std::string& query) const
    -> std::vector<StarObject> {
    LOG_F(INFO, "Request to search StarObject with query: {}", query);
    return pImpl_->searchStarObject(query);
}

auto SearchEngine::fuzzySearchStarObject(
    const std::string& query, int tolerance) const -> std::vector<StarObject> {
    LOG_F(INFO,
          "Request to perform fuzzy search on StarObject with query: '{}' and "
          "tolerance: {}",
          query, tolerance);
    return pImpl_->fuzzySearchStarObject(query, tolerance);
}

auto SearchEngine::autoCompleteStarObject(const std::string& prefix) const
    -> std::vector<std::string> {
    LOG_F(INFO, "Request to auto-complete StarObject with prefix: {}", prefix);
    return pImpl_->autoCompleteStarObject(prefix);
}

std::vector<StarObject> SearchEngine::getRankedResults(
    std::vector<StarObject>& results) {
    LOG_F(INFO, "Request to rank search results.");
    return Impl::getRankedResultsStatic(results);
}

bool SearchEngine::loadFromNameJson(const std::string& filename) {
    LOG_F(INFO, "Request to load StarObjects from name JSON file: {}",
          filename);
    return pImpl_->loadFromNameJson(filename);
}

bool SearchEngine::loadFromCelestialJson(const std::string& filename) {
    LOG_F(INFO, "Request to load CelestialObjects from JSON file: {}",
          filename);
    return pImpl_->loadFromCelestialJson(filename);
}

auto SearchEngine::filterSearch(
    const std::string& type, const std::string& morphology, double minMagnitude,
    double maxMagnitude) const -> std::vector<StarObject> {
    LOG_F(INFO,
          "Request to perform filtered search with type: '{}', morphology: "
          "'{}', magnitude range: {}-{}",
          type, morphology, minMagnitude, maxMagnitude);
    return pImpl_->filterSearch(type, morphology, minMagnitude, maxMagnitude);
}

std::vector<std::pair<std::string, double>> SearchEngine::recommendItems(
    const std::string& user, int topN) const {
    LOG_F(INFO, "Request to recommend top {} items for user '{}'.", topN, user);
    return pImpl_->recommendItems(user, topN);
}

bool SearchEngine::saveRecommendationModel(const std::string& filename) const {
    LOG_F(INFO, "Request to save Recommendation Engine model to '{}'.",
          filename);
    return pImpl_->saveRecommendationModel(filename);
}

bool SearchEngine::loadRecommendationModel(const std::string& filename) {
    LOG_F(INFO, "Request to load Recommendation Engine model from '{}'.",
          filename);
    return pImpl_->loadRecommendationModel(filename);
}

void SearchEngine::trainRecommendationEngine() {
    LOG_F(INFO, "Request to train Recommendation Engine.");
    pImpl_->trainRecommendationEngine();
}

}  // namespace lithium::target