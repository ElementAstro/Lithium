#ifndef MUSTACHE_TEMPLATE
#define MUSTACHE_TEMPLATE

#include <variant>
#include "nlohmann/json.hpp"

namespace html
{
    struct context : std::unordered_map<std::string, bustache::format>
    {
        using unordered_map::unordered_map;

        bustache::format const *operator()(std::string const &key) const
        {
            auto it = find(key);
            return it == end() ? nullptr : &it->second;
        }
    };

    struct value;

    struct object : std::vector<std::pair<std::string, value>>
    {
        using vector::vector;
        using mapped_type = value;

        const_iterator find(std::string const &key) const;
    };

    using array = std::vector<value>;
    using lazy_value = std::function<value(bustache::ast::view const *)>;
    using lazy_format = std::function<bustache::format(bustache::ast::view const *)>;

    struct value : std::variant<bool, int, double, std::string, object, array, lazy_value, lazy_format>
    {
        using variant::variant;

        value(char const *str) : variant(std::string(str)) {}
    };

    object::const_iterator object::find(std::string const &key) const
    {
        return std::find_if(begin(), end(), [&key](value_type const &pair)
                            { return pair.first == key; });
    }
}

template <>
struct bustache::impl_compatible<html::value>
{
    static value_ptr get_value_ptr(html::value::variant const &self)
    {
        return std::visit([](auto const &val)
                          { return value_ptr(&val); },
                          self);
    }
};

template <>
struct bustache::impl_model<nlohmann::json>
{
    impl_model() = delete;
};

template <>
struct bustache::impl_compatible<nlohmann::json>
{
    static value_ptr get_value_ptr(nlohmann::json const &self)
    {
        nlohmann::json::value_t const kind(self);
        switch (kind)
        {
        case nlohmann::json::value_t::boolean:
            return value_ptr(self.get_ptr<nlohmann::json::boolean_t const *>());
        case nlohmann::json::value_t::number_integer:
            return value_ptr(self.get_ptr<nlohmann::json::number_integer_t const *>());
        case nlohmann::json::value_t::number_unsigned:
            return value_ptr(self.get_ptr<nlohmann::json::number_unsigned_t const *>());
        case nlohmann::json::value_t::number_float:
            return value_ptr(self.get_ptr<nlohmann::json::number_float_t const *>());
        case nlohmann::json::value_t::string:
            return value_ptr(self.get_ptr<nlohmann::json::string_t const *>());
        case nlohmann::json::value_t::array:
            return value_ptr(self.get_ptr<nlohmann::json::array_t const *>());
        case nlohmann::json::value_t::object:
            return value_ptr(self.get_ptr<nlohmann::json::object_t const *>());
        }
        return value_ptr();
    }
};


#endif