#pragma once

#include <string>
#include <vector>
#include <tuple>
#if _cplusplus >= 201703L
#include <type_traits>
#endif

/**
 * @class DsoObject
 * @brief 表示一个天体对象 (Distributed Star Object)
 */
class DsoObject
{
public:
    /**
     * @brief 构造函数
     * @param name 天体对象的名称
     */
    DsoObject(const std::string &name);

    /**
     * @brief 将天体对象转换为字符串表示形式
     * @return 天体对象的字符串表示形式
     */
    std::string toString() const;

    /**
     * @brief 获取天体对象所属的星座
     * @return 星座的名称
     */
    std::string getConstellation() const;

    /**
     * @brief 获取天体对象的坐标
     * @tparam T 坐标的类型 , std::vector<std::string> or std::vector<double>
     * @return 天体对象的坐标
     */
    template<typename T>
#if _cplusplus >= 202002L
    requires std::is_arithmetic_v<T>
#else
    std::enable_if_t<std::is_arithmetic_v<T>, std::vector<T>> getCoords() const;
#endif

    /**
     * @brief 获取天体对象的赤经
     * @return 赤经的字符串表示形式
     */
    std::string getRa() const;

    /**
     * @brief 获取天体对象的赤纬
     * @return 赤纬的字符串表示形式
     */
    std::string getDec() const;

    /**
     * @brief 获取天体对象的尺寸
     * @return 包含主轴长度、次轴长度和位置角的尺寸数组
     */
    std::vector<double> getDimensions() const;

    /**
     * @brief 获取天体对象的哈勃分类
     * @return 哈勃分类的字符串表示形式
     */
    std::string getHubble() const;

    /**
     * @brief 获取天体对象的ID
     * @return 天体对象的ID
     */
    int getId() const;

    /**
     * @brief 获取天体对象的所有标识符
     * @return 返回一个包含多个标识符的元组，依次是目录、Messier编号、NGC编号、IC编号和常用名称
     */
    std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> getIdentifiers() const;

private:
    int m_id;                    /**< 天体对象的ID */
    std::string m_name;          /**< 天体对象的名称 */
    std::string m_type;          /**< 天体对象的类型 */
    double m_ra;                 /**< 天体对象的赤经 */
    double m_dec;                /**< 天体对象的赤纬 */
    std::string m_const;         /**< 天体对象所属的星座 */
    std::string m_notngc;        /**< 是否不属于NGC目录 */
    double m_majax;              /**< 天体对象的主轴长度 */
    double m_minax;              /**< 天体对象的次轴长度 */
    int m_pa;                    /**< 天体对象的位置角 */
    double m_bmag;               /**< 天体对象的B星等 */
    double m_vmag;               /**< 天体对象的V星等 */
    double m_jmag;               /**< 天体对象的J星等 */
    double m_hmag;               /**< 天体对象的H星等 */
    double m_kmag;               /**< 天体对象的K星等 */
    double m_sbrightn;           /**< 天体对象的表面亮度 */
    std::string m_hubble;        /**< 天体对象的哈勃分类 */
    double m_parallax;           /**< 天体对象的视差 */
    double m_pmra;               /**< 天体对象的赤经年周运动 */
    double m_pmdec;              /**< 天体对象的赤纬年周运动 */
    double m_radvel;             /**< 天体对象的径向速度 */
    double m_redshift;           /**< 天体对象的红移 */
    double m_cstarumag;          /**< 中心恒星的U星等 */
    double m_cstarbmag;          /**< 中心恒星的B星等 */
    double m_cstarvmag;          /**< 中心恒星的V星等 */
    std::string m_messier;       /**< 天体对象的Messier编号 */
    std::string m_ngc;           /**< 天体对象的NGC编号 */
    std::string m_ic;            /**< 天体对象的IC编号 */
    std::string m_cstarnames;    /**< 中心恒星的名称 */
    std::string m_identifiers;   /**< 天体对象的标识符 */
    std::string m_commonnames;   /**< 天体对象的常用名称 */
    std::string m_nednotes;      /**< 天体对象的NED注释 */
    std::string m_ongcnotes;     /**< 天体对象的OpenNGC注释 */
};
