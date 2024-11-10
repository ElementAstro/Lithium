#ifndef STAR_SEARCH_SEARCH_HPP
#define STAR_SEARCH_SEARCH_HPP

#include <algorithm>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "atom/macro.hpp"
#include "atom/type/json_fwd.hpp"

// 包含偏好引擎的头文件
#include "preference.hpp"

namespace lithium::target {

class CelestialObject {
public:
    // 构造函数
    CelestialObject(std::string id, std::string identifier,
                    std::string mIdentifier, std::string extensionName,
                    std::string component, std::string className,
                    std::string amateurRank, std::string chineseName,
                    std::string type, std::string duplicateType,
                    std::string morphology, std::string constellationZh,
                    std::string constellationEn, std::string raJ2000,
                    double raDJ2000, std::string decJ2000, double decDJ2000,
                    double visualMagnitudeV, double photographicMagnitudeB,
                    double bMinusV, double surfaceBrightness, double majorAxis,
                    double minorAxis, int positionAngle,
                    std::string detailedDescription,
                    std::string briefDescription)
        : ID(std::move(id)),
          Identifier(std::move(identifier)),
          MIdentifier(std::move(mIdentifier)),
          ExtensionName(std::move(extensionName)),
          Component(std::move(component)),
          ClassName(std::move(className)),
          AmateurRank(std::move(amateurRank)),
          ChineseName(std::move(chineseName)),
          Type(std::move(type)),
          DuplicateType(std::move(duplicateType)),
          Morphology(std::move(morphology)),
          ConstellationZh(std::move(constellationZh)),
          ConstellationEn(std::move(constellationEn)),
          RAJ2000(std::move(raJ2000)),
          RADJ2000(raDJ2000),
          DecJ2000(std::move(decJ2000)),
          DecDJ2000(decDJ2000),
          VisualMagnitudeV(visualMagnitudeV),
          PhotographicMagnitudeB(photographicMagnitudeB),
          BMinusV(bMinusV),
          SurfaceBrightness(surfaceBrightness),
          MajorAxis(majorAxis),
          MinorAxis(minorAxis),
          PositionAngle(positionAngle),
          DetailedDescription(std::move(detailedDescription)),
          BriefDescription(std::move(briefDescription)) {}

    // 默认构造函数
    CelestialObject() = default;

    // JSON反序列化函数
    static auto from_json(const nlohmann::json& j) -> CelestialObject;

    // JSON序列化函数
    [[nodiscard]] auto to_json() const -> nlohmann::json;

    // 获取名称（假设 Identifier 是名称）
    [[nodiscard]] const std::string& getName() const { return Identifier; }

    // 数据成员
    std::string ID;
    std::string Identifier;
    std::string MIdentifier;
    std::string ExtensionName;
    std::string Component;
    std::string ClassName;
    std::string AmateurRank;
    std::string ChineseName;
    std::string Type;
    std::string DuplicateType;
    std::string Morphology;
    std::string ConstellationZh;
    std::string ConstellationEn;
    std::string RAJ2000;
    double RADJ2000;
    std::string DecJ2000;
    double DecDJ2000;
    double VisualMagnitudeV;
    double PhotographicMagnitudeB;
    double BMinusV;
    double SurfaceBrightness;
    double MajorAxis;
    double MinorAxis;
    int PositionAngle;
    std::string DetailedDescription;
    std::string BriefDescription;
};

class StarObject {
private:
    std::string name_;
    std::vector<std::string> aliases_;
    int clickCount_;
    CelestialObject celestialObject_;

public:
    StarObject(std::string name, std::vector<std::string> aliases,
               int clickCount = 0);

    // 访问器方法
    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] const std::vector<std::string>& getAliases() const;
    [[nodiscard]] int getClickCount() const;

    // 修改器方法
    void setName(const std::string& name);
    void setAliases(const std::vector<std::string>& aliases);
    void setClickCount(int clickCount);

    // JSON序列化函数
    void setCelestialObject(const CelestialObject& celestialObject);
    [[nodiscard]] CelestialObject getCelestialObject() const;
    [[nodiscard]] nlohmann::json to_json() const;
} ATOM_ALIGNAS(128);

/**
 * @brief Trie（前缀树）用于存储和搜索字符串
 *
 * Trie 用于高效地存储和检索字符串，特别适用于自动完成等任务
 */
class Trie {
    struct TrieNode {
        std::unordered_map<char, TrieNode*> children;  ///< 子节点
        bool isEndOfWord = false;  ///< 标记是否为单词结尾
    } ATOM_ALIGNAS(64);

public:
    /**
     * @brief 构造一个空的 Trie
     */
    Trie();

    /**
     * @brief 析构函数，释放内存
     */
    ~Trie();

    // 禁用拷贝构造和赋值运算符
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    // 默认移动构造和赋值运算符
    Trie(Trie&&) noexcept = default;
    Trie& operator=(Trie&&) noexcept = default;

    /**
     * @brief 向 Trie 中插入一个单词
     *
     * @param word 要插入的单词
     */
    void insert(const std::string& word);

    /**
     * @brief 根据给定的前缀提供自动完成建议
     *
     * @param prefix 要搜索的前缀
     * @return std::vector<std::string> 自动完成建议的向量
     */
    [[nodiscard]] auto autoComplete(const std::string& prefix) const
        -> std::vector<std::string>;

private:
    /**
     * @brief 深度优先搜索，收集以给定前缀开始的所有单词
     *
     * @param node 当前访问的 TrieNode
     * @param prefix 当前形成的前缀
     * @param suggestions 用于收集建议的向量
     */
    void dfs(TrieNode* node, const std::string& prefix,
             std::vector<std::string>& suggestions) const;

    /**
     * @brief 递归释放 Trie 节点的内存
     *
     * @param node 当前要释放的 TrieNode
     */
    void clear(TrieNode* node);

    TrieNode* root_;  ///< Trie 的根节点
};

/**
 * @brief 用于星体对象的搜索引擎
 */
class SearchEngine {
public:
    SearchEngine();
    ~SearchEngine();

    void addStarObject(const StarObject& starObject);

    [[nodiscard]] auto searchStarObject(const std::string& query) const
        -> std::vector<StarObject>;

    [[nodiscard]] auto fuzzySearchStarObject(const std::string& query,
                                             int tolerance) const
        -> std::vector<StarObject>;

    [[nodiscard]] auto autoCompleteStarObject(const std::string& prefix) const
        -> std::vector<std::string>;

    [[nodiscard]] static auto getRankedResults(std::vector<StarObject>& results)
        -> std::vector<StarObject>;

    // 从 JSON 文件加载数据
    bool loadFromNameJson(const std::string& filename);
    bool loadFromCelestialJson(const std::string& filename);

    // 高级搜索 - 过滤条件
    [[nodiscard]] auto filterSearch(
        const std::string& type = "", const std::string& morphology = "",
        double minMagnitude = -std::numeric_limits<double>::infinity(),
        double maxMagnitude = std::numeric_limits<double>::infinity()) const
        -> std::vector<StarObject>;

    // 新增：偏好引擎相关方法
    bool initializeRecommendationEngine(const std::string& modelFilename);
    void addUserRating(const std::string& user, const std::string& item,
                       double rating);
    std::vector<std::pair<std::string, double>> recommendItems(
        const std::string& user, int topN = 5) const;
    bool saveRecommendationModel(const std::string& filename) const;
    bool loadRecommendationModel(const std::string& filename);
    void trainRecommendationEngine();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

}  // namespace lithium::target

#endif  // STAR_SEARCH_SEARCH_HPP