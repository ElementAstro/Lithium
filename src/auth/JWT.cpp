
#include "JWT.hpp"

JWT::JWT(const oatpp::String &secret,
		 const oatpp::String &issuer)
	: m_secret(secret), m_issuer(issuer), m_verifier(jwt::verify()
														 .allow_algorithm(jwt::algorithm::hs256{secret})
														 .with_issuer(issuer))
{
}

oatpp::String JWT::createToken(const std::shared_ptr<Payload> &payload)
{
	auto token = jwt::create()
					 .set_issuer(m_issuer)
					 .set_type("JWT")

					 .set_payload_claim("userId", payload->userId)

					 .sign(jwt::algorithm::hs256{m_secret});
	return token;
}

std::shared_ptr<JWT::Payload> JWT::readAndVerifyToken(const oatpp::String &token)
{

	auto decoded = jwt::decode(token);
	m_verifier.verify(decoded);

	auto payload = std::make_shared<Payload>();
	payload->userId = decoded.get_payload_claim("userId").to_json().get<std::string>();

	return payload;
}