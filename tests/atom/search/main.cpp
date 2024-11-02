#include "test_cache.hpp"
#include "test_lru.hpp"
#include "test_search.hpp"
#include "test_ttl.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}