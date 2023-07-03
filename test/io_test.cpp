#include "../src/io/io.hpp"

using namespace OpenAPT::File;

int main()
{
    create_directory("aaa");
    copy_file("clients.json","aaa.json");
    return 0;
}