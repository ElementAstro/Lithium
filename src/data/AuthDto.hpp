#ifndef EXAMPLE_JWT_AUTHDTO_HPP
#define EXAMPLE_JWT_AUTHDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class AuthDto : public oatpp::DTO {
  
  DTO_INIT(AuthDto, DTO)

  DTO_FIELD(String, token);

};

#include OATPP_CODEGEN_END(DTO)

#endif /* EXAMPLE_JWT_AUTHDTO_HPP */
