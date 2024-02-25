
#ifndef HEAL_AUTH_JWT_AUTHHANDLER_HPP
#define HEAL_AUTH_JWT_AUTHHANDLER_HPP

#include "JWT.hpp"

class AuthHandler : public oatpp::web::server::handler::BearerAuthorizationHandler {
private:
  std::shared_ptr<JWT> m_jwt;
public:

  AuthHandler(const std::shared_ptr<JWT>& jwt);

  std::shared_ptr<AuthorizationObject> authorize(const oatpp::String& token) override;

};

#endif //HEAL_AUTH_JWT_AUTHHANDLER_HPP
