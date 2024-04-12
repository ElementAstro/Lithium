#ifndef CARBON_NLOHMANN_JSON_WRAP_HPP
#define CARBON_NLOHMANN_JSON_WRAP_HPP

#include "carbon/carbon.hpp"

#include "json.hpp"

namespace Carbon {
class json_wrap {
public:
    static Module &library(Module &m) {
        m.add(Carbon::fun(
                  [](const std::string &t_str) { return from_json(t_str); }),
              "from_json");
        m.add(Carbon::fun(&json_wrap::to_json), "to_json");

        return m;
    }

private:
    static Boxed_Value from_json(const nlohmann::json &t_json) {
        switch (t_json.type()) {
            case nlohmann::json::value_t::null:
                return Boxed_Value();
            case nlohmann::json::value_t::object: {
                std::map<std::string, Boxed_Value> m;

                for (const auto &p :
                     t_json.get<std::map<std::string, nlohmann::json>>()) {
                    m.insert(std::make_pair(p.first, from_json(p.second)));
                }

                return Boxed_Value(m);
            }
            case nlohmann::json::value_t::array: {
                std::vector<Boxed_Value> vec;

                for (const auto &p :
                     t_json.get<std::vector<nlohmann::json>>()) {
                    vec.emplace_back(from_json(p));
                }

                return Boxed_Value(vec);
            }
            case nlohmann::json::value_t::string:
                return Boxed_Value(t_json.get<std::string>());
            case nlohmann::json::value_t::number_float:
                return Boxed_Value(t_json.get<float>());
            case nlohmann::json::value_t::number_integer:
                return Boxed_Value(t_json.get<int>());
            case nlohmann::json::value_t::boolean:
                return Boxed_Value(t_json.get<bool>());
        }

        throw std::runtime_error("Unknown JSON type");
    }

    static Boxed_Value from_json(const std::string &t_json) {
        try {
            return from_json(nlohmann::json::parse(t_json));
        } catch (const std::out_of_range &) {
            throw std::runtime_error("Unparsed JSON input");
        }
    }

    static std::string to_json(const Boxed_Value &t_bv) {
        return to_json_object(t_bv).dump();
    }

    static nlohmann::json to_json_object(const Boxed_Value &t_bv) {
        try {
            const std::map<std::string, Boxed_Value> m =
                Carbon::boxed_cast<const std::map<std::string, Boxed_Value> &>(
                    t_bv);

            nlohmann::json obj(nlohmann::json::value_t::object);
            for (const auto &o : m) {
                obj[o.first] = to_json_object(o.second);
            }
            return obj;
        } catch (const Carbon::exception::bad_boxed_cast &) {
            // not a map
        }

        try {
            const std::vector<Boxed_Value> v =
                Carbon::boxed_cast<const std::vector<Boxed_Value> &>(t_bv);

            nlohmann::json obj(nlohmann::json::value_t::array);
            for (size_t i = 0; i < v.size(); ++i) {
                obj[i] = to_json_object(v[i]);
            }
            return obj;
        } catch (const Carbon::exception::bad_boxed_cast &) {
            // not a vector
        }

        try {
            Boxed_Number bn(t_bv);
            if (Boxed_Number::is_floating_point(t_bv)) {
                return nlohmann::json(bn.get_as<double>());
            } else {
                return nlohmann::json(bn.get_as<std::int64_t>());
            }
        } catch (const Carbon::detail::exception::bad_any_cast &) {
            // not a number
        }

        try {
            return nlohmann::json(boxed_cast<bool>(t_bv));
        } catch (const Carbon::exception::bad_boxed_cast &) {
            // not a bool
        }

        try {
            return nlohmann::json(boxed_cast<std::string>(t_bv));
        } catch (const Carbon::exception::bad_boxed_cast &) {
            // not a string
        }

        try {
            const Carbon::dispatch::Dynamic_Object &o =
                boxed_cast<const dispatch::Dynamic_Object &>(t_bv);

            nlohmann::json obj(nlohmann::json::value_t::object);
            for (const auto &attr : o.get_attrs()) {
                obj[attr.first] = to_json_object(attr.second);
            }
            return obj;
        } catch (const Carbon::exception::bad_boxed_cast &) {
            // not a dynamic object
        }

        if (t_bv.is_null())
            return nlohmann::json();  // a null value

        throw std::runtime_error("Unknown object type to convert to JSON");
    }
};

}  // namespace Carbon

#endif
