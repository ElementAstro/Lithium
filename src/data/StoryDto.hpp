#ifndef EXAMPLE_JWT_STORYDTO_HPP
#define EXAMPLE_JWT_STORYDTO_HPP

#include "PageDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class StoryDto : public oatpp::DTO {

  DTO_INIT(StoryDto, DTO)

  DTO_FIELD(String, id);
  DTO_FIELD(String, content);

};

class StoryPageDto : public PageDto<oatpp::Object<StoryDto>> {

  DTO_INIT(StoryPageDto, PageDto<oatpp::Object<StoryDto>>)

};

#include OATPP_CODEGEN_END(DTO)


#endif //EXAMPLE_JWT_STORYDTO_HPP
