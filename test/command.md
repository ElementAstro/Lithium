```
g++ task_test.cpp -std=c++2a -I../libs -o task_test -I../src/core/basic
g++ device_test.cpp -std=c++2a -I../libs -o device_test -I../src/core/basic

g++ -std=c++11 -fPIC -c my_library.cpp -o my_library.o -I../libs -I../src/task
g++ -shared -Wl,-soname,libexample.so -o libexample.so my_library.o
g++ module_test.cpp ../src/thread/thread.cpp ../src/module/modloader.cpp -std=c++2a -I../libs -o module_test -lfmt -ldl

g++ wsserver_test.cpp ../src/server/commander.cpp ../src/server/wsserver.cpp -std=c++2a -I../libs -o wsserver_test -I../src/ -lfmt -I../src/core/ -I../src/server  -lboost_system-mt -lboost_thread-mt  -lwsock32 -lws2_32 -D_WEBSOCKETPP_CPP11_THREAD_=1 -I/usr/include/boost

g++ http_server_test.cpp ../src/server/hserver.cpp -lssl -std=c++2a -I../libs -o http_server_test -I../src/core/basic -lfmt -I../src/core/ -I../src/server  -lboost_system-mt -lboost_thread-mt  -lwsock32 -lws2_32 -I/usr/include/boost -I../libs/crow -lcrypto

g++ launcher.cpp -lssl -std=c++2a -I../../libs -o launcher -I../core/basic -lfmt -I../core/  -lboost_system-mt -lboost_thread-mt  -lwsock32 -lws2_32 -I/usr/include/boost -lcrypto

g++ httptest.cpp ../src/network/httpclient.cpp -lssl -std=c++2a -I../libs -o httptest -lfmt -I../src/network  -lboost_system-mt -lboost_thread-mt  -lwsock32 -lws2_32 -I/usr/include/boost -lcrypto
-lwinhttp

g++ script_test.cpp ../src/module/sheller.cpp -lssl -std=c++2a -I../libs -o script_test -lfmt -I../src/module 

g++ message_test.cpp -std=c++2a -o message_test

g++ -std=c++20 -fPIC -c camera_lib_test.cpp -o camera_lib_test.o -I../libs -I../src/modules/device -I ../src
g++ -shared -Wl,-soname,libmycamera.so -o libmycamera.so ../src/modules/device/device.cpp ../src/modules/property/uuid.cpp camera_lib_test.o -I../libs -I../src

```