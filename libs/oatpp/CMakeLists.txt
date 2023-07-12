cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/core/base/Environment.hpp" OATPP_VERSION_MACRO REGEX "#define OATPP_VERSION \"[0-9]+.[0-9]+.[0-9]+\"$")
string(REGEX REPLACE "#define OATPP_VERSION \"([0-9]+.[0-9]+.[0-9]+)\"$" "\\1" oatpp_VERSION "${OATPP_VERSION_MACRO}")

###################################################################################################
## These variables are passed to oatpp-module-install.cmake script
## use these variables to configure module installation

set(OATPP_THIS_MODULE_NAME oatpp) ## name of the module (also name of folders in installation dirs)
set(OATPP_THIS_MODULE_VERSION ${oatpp_VERSION}) ## version of the module (also sufix of folders in installation dirs)

###################################################################################################

project(oatpp VERSION ${OATPP_THIS_MODULE_VERSION} LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(OATPP_INSTALL "Create installation target for oat++" ON)
option(OATPP_BUILD_TESTS "Create test target for oat++" ON)
option(OATPP_LINK_TEST_LIBRARY "Link oat++ test library" ON)
option(OATPP_LINK_ATOMIC "Link atomic library for other platform than MSVC|MINGW|APPLE|FreeBSD" ON)
option(OATPP_MSVC_LINK_STATIC_RUNTIME "MSVC: Link with static runtime (/MT and /MTd)." OFF)

###################################################################################################
## COMPILATION CONFIG #############################################################################
###################################################################################################

if(OATPP_LINK_TEST_LIBRARY)
	set(OATPP_THIS_MODULE_LIBRARIES oatpp oatpp-test) ## list of libraries to find when find_package is called
	set(OATPP_THIS_MODULE_TARGETS oatpp oatpp-test) ## list of targets to install
	set(OATPP_THIS_MODULE_DIRECTORIES oatpp oatpp-test) ## list of directories to install
else()
	set(OATPP_THIS_MODULE_LIBRARIES oatpp) ## list of libraries to find when find_package is called
	set(OATPP_THIS_MODULE_TARGETS oatpp) ## list of targets to install
	set(OATPP_THIS_MODULE_DIRECTORIES oatpp) ## list of directories to install
endif()

option(OATPP_DISABLE_ENV_OBJECT_COUNTERS "Disable object counting for Release builds for better performance" OFF)
option(OATPP_DISABLE_POOL_ALLOCATIONS "This will make oatpp::base::memory::MemoryPool, method obtain and free call new and delete directly" OFF)

set(OATPP_THREAD_HARDWARE_CONCURRENCY "AUTO" CACHE STRING "Predefined value for function oatpp::concurrency::Thread::getHardwareConcurrency()")

option(OATPP_COMPAT_BUILD_NO_THREAD_LOCAL "Disable 'thread_local' feature" OFF)
option(OATPP_COMPAT_BUILD_NO_SET_AFFINITY "No 'pthread_setaffinity_np' method" OFF)

option(OATPP_DISABLE_LOGV "DISABLE logs priority V" OFF)
option(OATPP_DISABLE_LOGD "DISABLE logs priority D" OFF)
option(OATPP_DISABLE_LOGI "DISABLE logs priority I" OFF)
option(OATPP_DISABLE_LOGW "DISABLE logs priority W" OFF)
option(OATPP_DISABLE_LOGE "DISABLE logs priority E" OFF)

## Print config ##################################################################################

message("############################################################################")
message("## oatpp module compilation config:")

message("OATPP_DISABLE_ENV_OBJECT_COUNTERS=${OATPP_DISABLE_ENV_OBJECT_COUNTERS}")
message("OATPP_THREAD_HARDWARE_CONCURRENCY=${OATPP_THREAD_HARDWARE_CONCURRENCY}")
message("OATPP_COMPAT_BUILD_NO_THREAD_LOCAL=${OATPP_COMPAT_BUILD_NO_THREAD_LOCAL}")

## Set definitions ###############################################################################

if(OATPP_DISABLE_ENV_OBJECT_COUNTERS)
    add_definitions(-DOATPP_DISABLE_ENV_OBJECT_COUNTERS)
endif()

if(OATPP_DISABLE_POOL_ALLOCATIONS)
    add_definitions (-DOATPP_DISABLE_POOL_ALLOCATIONS)
    message("WARNING: OATPP_DISABLE_POOL_ALLOCATIONS option is deprecated and has no effect.")
endif()

set(AUTO_VALUE AUTO)
if(NOT OATPP_THREAD_HARDWARE_CONCURRENCY STREQUAL AUTO_VALUE)
    add_definitions (-DOATPP_THREAD_HARDWARE_CONCURRENCY=${OATPP_THREAD_HARDWARE_CONCURRENCY})
endif()

if(OATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT)
    add_definitions (-DOATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT=${OATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT})
    message("WARNING: OATPP_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT option is deprecated and has no effect.")
endif()

if(OATPP_COMPAT_BUILD_NO_THREAD_LOCAL)
    add_definitions(-DOATPP_COMPAT_BUILD_NO_THREAD_LOCAL)
endif()

if(OATPP_COMPAT_BUILD_NO_SET_AFFINITY)
    add_definitions(-DOATPP_COMPAT_BUILD_NO_SET_AFFINITY)
endif()

if(OATPP_DISABLE_LOGV)
    add_definitions(-DOATPP_DISABLE_LOGV)
endif()

if(OATPP_DISABLE_LOGD)
    add_definitions(-DOATPP_DISABLE_LOGD)
endif()

if(OATPP_DISABLE_LOGI)
    add_definitions(-DOATPP_DISABLE_LOGI)
endif()

if(OATPP_DISABLE_LOGW)
    add_definitions(-DOATPP_DISABLE_LOGW)
endif()

if(OATPP_DISABLE_LOGE)
    add_definitions(-DOATPP_DISABLE_LOGE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 5.0)
    add_definitions(-DOATPP_DISABLE_STD_PUT_TIME)
endif()

message("############################################################################")

###################################################################################################

message("oatpp version: '${OATPP_THIS_MODULE_VERSION}'")

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

#######################################################################################################
## oatpp

add_library(oatpp
        algorithm/CRC.cpp
        algorithm/CRC.hpp
        codegen/ApiClient_define.hpp
        codegen/ApiClient_undef.hpp
        codegen/ApiController_define.hpp
        codegen/ApiController_undef.hpp
        codegen/DbClient_define.hpp
        codegen/DbClient_undef.hpp
        codegen/DTO_define.hpp
        codegen/DTO_undef.hpp
        codegen/api_controller/auth_define.hpp
        codegen/api_controller/auth_undef.hpp
        codegen/api_controller/base_define.hpp
        codegen/api_controller/base_undef.hpp
        codegen/api_controller/bundle_define.hpp
        codegen/api_controller/bundle_undef.hpp
        codegen/api_controller/cors_define.hpp
        codegen/api_controller/cors_undef.hpp
        codegen/dto/base_define.hpp
        codegen/dto/base_undef.hpp
        codegen/dto/enum_define.hpp
        codegen/dto/enum_undef.hpp
        core/IODefinitions.cpp
        core/IODefinitions.hpp
        core/Types.hpp
        core/async/ConditionVariable.cpp
        core/async/ConditionVariable.hpp
        core/async/Coroutine.cpp
        core/async/Coroutine.hpp
        core/async/CoroutineWaitList.cpp
        core/async/CoroutineWaitList.hpp
        core/async/Error.cpp
        core/async/Error.hpp
        core/async/Executor.cpp
        core/async/Executor.hpp
        core/async/Lock.cpp
        core/async/Lock.hpp
        core/async/Processor.cpp
        core/async/Processor.hpp
        core/async/utils/FastQueue.hpp
        core/async/worker/IOEventWorker_common.cpp
        core/async/worker/IOEventWorker_epoll.cpp
        core/async/worker/IOEventWorker_kqueue.cpp
        core/async/worker/IOEventWorker_stub.cpp
        core/async/worker/IOEventWorker.hpp
        core/async/worker/IOWorker.cpp
        core/async/worker/IOWorker.hpp
        core/async/worker/TimerWorker.cpp
        core/async/worker/TimerWorker.hpp
        core/async/worker/Worker.cpp
        core/async/worker/Worker.hpp
        core/base/CommandLineArguments.cpp
        core/base/CommandLineArguments.hpp
        core/base/Config.hpp
        core/base/Countable.cpp
        core/base/Countable.hpp
        core/base/Environment.cpp
        core/base/Environment.hpp
        core/base/ObjectHandle.hpp
        core/concurrency/SpinLock.cpp
        core/concurrency/SpinLock.hpp
        core/concurrency/Thread.cpp
        core/concurrency/Thread.hpp
        core/data/Bundle.cpp
        core/data/Bundle.hpp
        core/data/buffer/FIFOBuffer.cpp
        core/data/buffer/FIFOBuffer.hpp
        core/data/buffer/IOBuffer.cpp
        core/data/buffer/IOBuffer.hpp
        core/data/buffer/Processor.cpp
        core/data/buffer/Processor.hpp
        core/data/mapping/ObjectMapper.cpp
        core/data/mapping/ObjectMapper.hpp
        core/data/mapping/TypeResolver.cpp
        core/data/mapping/TypeResolver.hpp
        core/data/mapping/type/Any.cpp
        core/data/mapping/type/Any.hpp
        core/data/mapping/type/Collection.hpp
        core/data/mapping/type/Enum.cpp
        core/data/mapping/type/Enum.hpp
        core/data/mapping/type/List.cpp
        core/data/mapping/type/List.hpp
        core/data/mapping/type/Map.hpp
        core/data/mapping/type/Object.cpp
        core/data/mapping/type/Object.hpp
        core/data/mapping/type/PairList.cpp
        core/data/mapping/type/PairList.hpp
        core/data/mapping/type/Primitive.cpp
        core/data/mapping/type/Primitive.hpp
        core/data/mapping/type/Type.cpp
        core/data/mapping/type/Type.hpp
        core/data/mapping/type/UnorderedMap.cpp
        core/data/mapping/type/UnorderedMap.hpp
        core/data/mapping/type/UnorderedSet.cpp
        core/data/mapping/type/UnorderedSet.hpp
        core/data/mapping/type/Vector.cpp
        core/data/mapping/type/Vector.hpp
        core/data/resource/File.cpp
        core/data/resource/File.hpp
        core/data/resource/InMemoryData.cpp
        core/data/resource/InMemoryData.hpp
        core/data/resource/Resource.hpp
        core/data/resource/TemporaryFile.cpp
        core/data/resource/TemporaryFile.hpp
        core/data/share/LazyStringMap.hpp
        core/data/share/MemoryLabel.cpp
        core/data/share/MemoryLabel.hpp
        core/data/share/StringTemplate.cpp
        core/data/share/StringTemplate.hpp
        core/data/stream/BufferStream.cpp
        core/data/stream/BufferStream.hpp
        core/data/stream/FIFOStream.cpp
        core/data/stream/FIFOStream.hpp
        core/data/stream/FileStream.cpp
        core/data/stream/FileStream.hpp
        core/data/stream/Stream.cpp
        core/data/stream/Stream.hpp
        core/data/stream/StreamBufferedProxy.cpp
        core/data/stream/StreamBufferedProxy.hpp
        core/macro/basic.hpp
        core/macro/codegen.hpp
        core/macro/component.hpp
        core/parser/Caret.cpp
        core/parser/Caret.hpp
        core/parser/ParsingError.cpp
        core/parser/ParsingError.hpp
        core/provider/Invalidator.hpp
        core/provider/Pool.hpp
        core/provider/Provider.hpp
        core/utils/Binary.cpp
        core/utils/Binary.hpp
        core/utils/ConversionUtils.cpp
        core/utils/ConversionUtils.hpp
        core/utils/Random.cpp
        core/utils/Random.hpp
        core/utils/String.cpp
        core/utils/String.hpp
        encoding/Base64.cpp
        encoding/Base64.hpp
        encoding/Hex.cpp
        encoding/Hex.hpp
        encoding/Unicode.cpp
        encoding/Unicode.hpp
        encoding/Url.cpp
        encoding/Url.hpp
        network/Address.cpp
        network/Address.hpp
        network/ConnectionHandler.hpp
        network/ConnectionPool.cpp
        network/ConnectionPool.hpp
        network/ConnectionProvider.cpp
        network/ConnectionProvider.hpp
        network/ConnectionProviderSwitch.cpp
        network/ConnectionProviderSwitch.hpp
        network/Server.cpp
        network/Server.hpp
        network/Url.cpp
        network/Url.hpp
        network/monitor/ConnectionInactivityChecker.cpp
        network/monitor/ConnectionInactivityChecker.hpp
        network/monitor/ConnectionMaxAgeChecker.cpp
        network/monitor/ConnectionMaxAgeChecker.hpp
        network/monitor/ConnectionMonitor.cpp
        network/monitor/ConnectionMonitor.hpp
        network/monitor/MetricsChecker.hpp
        network/monitor/StatCollector.hpp
        network/tcp/Connection.cpp
        network/tcp/Connection.hpp
        network/tcp/client/ConnectionProvider.cpp
        network/tcp/client/ConnectionProvider.hpp
        network/tcp/server/ConnectionProvider.cpp
        network/tcp/server/ConnectionProvider.hpp
        network/virtual_/Interface.cpp
        network/virtual_/Interface.hpp
        network/virtual_/Pipe.cpp
        network/virtual_/Pipe.hpp
        network/virtual_/Socket.cpp
        network/virtual_/Socket.hpp
        network/virtual_/client/ConnectionProvider.cpp
        network/virtual_/client/ConnectionProvider.hpp
        network/virtual_/server/ConnectionProvider.cpp
        network/virtual_/server/ConnectionProvider.hpp
        orm/Connection.hpp
        orm/DbClient.cpp
        orm/DbClient.hpp
        orm/Executor.cpp
        orm/Executor.hpp
        orm/QueryResult.cpp
        orm/QueryResult.hpp
        orm/SchemaMigration.cpp
        orm/SchemaMigration.hpp
        orm/Transaction.cpp
        orm/Transaction.hpp
        parser/json/Beautifier.cpp
        parser/json/Beautifier.hpp
        parser/json/Utils.cpp
        parser/json/Utils.hpp
        parser/json/mapping/Deserializer.cpp
        parser/json/mapping/Deserializer.hpp
        parser/json/mapping/ObjectMapper.cpp
        parser/json/mapping/ObjectMapper.hpp
        parser/json/mapping/Serializer.cpp
        parser/json/mapping/Serializer.hpp
        web/client/ApiClient.cpp
        web/client/ApiClient.hpp
        web/client/HttpRequestExecutor.cpp
        web/client/HttpRequestExecutor.hpp
        web/client/RequestExecutor.cpp
        web/client/RequestExecutor.hpp
        web/client/RetryPolicy.cpp
        web/client/RetryPolicy.hpp
        web/mime/multipart/FileProvider.cpp
        web/mime/multipart/FileProvider.hpp
        web/mime/multipart/InMemoryDataProvider.cpp
        web/mime/multipart/InMemoryDataProvider.hpp
        web/mime/multipart/Multipart.cpp
        web/mime/multipart/Multipart.hpp
        web/mime/multipart/Part.cpp
        web/mime/multipart/Part.hpp
        web/mime/multipart/PartList.cpp
        web/mime/multipart/PartList.hpp
        web/mime/multipart/PartReader.cpp
        web/mime/multipart/PartReader.hpp
        web/mime/multipart/Reader.cpp
        web/mime/multipart/Reader.hpp
        web/mime/multipart/StatefulParser.cpp
        web/mime/multipart/StatefulParser.hpp
        web/mime/multipart/TemporaryFileProvider.cpp
        web/mime/multipart/TemporaryFileProvider.hpp
        web/protocol/CommunicationError.cpp
        web/protocol/CommunicationError.hpp
        web/protocol/http/Http.cpp
        web/protocol/http/Http.hpp
        web/protocol/http/encoding/Chunked.cpp
        web/protocol/http/encoding/Chunked.hpp
        web/protocol/http/encoding/EncoderProvider.hpp
        web/protocol/http/encoding/ProviderCollection.cpp
        web/protocol/http/encoding/ProviderCollection.hpp
        web/protocol/http/incoming/BodyDecoder.cpp
        web/protocol/http/incoming/BodyDecoder.hpp
        web/protocol/http/incoming/Request.cpp
        web/protocol/http/incoming/Request.hpp
        web/protocol/http/incoming/RequestHeadersReader.cpp
        web/protocol/http/incoming/RequestHeadersReader.hpp
        web/protocol/http/incoming/Response.cpp
        web/protocol/http/incoming/Response.hpp
        web/protocol/http/incoming/ResponseHeadersReader.cpp
        web/protocol/http/incoming/ResponseHeadersReader.hpp
        web/protocol/http/incoming/SimpleBodyDecoder.cpp
        web/protocol/http/incoming/SimpleBodyDecoder.hpp
        web/protocol/http/outgoing/Body.cpp
        web/protocol/http/outgoing/Body.hpp
        web/protocol/http/outgoing/BufferBody.cpp
        web/protocol/http/outgoing/BufferBody.hpp
        web/protocol/http/outgoing/MultipartBody.cpp
        web/protocol/http/outgoing/MultipartBody.hpp
        web/protocol/http/outgoing/Request.cpp
        web/protocol/http/outgoing/Request.hpp
        web/protocol/http/outgoing/Response.cpp
        web/protocol/http/outgoing/Response.hpp
        web/protocol/http/outgoing/ResponseFactory.cpp
        web/protocol/http/outgoing/ResponseFactory.hpp
        web/protocol/http/outgoing/StreamingBody.cpp
        web/protocol/http/outgoing/StreamingBody.hpp
        web/protocol/http/utils/CommunicationUtils.cpp
        web/protocol/http/utils/CommunicationUtils.hpp
        web/server/AsyncHttpConnectionHandler.cpp
        web/server/AsyncHttpConnectionHandler.hpp
        web/server/HttpConnectionHandler.cpp
        web/server/HttpConnectionHandler.hpp
        web/server/HttpProcessor.cpp
        web/server/HttpProcessor.hpp
        web/server/HttpRequestHandler.hpp
        web/server/HttpRouter.cpp
        web/server/HttpRouter.hpp
        web/server/api/ApiController.cpp
        web/server/api/ApiController.hpp
        web/server/api/Endpoint.cpp
        web/server/api/Endpoint.hpp
        web/server/handler/AuthorizationHandler.cpp
        web/server/handler/AuthorizationHandler.hpp
        web/server/handler/ErrorHandler.cpp
        web/server/handler/ErrorHandler.hpp
        web/server/interceptor/AllowCorsGlobal.cpp
        web/server/interceptor/AllowCorsGlobal.hpp
        web/server/interceptor/RequestInterceptor.hpp
        web/server/interceptor/ResponseInterceptor.hpp
        web/url/mapping/Pattern.cpp
        web/url/mapping/Pattern.hpp
        web/url/mapping/Router.hpp
)
include(Oatpp)
set_target_source_groups(oatpp STRIP_PREFIX ".")

set_target_properties(oatpp PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)
if (MSVC)
    target_compile_options(oatpp PRIVATE /permissive-)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD ON)

find_package(Threads REQUIRED)

#######################################################################################################
## Link external libraries

SET(OATPP_ADD_LINK_LIBS "")

if(MSVC OR MINGW)
        SET(OATPP_ADD_LINK_LIBS wsock32 ws2_32)
elseif(NOT APPLE AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        if(OATPP_LINK_ATOMIC)
                SET(OATPP_ADD_LINK_LIBS atomic)
        endif()
endif()

message("OATPP_ADD_LINK_LIBS=${OATPP_ADD_LINK_LIBS}")

target_link_libraries(oatpp PUBLIC ${CMAKE_THREAD_LIBS_INIT}
        ${OATPP_ADD_LINK_LIBS}
)

target_include_directories(oatpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
