# Example 1: Compile and run a C++ program with default settings
# This example compiles and runs the C++ source file main.cpp with default settings.
# The output executable will be named a.out, and core dumps will be searched in /tmp.
$ python3 core_runner.py main.cpp

# Example 2: Compile and run a C++ program with a custom output executable name
# This example compiles and runs the C++ source file main.cpp, and the output executable will be named my_program.
$ python3 core_runner.py main.cpp -o my_program

# Example 3: Compile and run a C++ program with a custom core dump directory
# This example compiles and runs the C++ source file main.cpp, and core dumps will be searched in the specified directory /var/cores.
$ python3 core_runner.py main.cpp -d /var/cores

# Example 4: Compile and run a C++ program with a custom core pattern
# This example compiles and runs the C++ source file main.cpp, and the core pattern for dump files will be set to /tmp/core.%e.%p.
$ python3 core_runner.py main.cpp -p /tmp/core.%e.%p

# Example 5: Compile and run a C++ program with unlimited core dump size
# This example compiles and runs the C++ source file main.cpp, and sets the core dump size to unlimited.
$ python3 core_runner.py main.cpp -u

# Example 6: Compile and run a C++ program with additional compilation flags
# This example compiles and runs the C++ source file main.cpp with additional compilation flags -O2 and -Wall.
$ python3 core_runner.py main.cpp -f -O2 -Wall

# Example 7: Compile and run a C++ program with a specific C++ standard
# This example compiles and runs the C++ source file main.cpp using the C++20 standard.
$ python3 core_runner.py main.cpp -s c++20

# Example 8: Compile and run a C++ program with custom GDB commands for core dump analysis
# This example compiles and runs the C++ source file main.cpp, and uses custom GDB commands for core dump analysis.
$ python3 core_runner.py main.cpp -g -ex "info locals" -ex "bt" -ex "quit"

# Example 9: Compile and run a C++ program with automatic core dump analysis
# This example compiles and runs the C++ source file main.cpp, and automatically analyzes the core dump if the program crashes.
$ python3 core_runner.py main.cpp -a

# Example 10: Compile and run a C++ program with logging to a specified log file
# This example compiles and runs the C++ source file main.cpp, and writes logs to the specified log file core_runner.log.
$ python3 core_runner.py main.cpp -l core_runner.log