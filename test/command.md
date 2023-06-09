```
g++ task_test.cpp -std=c++2a -I../libs -o task_test -I../src/core/basic
g++ device_test.cpp -std=c++2a -I../libs -o device_test -I../src/core/basic

g++ -std=c++11 -fPIC -c my_library.cpp -o my_library.o -I../libs -I../src/core/basic
g++ -shared -Wl,-soname,libexample.so -o libexample.so my_library.o
g++ module_test.cpp ../src/core/thread.cpp ../src/core/modloader.cpp -std=c++2a -I../libs -o module_test -I../src/core/basic -lfmt -I../src/core/ -ldl

```