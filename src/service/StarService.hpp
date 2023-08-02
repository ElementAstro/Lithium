
#ifndef STARSERVICE_HPP
#define STARSERVICE_HPP

#include "data/StatusDto.hpp"
#include "data/StarDto.hpp"

#include "database/StarDb.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/provider/Provider.hpp"

class StarService {
private:
  typedef oatpp::web::protocol::http::Status Status;
private:
  OATPP_COMPONENT(std::shared_ptr<StarDb>, m_database); // Inject database component
public:

  oatpp::Object<StarDto> starSearch(const oatpp::Object<StarDto>& dto);

};

#endif //STARSERVICE_HPP
