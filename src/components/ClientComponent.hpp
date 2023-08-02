#ifndef ClientComponent_hpp
#define ClientComponent_hpp

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/core/macro/component.hpp"

class ClientComponent
{
public:
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, clientResources)
    ([]
     {
    return oatpp::swagger::Resources::loadResources("websrc/client"); }());
};

#endif /* ClientComponent_hpp */