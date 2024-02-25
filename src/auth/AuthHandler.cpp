
#include "AuthHandler.hpp"

AuthHandler::AuthHandler(const std::shared_ptr<JWT>& jwt)
  : oatpp::web::server::handler::BearerAuthorizationHandler("API" /* Realm */)
  , m_jwt(jwt)
{}

std::shared_ptr<AuthHandler::AuthorizationObject> AuthHandler::authorize(const oatpp::String& token) {
  return m_jwt->readAndVerifyToken(token);
}