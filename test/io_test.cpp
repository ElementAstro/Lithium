#include "../src/io/io.hpp"

using namespace Lithium::File;

int main()
{
    create_directory("aaa");
    copy_file("clients.json","aaa.json");
    return 0;
}