#include <iostream>
#include <vector>
#include <functional>
#include <optional>

namespace Atom::Utils
{
    void Parser::print_usage(void) const
    {
        this->print_usage(std::cout);
    }

    void Parser::push(options &&opts)
    {
        for (auto &&o : opts)
        {
            options *list = &(o.necessary_ ? necessary_ : optional_);
            list->push_back(std::move(o));
        }
    }

    void Parser::clear(void)
    {
        necessary_.clear();
        optional_.clear();
    }

    void Parser::exec(int argc, const char *const argv[])
    {
        this->exec(std::cout, argc, argv);
    }
}
