#include "test_memory.hpp"
#include "test_object.hpp"
#include "test_ring.hpp"
#include "test_shared.hpp"
#include "test_short_alloc.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}