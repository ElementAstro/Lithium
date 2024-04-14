
#include "JWT.hpp"

JWT::JWT(const oatpp::String& secret, const oatpp::String& issuer)
    : m_secret(secret),
      m_issuer(issuer),
      m_verifier(jwt::verify()
                     .allow_algorithm(jwt::algorithm::hs256{secret})
                     .with_issuer(issuer)) {}

oatpp::String JWT::createToken(const std::shared_ptr<Payload>& payload) {
    auto token = jwt::create()
                     .set_issuer(m_issuer)
                     .set_type("JWS")

                     .set_payload_claim("userId", jwt::claim(payload->userId))

                     .sign(jwt::algorithm::hs256{m_secret});
    return token;
}

std::shared_ptr<JWT::Payload> JWT::readAndVerifyToken(
    const oatpp::String& token) {
    auto decoded = jwt::decode(token);
    m_verifier.verify(decoded);

    auto payload = std::make_shared<Payload>();
    payload->userId =
        decoded.get_payload_claim("").to_json().get("userId").to_str();

    return payload;
}