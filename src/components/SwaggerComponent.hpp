/*
 * SwaggerComponent.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-11

Description: Oatpp Swagger component

**************************************************/

#ifndef SwaggerComponent_hpp
#define SwaggerComponent_hpp

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/core/macro/component.hpp"

/**
 *  Swagger ui is served at
 *  http://host:port/swagger/ui
 */
class SwaggerComponent
{
public:
    /**
     *  General API docs info
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)
    ([]
     {
         oatpp::swagger::DocumentInfo::Builder builder;

         builder
             .setTitle("Lithium API")
             .setDescription("Lithium API")
             .setVersion("1.0")
             .setContactName("Max Qian")
             .setContactUrl("https://lightapt.com/")

             .setLicenseName("GNU GENERAL PUBLIC LICENSE, Version 3")
             .setLicenseUrl("https://www.gnu.org/licenses/gpl-3.0.en.html")

             .addServer("http://localhost:8000", "server on localhost");

         return builder.build();
     }());

    /**
     *  Swagger-Ui Resources (<oatpp-examples>/lib/oatpp-swagger/res)
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)
    ([]
     {
    // Make sure to specify correct full path to oatpp-swagger/res folder !!!
    return oatpp::swagger::Resources::loadResources("websrc/swagger"); }());
};

#endif /* SwaggerComponent_hpp */