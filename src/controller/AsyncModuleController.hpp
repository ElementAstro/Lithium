#ifndef Lithium_MODULECONTROLLER_HPP
#define Lithium_MODULECONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "LithiumApp.hpp"
#include "data/ModuleDto.hpp"
#include "data/StatusDto.hpp"

#include "atom/plugin/module_loader.hpp"
#include "atom/server/global_ptr.hpp"
#include "core/plugin/plugin.hpp"
#include "core/device.hpp"

#include "template/variable.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class ModuleController : public oatpp::web::server::api::ApiController
{
public:
    ModuleController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<ModuleController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<ModuleController>(objectMapper);
    }

public:

    ENDPOINT_INFO(getUILoadModule)
    {
        info->summary = "Load a plugin module from the specified path";
        info->addConsumes<Object<LoadPluginDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/load", getUILoadModule)
    {
        ENDPOINT_ASYNC_INIT(getUILoadModule)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<LoadPluginDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUILoadModule::returnResponse);
        }

        Action returnResponse(const oatpp::Object<LoadPluginDto>& body)
        {
            auto res = StatusDto::createShared();
            OATPP_ASSERT_HTTP(body->plugin_path.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            OATPP_ASSERT_HTTP(body->plugin_type.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            OATPP_ASSERT_HTTP(body->plugin_name.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            auto plugin_path = body->plugin_path.getValue("");
            auto plugin_name = body->plugin_name.getValue("");
            auto plugin_type = body->plugin_type.getValue("");
            if (!Lithium::MyApp->loadModule(plugin_path, plugin_name))
            {
                res->error = "ModuleError";
                res->message = fmt::format("Failed to load module: {}", plugin_name);
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIUnloadModule)
    {
        info->summary = "Unload module by name";
        info->addConsumes<Object<UnloadPluginDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/unload", getUIUnloadModule)
    {
        ENDPOINT_ASYNC_INIT(getUIUnloadModule)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<UnloadPluginDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIUnloadModule::returnResponse);
        }

        Action returnResponse(const oatpp::Object<UnloadPluginDto>& body)
        {
            auto res = StatusDto::createShared();
            OATPP_ASSERT_HTTP(body->plugin_name.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            auto plugin_name = body->plugin_name.getValue("");
            if (!Lithium::MyApp->unloadModule(plugin_name))
            {
                res->error = "ModuleError";
                res->message = fmt::format("Failed to unload module: {}", plugin_name);
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIGetModuleList)
    {
        info->summary = "Get module list";
        info->addConsumes<Object<GetModuleListDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<ReturnModuleListDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/list", getUIGetModuleList)
    {
        ENDPOINT_ASYNC_INIT(getUIGetModuleList)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<GetModuleListDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIGetModuleList::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetModuleListDto>& body)
        {
            auto res = ReturnModuleListDto::createShared();
            OATPP_ASSERT_HTTP(body->plugin_path.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            auto plugin_path = body->plugin_path.getValue("");
            auto module_list = Lithium::MyApp->getModuleList();
            for (auto module : module_list)
            {
                res->module_list->push_back(module);
                OATPP_LOGD("ModuleController", "Module: %s", module.c_str());
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRefreshModuleLists)
    {
        info->summary = "Refresh module list";
        info->addConsumes<Object<RefreshModuleListDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<ReturnModuleListDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/refresh", getUIRefreshModuleLists)
    {
        ENDPOINT_ASYNC_INIT(getUIRefreshModuleLists)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RefreshModuleListDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRefreshModuleLists::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RefreshModuleListDto>& body)
        {
            auto res = StatusDto::createShared();
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIEnableModule)
    {
        info->summary = "Enable module by name";
        info->addConsumes<Object<GetEnableModuleDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<ReturnEnableModuleDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/enable", getUIEnableModule)
    {
        ENDPOINT_ASYNC_INIT(getUIEnableModule)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<GetEnableModuleDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIEnableModule::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetEnableModuleDto>& body)
        {
            auto res = StatusDto::createShared();
            OATPP_ASSERT_HTTP(body->plugin_name.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            auto plugin_name = body->plugin_name.getValue("");
            if (!Lithium::MyApp->enableModule(plugin_name))
            {
                res->error = "ModuleError";
                res->message = fmt::format("Failed to enable module: {}", plugin_name);
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIDisableModule)
    {
        info->summary = "Disable module by name";
        info->addConsumes<Object<GetDisableModuleDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<ReturnDisableModuleDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/disable", getUIDisableModule)
    {
        ENDPOINT_ASYNC_INIT(getUIDisableModule)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<GetDisableModuleDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIDisableModule::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetDisableModuleDto>& body)
        {
            auto res = StatusDto::createShared();
            OATPP_ASSERT_HTTP(body->plugin_name.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            auto plugin_name = body->plugin_name.getValue("");
            if (!Lithium::MyApp->disableModule(plugin_name))
            {
                res->error = "ModuleError";
                res->message = fmt::format("Failed to disable module: {}", plugin_name);
            }
        }
    };

    ENDPOINT_INFO(getUIGetModuleStatus)
    {
        info->summary = "Get module status";
        info->addConsumes<Object<GetModuleStatusDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<ReturnModuleStatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/status", getUIGetModuleStatus)
    {
        ENDPOINT_ASYNC_INIT(getUIGetModuleStatus)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<GetModuleStatusDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIGetModuleStatus::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetModuleStatusDto>& body)
        {
            auto res = ReturnModuleStatusDto::createShared();
            OATPP_ASSERT_HTTP(body->module_name.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            auto module_name = body->module_name.getValue("");
            auto module_status = Lithium::MyApp->getModuleStatus(module_name);
            if (module_status)
            {
                res->module_status = module_status;
            }
            else
            {
                res->error = "ModuleError";
                res->message = fmt::format("Failed to get module status: {}", module_name);
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIGetModuleConfig)
    {
        info->summary = "Get module config";
        info->addConsumes<Object<GetModuleConfigDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<ReturnModuleConfigDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/config", getUIGetModuleConfig)
    {
        ENDPOINT_ASYNC_INIT(getUIGetModuleConfig)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<GetModuleConfigDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIGetModuleConfig::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetModuleConfigDto>& body)
        {
            auto res = ReturnModuleConfigDto::createShared();
            OATPP_ASSERT_HTTP(body->module_name.getValue("")!= "", Status::CODE_400, "Invalid Parameters");
            auto module_name = body->module_name.getValue("");
            auto module_config = Lithium::MyApp->getModuleConfig(module_name);
            if (!module_config.empty())
            {
                res->module_config = module_config.dump(4);
            }
            else
            {
                res->error = "ModuleError";
                res->message = fmt::format("Failed to get module config: {}", module_name);
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    // This function is very dangerous, please use it carefully
    // It will return the shared_ptr of the module, but we will do nothing with it
    // So please make sure you know what you are doing
    ENDPOINT_INFO(getUIGetInstance)
    {
        info->summary = "Get shared_ptr from a specific module and register it into global ptr mamanger";
        info->addConsumes<Object<GetInstanceDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
        info->addResponse<Object<ReturnModuleConfigDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET","/api/module/get/instance", getUIGetInstance)
    {
        ENDPOINT_ASYNC_INIT(getUIGetInstance)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<GetInstanceDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUIGetInstance::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetInstanceDto>& body)
        {
            auto res = ReturnModuleConfigDto::createShared();
            CHECK_VARIABLE(module_name,"Invalid Parameters")
            CHECK_VARIABLE(instance_name,"Invalid Parameters")
            CHECK_VARIABLE(instance_type,"Invalid Parameters")
            CHECK_VARIABLE(get_func,"Invalid Parameters")
            if (instance_type == "plugin" || instance_type == "module")
            {
                AddPtr(instance_name, GetPtr<Lithium::ModuleLoader>("ModuleLoader")->GetInstance<Plugin>(module_name, {}, instance_type));
            }
            else if (instance_type == "device")
            {
                AddPtr(instance_name, GetPtr<Lithium::ModuleLoader>("ModuleLoader")->GetInstance<Device>(module_name, {}, instance_type));
            }
            else
            {
                res->error = "ModuleError";
                res->message = fmt::format("Failed to get instance: {}", instance_name);
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_MODULECONTROLLER_HPP