
#include "StarService.hpp"

oatpp::Object<StarDto> StarService::starSearch(const oatpp::Object<StarDto> &dto)
{
    auto dbResult = m_database->selectData(dto);
    OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

    auto userId = oatpp::sqlite::Utils::getLastInsertRowId(dbResult->getConnection());

    return StarDto::createShared();
}
