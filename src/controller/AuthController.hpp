#ifndef EXAMPLE_JWT_AUTHCONTROLLER_HPP
#define EXAMPLE_JWT_AUTHCONTROLLER_HPP

#include "service/AuthService.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

/**
 * User REST controller.
 */
class AuthController : public oatpp::web::server::api::ApiController {
public:
  AuthController(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
private:
  AuthService m_userService; // Create user service.
public:

  static std::shared_ptr<AuthController> createShared(
    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
  ){
    return std::make_shared<AuthController>(objectMapper);
  }

  ENDPOINT_INFO(signUp) {
    info->summary = "Sign up";

    info->addConsumes<Object<SignUpDto>> ("application/json");

    info->addResponse<Object<AuthDto>> (Status::CODE_200, "application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
  }
  ENDPOINT("POST", "users/signup", signUp,
           BODY_DTO(Object<SignUpDto>, dto))
  {
    return createDtoResponse(Status::CODE_200, m_userService.signUp(dto));
  }

  ENDPOINT_INFO(signIn) {
    info->summary = "Sign in";

    info->addConsumes <Object<SignInDto>> ("application/json");

    info->addResponse <Object<AuthDto>> (Status::CODE_200, "application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
  }
  ENDPOINT("POST", "users/signin", signIn,
           BODY_DTO(Object<SignInDto>, dto))
  {
    return createDtoResponse(Status::CODE_200, m_userService.signIn(dto));
  }

  ENDPOINT_INFO(deleteUser) {
    info->summary = "Delete User";

    info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");
  }
  ENDPOINT("DELETE", "users", deleteUser,
           BUNDLE(String, userId))
  {
    return createDtoResponse(Status::CODE_200, m_userService.deleteUserById(userId));
  }

};

#include OATPP_CODEGEN_BEGIN(ApiController) //<- End Codegen

#endif //EXAMPLE_JWT_AUTHCONTROLLER_HPP
