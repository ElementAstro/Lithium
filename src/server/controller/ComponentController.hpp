#ifndef LITHIUM_SERVER_COMPONENT_CONTROLLER_HPP
#define LITHIUM_SERVER_COMPONENT_CONTROLLER_HPP

#include <algorithm>
#include <any>
#include <exception>
#include <optional>
#include <stdexcept>
#include "Types.hpp"
#include "addon/toolchain.hpp"
#include "components/dispatch.hpp"
#include "components/var.hpp"
#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "data/ComponentDto.hpp"
#include "data/RequestDto.hpp"

#include "addon/manager.hpp"
#include "utils/constant.hpp"

#include "atom/async/message_bus.hpp"
#include "atom/async/queue.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/container.hpp"
#include "atom/utils/to_any.hpp"

#if ENABLE_ASYNC
#include "atom/io/async_io.hpp"
#else
#include "atom/io/io.hpp"
#endif

template <typename T>
using OatppField = oatpp::Fields<T>;

// 定义数字类型的元组
using OatppNumberTypes =
    std::tuple<oatpp::Int8, oatpp::Int16, oatpp::Int32, oatpp::Int64,
               oatpp::UInt8, oatpp::UInt16, oatpp::UInt32, oatpp::UInt64,
               oatpp::Float32, oatpp::Float64>;

template <typename T, typename Tuple, std::size_t... I>
constexpr bool is_oatpp_number_field_type_impl(std::index_sequence<I...>) {
    return std::disjunction_v<
        std::is_same<T, OatppField<std::tuple_element_t<I, Tuple>>>...>;
}

template <typename T>
constexpr bool is_oatpp_number_field_type =
    is_oatpp_number_field_type_impl<T, OatppNumberTypes>(
        std::make_index_sequence<std::tuple_size_v<OatppNumberTypes>>{});

auto jsonToPackageJsonDto(const std::string& json) {
    // 创建一个 JSON 解析器
    auto objectMapper = oatpp::json::ObjectMapper();

    // 解析 JSON 字符串并转换为 PackageJsonDto 对象
    auto packageJsonDto =
        objectMapper.readFromString<oatpp::Object<PackageJsonDto>>(json);

    return packageJsonDto;
}

using json = nlohmann::json;

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class ComponentController : public oatpp::web::server::api::ApiController {
    using ControllerType = ComponentController;
    static std::weak_ptr<lithium::ComponentManager> mComponentManager;
    static std::weak_ptr<atom::async::MessageBus> mMessageBus;
    static std::shared_ptr<atom::async::ThreadSafeQueue<json>> mMessageQueue;

#if ENABLE_ASYNC
    static std::weak_ptr<atom::async::io::AsyncFile> mAsyncIO;
#endif

public:
    ComponentController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                        objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {
        GET_OR_CREATE_WEAK_PTR(mComponentManager, lithium::ComponentManager,
                               Constants::COMPONENT_MANAGER);
        GET_OR_CREATE_WEAK_PTR(mMessageBus, atom::async::MessageBus,
                               Constants::MESSAGE_BUS);
        mMessageQueue = std::make_shared<atom::async::ThreadSafeQueue<json>>();

        mMessageBus.lock()->subscribe<json>(
            Constants::MESSAGE_BUS, [](const json& message) {
                LOG_F(INFO, "Message received: {}", message.dump());
                ComponentController::mMessageQueue->emplace(message);
            });
#if ENABLE_ASYNC
        GET_OR_CREATE_PTR(mAsyncIO, atom::async::io::AsyncFile,
                          Constants::ASYNC_IO);
#endif
    }

    ENDPOINT_INFO(getUIApiServreComponentLoad) {
        info->summary = "Hot load component";
        info->addConsumes<Object<RequestComponentLoadDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<ReturnComponentLoadNotFoundDto>>(
            Status::CODE_300, "application/json");
        info->addResponse<Object<ReturnComponentFailToLoadDto>>(
            Status::CODE_301, "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/component/load", getUIApiServreComponentLoad) {
        ENDPOINT_ASYNC_INIT(getUIApiServreComponentLoad);

        static constexpr auto COMMAND = "lithium.server.component.load";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestComponentLoadDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServreComponentLoad::returnResponse);
        }

        template <typename T>
        auto createErrorResponse(const std::string& errorMsg,
                                 const std::string& componentName, int code) {
            auto res = T::createShared();
            res->status = "error";
            res->code = code;
            res->error = errorMsg;
            res->command = COMMAND;
            res->component = componentName;
            return controller->createDtoResponse(Status::CODE_300, res);
        }

        auto handleLoadFailure(const std::string& componentName) {
            auto res = ReturnComponentFailToLoadDto::createShared();
            res->status = "error";
            res->code = 301;
            res->command = COMMAND;
            res->component = componentName;

            if (auto msg = mMessageQueue->take(); msg.has_value()) {
                res->error = msg.value()["error"].get<std::string>();
                res->stacktrace = msg.value()["stacktrace"].get<std::string>();
                LOG_F(ERROR, "Failed to load component: {}, {}", *res->error,
                      *res->stacktrace);
            } else {
                res->error = "Failed to load component";
            }

            return controller->createDtoResponse(Status::CODE_301, res);
        }

        static auto verifyComponentsLoaded(
            const oatpp::List<Object<ComponentDto>>& components,
            const std::vector<std::string>& loadedComponents) -> bool {
            std::vector<std::string> componentsList;
            for (const auto& component : *components) {
                componentsList.push_back(component->name.getValue(""));
            }
            return atom::utils::isSubset(componentsList, loadedComponents);
        }

        auto createSuccessResponse(const std::string& message) {
            auto res = StatusDto::createShared();
            res->status = "success";
            res->code = 200;
            res->command = COMMAND;
            res->message = message;
            return controller->createDtoResponse(Status::CODE_200, res);
        }

        auto returnResponse(
            const oatpp::Object<RequestComponentLoadDto>& body) -> Action {
            auto components = body->components;
            auto componentManager = mComponentManager.lock();

            for (const auto& component : *components) {
                auto componentName = component->name;
                auto componentPath = component->path;
                auto componentInstance = component->instance;
                auto componentFullName =
                    componentName + "::" + componentInstance;

                if (componentManager->hasComponent(componentFullName)) {
                    LOG_F(WARNING, "Component {} already loaded",
                          componentInstance.getValue(""));
                    continue;
                }

                auto componentLibrary =
                    componentPath + Constants::PATH_SEPARATOR + componentName +
                    Constants::LIB_EXTENSION;

                bool isExist = false;

#if ENABLE_ASYNC
                mAsyncIO.lock()->asyncExists(
                    componentLibrary.getValue(""),
                    [&isExist](bool result) { isExist = result; });
#else
                isExist = atom::io::isFileExists(componentLibrary.c_str());
#endif
                if (!isExist) {
                    return _return(
                        createErrorResponse<ReturnComponentLoadNotFoundDto>(
                            "Component library not found", componentName, 300));
                }

                // 加载组件
                if (componentManager->loadComponent(
                        {{"name", componentName},
                         {"path", componentPath},
                         {"instance", componentInstance},
                         {"library", componentLibrary}})) {
                    LOG_F(INFO, "Component {} loaded",
                          componentInstance.getValue(""));
                } else {
                    LOG_F(ERROR, "Failed to load component {}",
                          componentInstance.getValue(""));
                    return _return(handleLoadFailure(componentName));
                }
            }

            if (!verifyComponentsLoaded(components,
                                        componentManager->getComponentList())) {
                return _return(
                    createErrorResponse<ReturnComponentFailToLoadDto>(
                        "Failed to load component", "", 301));
            }

            return _return(createSuccessResponse("Components loaded"));
        }
    };

    ENDPOINT_INFO(getUIApiServreComponentUnload) {
        info->summary = "Unload component";
        info->addConsumes<RequestComponentUnloadDto>(
            "application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<ReturnComponentUnloadNotFoundDto>>(
            Status::CODE_300, "application/json");
        info->addResponse<Object<ReturnComponentFailToUnloadDto>>(
            Status::CODE_301, "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/component/unload",
                   getUIApiServreComponentUnload) {
        ENDPOINT_ASYNC_INIT(getUIApiServreComponentUnload);

        static constexpr auto COMMAND = "lithium.server.component.unload";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestComponentUnloadDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServreComponentUnload::returnResponse);
        }

        template <typename T>
        auto createErrorResponse(const std::string& errorMsg,
                                 const std::string& componentName, int code) {
            auto res = T::createShared();
            res->status = "error";
            res->code = code;
            res->error = errorMsg;
            res->command = COMMAND;
            res->component = componentName;
            return controller->createDtoResponse(Status::CODE_300, res);
        }

        auto handleUnloadFailure(const std::string& componentName) {
            auto res = ReturnComponentFailToUnloadDto::createShared();
            res->status = "error";
            res->code = 301;
            res->command = COMMAND;
            res->component = componentName;

            if (auto msg = mMessageQueue->take(); msg.has_value()) {
                res->error = msg.value()["error"].get<std::string>();
                res->stacktrace = msg.value()["stacktrace"].get<std::string>();
                LOG_F(ERROR, "Failed to unload component: {}, {}", *res->error,
                      *res->stacktrace);
            } else {
                res->error = "Failed to unload component";
            }

            return controller->createDtoResponse(Status::CODE_301, res);
        }

        auto createSuccessResponse(const std::string& message) {
            auto res = StatusDto::createShared();
            res->status = "success";
            res->code = 200;
            res->command = COMMAND;
            res->message = message;
            return controller->createDtoResponse(Status::CODE_200, res);
        }

        auto returnResponse(
            const oatpp::Object<RequestComponentUnloadDto>& body) -> Action {
            auto components = body->components;
            auto componentManager = mComponentManager.lock();

            for (const auto& component : *components) {
                auto componentName = component->name;
                auto componentInstance = component->instance;
                auto componentFullName =
                    componentName + "::" + componentInstance;

                if (!componentManager->hasComponent(componentFullName)) {
                    LOG_F(WARNING, "Component {} not loaded",
                          componentInstance.getValue(""));
                    continue;
                }

                if (!componentManager->unloadComponent(componentFullName)) {
                    LOG_F(ERROR, "Failed to unload component {}",
                          componentInstance.getValue(""));
                    return _return(handleUnloadFailure(componentName));
                }

                if (componentManager->hasComponent(componentFullName)) {
                    LOG_F(ERROR, "Failed to unload component {}",
                          componentInstance.getValue(""));
                    return _return(handleUnloadFailure(componentName));
                }

                LOG_F(INFO, "Component {} unloaded",
                      componentInstance.getValue(""));
                return _return(createSuccessResponse("Components unloaded"));
            }
            return _return(createSuccessResponse("Components unloaded"));
        }
    };

    ENDPOINT_INFO(getUIApiServreComponentReload) {
        info->summary = "Reload component";
        info->addConsumes<RequestComponentReloadDto>(
            "application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/component/reload",
                   getUIApiServreComponentReload) {
        ENDPOINT_ASYNC_INIT(getUIApiServreComponentReload);

        static constexpr auto COMMAND = "lithium.server.component.reload";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestComponentUnloadDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServreComponentReload::returnResponse);
        }

        template <typename T>
        auto createErrorResponse(const std::string& errorMsg,
                                 const std::string& componentName, int code) {
            auto res = T::createShared();
            res->status = "error";
            res->code = code;
            res->error = errorMsg;
            res->command = COMMAND;
            res->component = componentName;
            return controller->createDtoResponse(Status::CODE_300, res);
        }

        auto handleUnloadFailure(const std::string& componentName) {
            auto res = ReturnComponentFailToUnloadDto::createShared();
            res->status = "error";
            res->code = 301;
            res->command = COMMAND;
            res->component = componentName;

            if (auto msg = mMessageQueue->take(); msg.has_value()) {
                res->error = msg.value()["error"].get<std::string>();
                res->stacktrace = msg.value()["stacktrace"].get<std::string>();
                LOG_F(ERROR, "Failed to unload component: {}, {}", *res->error,
                      *res->stacktrace);
            } else {
                res->error = "Failed to unload component";
            }

            return controller->createDtoResponse(Status::CODE_301, res);
        }

        auto handleLoadFailure(const std::string& componentName) {
            auto res = ReturnComponentFailToLoadDto::createShared();
            res->status = "error";
            res->code = 301;
            res->command = COMMAND;
            res->component = componentName;

            if (auto msg = mMessageQueue->take(); msg.has_value()) {
                res->error = msg.value()["error"].get<std::string>();
                res->stacktrace = msg.value()["stacktrace"].get<std::string>();
                LOG_F(ERROR, "Failed to load component: {}, {}", *res->error,
                      *res->stacktrace);
            } else {
                res->error = "Failed to load component";
            }
            return controller->createDtoResponse(Status::CODE_301, res);
        }

        auto createSuccessResponse(const std::string& message) {
            auto res = StatusDto::createShared();
            res->status = "success";
            res->code = 200;
            res->command = COMMAND;
            res->message = message;
            return controller->createDtoResponse(Status::CODE_200, res);
        }

        auto returnResponse(
            const oatpp::Object<RequestComponentUnloadDto>& body) -> Action {
            auto components = body->components;
            auto componentManager = mComponentManager.lock();

            for (const auto& component : *components) {
                auto componentName = component->name;
                auto componentInstance = component->instance;
                auto componentFullName =
                    componentName + "::" + componentInstance;

                if (!componentManager->hasComponent(componentFullName)) {
                    LOG_F(WARNING, "Component {} not loaded",
                          componentInstance.getValue(""));
                    continue;
                }

                if (!componentManager->reloadComponent(
                        {{"name", componentName},
                         {"instance", componentInstance}})) {
                    LOG_F(ERROR, "Failed to reload component {}",
                          componentInstance.getValue(""));
                    return _return(handleUnloadFailure(componentName));
                }

                LOG_F(INFO, "Component {} reloaded",
                      componentInstance.getValue(""));
                return _return(createSuccessResponse("Components reloaded"));
            }
        }
    };

    ENDPOINT_INFO(getUIApiServreComponentList) {
        info->name = "getUIApiServreComponentList";
        info->summary = "Get component list";
        info->description =
            "Get component list from the server, including "
            "component name, instance, and description";
        info->addConsumes<Object<RequestDto>>("application/json");
        info->addResponse<Object<ReturnComponentListDto>>(Status::CODE_200,
                                                          "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/component/list", getUIApiServreComponentList) {
        ENDPOINT_ASYNC_INIT(getUIApiServreComponentList);

        static constexpr auto COMMAND = "lithium.server.component.list";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServreComponentList::returnResponse);
        }

        auto returnResponse(
            [[maybe_unused]] const oatpp::Object<RequestDto>& body) -> Action {
            auto res = ReturnComponentListDto::createShared();
            res->status = "success";
            res->code = 200;
            res->command = COMMAND;
            res->message = "Components list";
            for (const auto& component :
                 mComponentManager.lock()->getComponentList()) {
                auto instance = ComponentInstanceDto::createShared();
                auto info =
                    mComponentManager.lock()->getComponentInfo(component);
                if (!info.has_value()) {
                    LOG_F(ERROR, "Failed to get component info: {}", component);
                    continue;
                }
                /*
                Example:
                {
                    "name": "component",
                    "instance": "instance",
                    "description": "description",
                    "functions": [
                        {
                            "name": "function",
                            "description": "description",
                            "argsType": ["type"],
                            "returnType": "type"
                        }
                    ]
                }
                */
                instance->name = component;
                instance->instance = component;
                instance->description =
                    mComponentManager.lock()->getComponentDoc(component);
                for (const auto& func : info.value()["functions"].get<json>()) {
                    if (!func.is_object() || !func.contains("name") ||
                        !func.contains("description") ||
                        !func.contains("argsType") ||
                        !func.contains("returnType")) {
                        LOG_F(ERROR, "Failed to get function info: {}",
                              component);
                        LOG_F(ERROR,
                              "Just tell the developer and punish him for "
                              "not following the rules");
                        continue;
                    }
                    auto function = ComponentFunctionDto();
                    function.name = func["name"].get<std::string>();
                    function.description =
                        func["description"].get<std::string>();
                    for (const auto& arg :
                         func["argsType"].get<std::vector<std::string>>()) {
                        function.argsType->emplace_back(arg);
                    }
                    function.returnType = func["returnType"].get<std::string>();
                }
                res->components->emplace_back(instance);
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIApiServreComponentInfo) {
        info->name = "getUIApiServreComponentInfo";
        info->summary = "Get component info";
        info->description =
            "Get the specific component info from the server, just like "
            "package.json";
        info->addConsumes<Object<RequestComponentInfoDto>>("application/json");
        info->addResponse<Object<ReturnComponentInfoDto>>(Status::CODE_200,
                                                          "application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_300,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/component/info", getUIApiServreComponentInfo) {
        ENDPOINT_ASYNC_INIT(getUIApiServreComponentInfo);

        static constexpr auto COMMAND = "lithium.server.component.info";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestComponentInfoDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServreComponentInfo::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<RequestComponentInfoDto>& body) -> Action {
            auto component = body->component;
            auto componentManager = mComponentManager.lock();
            auto componentInfo = componentManager->getComponentInfo(component);

            if (!componentInfo.has_value()) {
                auto res = StatusDto::createShared();
                res->status = "error";
                res->code = 300;
                res->command = COMMAND;
                res->message = "Component not found";
                return _return(
                    controller->createDtoResponse(Status::CODE_300, res));
            }

            auto res = ReturnComponentInfoDto::createShared();
            res->status = "success";
            res->code = 200;
            res->command = COMMAND;
            res->message = "Component info";
            res->component_info->emplace_back(
                jsonToPackageJsonDto(componentInfo.value().dump()));
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIApiServerComponentRunFunction) {
        info->summary = "Run component function";
        info->addConsumes<Object<RequestComponentRunFunctionDto>>(
            "application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<ReturnComponentFunctionNotFoundDto>>(
            Status::CODE_300, "application/json");
        info->addResponse<Object<ReturnComponentFunctionFailToRunDto>>(
            Status::CODE_301, "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/component/run",
                   getUIApiServerComponentRunFunction) {
        ENDPOINT_ASYNC_INIT(getUIApiServerComponentRunFunction);

        static constexpr auto COMMAND = "lithium.server.component.run";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<
                    oatpp::Object<RequestComponentRunFunctionDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(
                    &getUIApiServerComponentRunFunction::returnResponse);
        }

        template <typename T>
        auto createErrorResponse(const std::string& errorMsg,
                                 const std::string& componentName,
                                 const std::string& functionName, int code) {
            auto res = T::createShared();
            res->status = "error";
            res->code = code;
            res->error = errorMsg;
            res->command = COMMAND;
            res->component = componentName;
            res->function = functionName;
            return controller->createDtoResponse(Status::CODE_300, res);
        }

        auto handleRunFailure(const std::string& componentName,
                              const std::string& functionName) {
            auto res = ReturnComponentFunctionFailToRunDto::createShared();
            res->status = "error";
            res->code = 301;
            res->command = COMMAND;
            res->component = componentName;
            res->function = functionName;

            if (auto msg = mMessageQueue->take(); msg.has_value()) {
                res->error = msg.value()["error"].get<std::string>();
                res->stacktrace = msg.value()["stacktrace"].get<std::string>();
                LOG_F(ERROR, "Failed to run function: {}, {}", *res->error,
                      *res->stacktrace);
            } else {
                res->error = "Failed to run function";
            }

            return controller->createDtoResponse(Status::CODE_301, res);
        }

        auto createSuccessResponse(const std::string& message) {
            auto res = StatusDto::createShared();
            res->status = "success";
            res->code = 200;
            res->command = COMMAND;
            res->message = message;
            return controller->createDtoResponse(Status::CODE_200, res);
        }

        template <typename RetrieveFunc, typename T>
        auto process(RetrieveFunc & retrieveFunc, const oatpp::Any& arg,
                     std::vector<std::any>& functionArgs) -> bool {
            if (auto argValue = retrieveFunc.template operator()<T>(arg);
                argValue.has_value()) {
                if constexpr (std::is_same_v<T, oatpp::String>) {
                    functionArgs.emplace_back(argValue.value().getValue(""));
                } else if constexpr (
                    std::is_same_v<T, oatpp::List<oatpp::Int8>> ||
                    std::is_same_v<T, oatpp::List<oatpp::Int16>> ||
                    std::is_same_v<T, oatpp::List<oatpp::Int32>> ||
                    std::is_same_v<T, oatpp::List<oatpp::Int64>> ||
                    std::is_same_v<T, oatpp::List<oatpp::UInt8>> ||
                    std::is_same_v<T, oatpp::List<oatpp::UInt16>> ||
                    std::is_same_v<T, oatpp::List<oatpp::UInt32>> ||
                    std::is_same_v<T, oatpp::List<oatpp::UInt64>>) {
                    // functionArgs.emplace_back(argValue.value().getValue(0));
                    return true;
                }
                return false;
            }
        }

        template <typename RetrieveFunc, typename... Types>
        auto processes(RetrieveFunc & retrieveFunc, const oatpp::Any& arg,
                       std::vector<std::any>& functionArgs,
                       const std::tuple<Types...>&) -> bool {
            return (
                process<RetrieveFunc, Types>(retrieveFunc, arg, functionArgs) ||
                ...);
        }

        auto returnResponse(
            const oatpp::Object<RequestComponentRunFunctionDto>& body)
            -> Action {
            LOG_SCOPE_FUNCTION(INFO);
            auto component = body->component;
            auto function = body->function;
            auto args = body->args;
            auto anyArgs = body->anyArgs;
            auto ignore = body->ignore;
            auto componentManager = mComponentManager.lock();

            // TODO: Here we have a serious problem, we need to parse the
            // json to get function arguments, but we should we treat all
            // arguments as string first and then parse them to real type?
            // Maybe we could use oatpp::Any to store the arguments and then
            // directly pass them to std::vector<any> or FunctionParams

            if (!componentManager->hasComponent(component)) {
                LOG_F(ERROR, "Component {} not found", *component);
                return _return(
                    createErrorResponse<ReturnComponentFunctionNotFoundDto>(
                        "Component not found", component, function, 300));
            }

            // Check if the component is loaded
            if (auto componentWeakPtr =
                    componentManager->getComponent(component);
                !componentWeakPtr.has_value() || componentWeakPtr->expired()) {
                LOG_F(ERROR, "Component pointer is invalid: {}", *component);
                return _return(
                    createErrorResponse<ReturnComponentFunctionNotFoundDto>(
                        "Component pointer is invalid", component, function,
                        300));
            } else {
                auto componentPtr = componentWeakPtr->lock();
                if (!componentPtr->has(function)) {
                    LOG_F(ERROR, "Function {} not found", *function);
                    return _return(
                        createErrorResponse<ReturnComponentFunctionNotFoundDto>(
                            "Function not found", component, function, 300));
                }

                // Parse the json message and get the function arguments
                // There we have two list of arguments, and we assume that
                // only one of them is not empty
                std::vector<std::any> functionArgs;
                if (!args->empty()) {
                    functionArgs.reserve(args->size());
                    atom::utils::Parser parser;
                    for (const auto& arg : *args) {
                        LOG_F(INFO, "Argument: {}", arg.getValue(""));
                        // Then we will use atom::utils::Parser to parse the
                        // string to real type
                        // !Important: the string argument
                        // must have a literal type identifier or it will
                        // simply be treated as string Here we will try to
                        // parse it in one-by-one mode
                        auto realArg = parser.parseLiteral(arg.getValue(""));
                        if (realArg.has_value()) {
                            functionArgs.push_back(realArg.value());
                        } else {
                            LOG_F(ERROR, "Failed to parse argument: {}", arg);
                            return _return(createErrorResponse<
                                           ReturnComponentFunctionNotFoundDto>(
                                "Failed to parse argument", component, function,
                                300));
                        }
                    }
                } else if (!anyArgs->empty()) {
                    functionArgs.reserve(anyArgs->size());
                    auto retrieveFunc =
                        []<typename T>(
                            const oatpp::Any& arg) -> std::optional<T> {
                        // Due to the conversion rule from oatpp::Any
                        try {
                            return arg.retrieve<T>();
                        } catch (const std::runtime_error& e) {
                            return std::nullopt;
                        }
                    };

                    for (const auto& arg : *anyArgs) {
                        // How should we deal with: arg->type->nameQualifier
                        // Here we will ttry to retrieve the value one type
                        // by one type, if the type is matched then we will
                        // extract the value and push it to the functionArgs
                        // Call the processes function to process all types

                        // TODO: Here we need a simple way to handle the
                        // argument conversion
                        bool success = processes(
                            retrieveFunc, arg, functionArgs,
                            std::tuple<oatpp::Int8, oatpp::Int16, oatpp::Int32,
                                       oatpp::Int64, oatpp::UInt8,
                                       oatpp::UInt16, oatpp::UInt32,
                                       oatpp::UInt64, oatpp::Float32,
                                       oatpp::Float64, oatpp::String,
                                       oatpp::Boolean, oatpp::List<oatpp::Int8>,
                                       oatpp::List<oatpp::Int16>,
                                       oatpp::List<oatpp::Int32>,
                                       oatpp::List<oatpp::Int64>,
                                       oatpp::List<oatpp::UInt8>,
                                       oatpp::List<oatpp::UInt16>,
                                       oatpp::List<oatpp::UInt32>,
                                       oatpp::List<oatpp::UInt64>,
                                       oatpp::List<oatpp::Float32>,
                                       oatpp::List<oatpp::Float64>,
                                       oatpp::List<oatpp::String>,
                                       oatpp::List<oatpp::Boolean>>{});
                        if (!success) {
                            return _return(createErrorResponse<
                                           ReturnComponentFunctionNotFoundDto>(
                                "Failed to parse argument", component, function,
                                300));
                        }
                    }
                }

                // Call the function
                try {
                    auto result =
                        componentPtr->dispatch(function, functionArgs);
                } catch (const DispatchException& e) {
                    LOG_F(ERROR, "Failed to run function: {}", e.what());
                    return _return(handleRunFailure(component, function));
                } catch (const DispatchTimeout& e) {
                    LOG_F(ERROR, "Failed to run function: {}", e.what());
                    return _return(handleRunFailure(component, function));
                } catch (const std::exception& e) {
                    LOG_F(ERROR, "Failed to run function: {}", e.what());
                    return _return(handleRunFailure(component, function));
                }
            }
            if (ignore) {
                return _return(createSuccessResponse("Function ignored"));
            }
            return _return(createSuccessResponse("Function executed"));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // LITHIUM_SERVER_COMPONENT_CONTROLLER_HPP
