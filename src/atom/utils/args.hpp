#pragma once

#include <string>
#include <functional>
#include <optional>
#include <vector>

namespace Atom::Utils
{
    class Parser
    {
        struct option
        {
            std::string_view sname_;
            std::string_view lname_;
            std::string_view description_;
            bool necessary_;
            std::optional<std::string> default_;
            std::function<void(const Parser &, const std::string &)> handle_;
        };

        using options = std::vector<option>;

        options necessary_, optional_;
        std::function<void(const Parser &, const std::string &)> usage_;
        std::string path_;

    public:
        options &necessary(void) { return necessary_; }
        const options &necessary(void) const { return necessary_; }

        options &optional(void) { return optional_; }
        const options &optional(void) const { return optional_; }

        std::function<void(const Parser &, const std::string &)> &usage(void) { return usage_; }
        const std::function<void(const Parser &, const std::string &)> &usage(void) const { return usage_; }

        template <typename T>
        void print_usage(T &&out) const;

        void print_usage(void) const;

        void push(options &&opts);

        void clear(void);

        template <typename T>
        void exec(T &&out, int argc, const char *const argv[]);

        void exec(int argc, const char *const argv[]);
    };

    template <typename T>
    void Parser::exec(T &&out, int argc, const char *const argv[])
    {
        if (argc >= 1)
            path_ = argv[0];
        if (argc <= 1)
            this->print_usage(std::forward<T>(out));
        else
        {
            struct ST_opt
            {
                std::function<void(const Parser &, const std::string &)> hd_;
                std::string cm_;
            };
            std::vector<ST_opt> exec_list;
            size_t c_nec = 0;
            for (int i = 1; i < argc; ++i)
            {
                std::string_view a = argv[i];
                size_t c = a.find_first_of('=');
                std::string_view o = a.substr(0, c);
                auto foreach = [&](const auto &cc, auto fr)
                {
                    for (auto it = cc.begin(); it != cc.end(); ++it)
                    {
                        if (o == it->sname_ || o == it->lname_)
                        {
                            exec_list.push_back(ST_opt{
                                it->handle_,
                                c == std::string::npos ? it->default_.value_or("") : a.substr(c + 1)});
                            fr();
                        }
                    }
                };
                foreach (necessary_, [&c_nec]
                         { ++c_nec; })
                    ;
                foreach (optional_, [] {})
                    ;
            }
            if (c_nec != necessary_.size() || exec_list.empty())
                this->print_usage(std::forward<T>(out));
            else
            {
                for (auto &e : exec_list)
                {
                    std::invoke(e.hd_, *this, e.cm_);
                }
            }
        }
    }

    template <typename T>
    void Parser::print_usage(T &&out) const
    {
        auto flw = capo::output(std::forward<T>(out), "");
        if (path_.empty())
        {
            flw("Must has at least one argument (the path of current program).").ln();
            return;
        }
        size_t slash = path_.find_last_of('\\');
        if (slash == std::string::npos)
            slash = path_.find_last_of('/');
        std::string name = path_.substr(slash + 1);
        if (usage_)
        {
            usage_(*this, name);
        }
        else
        {
            flw("Usage: {} ", name);
            if (!necessary_.empty())
            {
                for (auto &o : necessary_)
                {
                    flw(o.lname_);
                    if (o.default_.has_value())
                        flw("={}", o.default_.value());
                    flw(" ");
                }
            }
            flw("[OPTIONS]...").ln()("Options: ").ln();
            auto print_opt = [&](const auto &o)
            {
                flw("  {}, {}\t{}", o.sname_, o.lname_, o.description_);
                if (o.default_.has_value())
                    flw("[={}]", o.default_.value());
                flw.ln();
            };
            for (auto &o : necessary_)
            {
                print_opt(o);
            }
            for (auto &o : optional_)
            {
                print_opt(o);
            }
        }
    }
}