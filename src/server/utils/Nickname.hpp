#ifndef Nickname_hpp
#define Nickname_hpp

#include "oatpp/Types.hpp"

#include <random>
#include <vector>

class Nickname {
public:
    static constexpr int AVATARS_SIZE = 70;
    static const char* const AVATARS[];

    static constexpr int ADJECTIVES_SIZE = 103;
    static const char* const ADJECTIVES[];

    static constexpr int NOUNS_SIZE = 49;
    static const char* NOUNS[];

private:
    static thread_local std::mt19937 RANDOM_GENERATOR;

public:
    static oatpp::String random();
};

#endif  // Nickname_hpp
