
#ifndef EXAMPLE_JWT_AUTHINTERCEPTOR_HPP
#define EXAMPLE_JWT_AUTHINTERCEPTOR_HPP

#include "auth/AuthHandler.hpp"

#include "oatpp/web/server/interceptor/RequestInterceptor.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/handler/AuthorizationHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"

class AuthInterceptor : public oatpp::web::server::interceptor::RequestInterceptor {
private:
  AuthHandler m_authHandler;
  oatpp::web::server::HttpRouterTemplate<bool> authEndpoints;
public:

  AuthInterceptor(const std::shared_ptr<JWT>& jwt);

  std::shared_ptr<OutgoingResponse> intercept(const std::shared_ptr<IncomingRequest>& request) override;
};

#endif //EXAMPLE_JWT_AUTHINTERCEPTOR_HPP
