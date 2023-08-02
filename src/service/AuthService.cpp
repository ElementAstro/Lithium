
#include "AuthService.hpp"

oatpp::Object<AuthDto> AuthService::signUp(const oatpp::Object<SignUpDto> &dto)
{

    auto user = UserDto::createShared();
    user->id = nullptr;
    user->userName = dto->userName;
    user->email = dto->email;
    user->password = dto->password;
    auto dbResult = m_database->createUser(user);
    if (!dbResult->isSuccess())
    {
        OATPP_LOGE("AuthService", "DB-Error: '%s'", dbResult->getErrorMessage()->c_str());
    }
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_401, "Unauthorized");

    auto result = dbResult->fetch<oatpp::Vector<oatpp::Vector<oatpp::String>>>();
    OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_401, "Unauthorized")

    auto newUserId = result[0][0];

    auto payload = std::make_shared<JWT::Payload>();
    payload->userId = newUserId;

    auto auth = AuthDto::createShared();
    auth->token = m_jwt->createToken(payload);

    return auth;
}

oatpp::Object<AuthDto> AuthService::signIn(const oatpp::Object<SignInDto> &dto)
{

    auto dbResult = m_database->authenticateUser(dto->userName, dto->password);
    if (!dbResult->isSuccess())
    {
        OATPP_LOGE("AuthService", "DB-Error: '%s'", dbResult->getErrorMessage()->c_str());
    }
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_401, "Unauthorized")

    auto result = dbResult->fetch<oatpp::Vector<oatpp::Vector<oatpp::String>>>();
    OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_401, "Unauthorized")

    auto userId = result[0][0];

    auto payload = std::make_shared<JWT::Payload>();
    payload->userId = userId;

    auto auth = AuthDto::createShared();
    auth->token = m_jwt->createToken(payload);

    return auth;
}

oatpp::Object<StatusDto> AuthService::deleteUserById(const oatpp::String &userId)
{
    auto dbResult = m_database->deleteUserById(userId);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
    auto status = StatusDto::createShared();
    status->status = "OK";
    status->code = 200;
    status->message = "User was successfully deleted";
    return status;
}