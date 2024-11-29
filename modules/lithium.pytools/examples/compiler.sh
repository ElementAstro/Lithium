# Example 1: Compile source files using GCC with C++17 standard
# This example compiles source1.cpp and source2.cpp using the GCC compiler with the C++17 standard.
# The output object file will be saved as output.o.
$ python3 compiler.py source1.cpp source2.cpp -o output.o --compiler GCC --cpp_version c++17

# Example 2: Compile source files using Clang with additional compilation flags
# This example compiles source1.cpp using the Clang compiler with the C++20 standard and additional compilation flags.
# The output object file will be saved as output.o.
$ python3 compiler.py source1.cpp -o output.o --compiler Clang --cpp_version c++20 --compile-flags -O3 -Wall

# Example 3: Link object files into an executable using MSVC
# This example links object1.o and object2.o into an executable using the MSVC compiler.
# The output executable will be saved as output.exe.
$ python3 compiler.py object1.o object2.o -o output.exe --compiler MSVC --link

# Example 4: Compile and link source files using Intel C++ Compiler with additional flags
# This example compiles and links source1.cpp using the Intel C++ Compiler with the C++23 standard and additional flags.
# The output executable will be saved as output.
$ python3 compiler.py source1.cpp -o output --compiler "Intel C++ Compiler" --cpp_version c++23 --link --flags -O2 -g

# Example 5: Compile source files using GCC with options loaded from a JSON file
# This example compiles source1.cpp using the GCC compiler with options loaded from options.json.
# The output object file will be saved as output.o.
$ python3 compiler.py source1.cpp -o output.o --compiler GCC --cpp_version c++17 --json-options options.json

# Example 6: Display system and compiler helper information
# This example displays system and compiler helper information.
$ python3 compiler.py --show-info

# Example 7: Compile source files using the default detected compiler and C++ version
# This example compiles source1.cpp using the default detected compiler and C++ version.
# The output object file will be saved as output.o.
$ python3 compiler.py source1.cpp -o output.o

# Example 8: Compile source files using Clang with additional compilation and linking flags
# This example compiles source1.cpp using the Clang compiler with additional compilation and linking flags.
# The output object file will be saved as output.o.
$ python3 compiler.py source1.cpp -o output.o --compiler Clang --cpp_version c++20 --compile-flags -O3 -Wall --link-flags -lpthread

# Example 9: Compile source files using MSVC with C++20 standard and display detailed logging
# This example compiles source1.cpp using the MSVC compiler with the C++20 standard and displays detailed logging.
# The output object file will be saved as output.obj.
$ python3 compiler.py source1.cpp -o output.obj --compiler MSVC --cpp_version c++20 --flags /EHsc /MD

# Example 10: Compile and link source files using GCC with C++17 standard and additional flags
# This example compiles and links source1.cpp and source2.cpp using the GCC compiler with the C++17 standard and additional flags.
# The output executable will be saved as output.
$ python3 compiler.py source1.cpp source2.cpp -o output --compiler GCC --cpp_version c++17 --link --flags -O2 -g