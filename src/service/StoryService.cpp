
#include "StoryService.hpp"

oatpp::Object<StoryDto> StoryService::storyDtoFromModel(const oatpp::Object<StoryModel>& model) {
  auto dto = StoryDto::createShared();
  dto->id = model->id;
  dto->content = model->content;
  return dto;
}

oatpp::Object<StoryModel> StoryService::storyModelFromDto(const oatpp::String& userId, const oatpp::Object<StoryDto>& dto) {
  auto model = StoryModel::createShared();
  model->userId = userId;
  model->id = dto->id;
  model->content = dto->content;
  return model;
}

oatpp::Object<StoryDto> StoryService::createStory(const oatpp::String& userId, const oatpp::Object<StoryDto>& dto) {

  auto dbResult = m_database->createStory(storyModelFromDto(userId, dto));
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<StoryModel>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown Error");
  return storyDtoFromModel(result[0]);

}

oatpp::Object<StoryDto> StoryService::updateStory(const oatpp::String& userId, const oatpp::Object<StoryDto>& dto) {

  auto dbResult = m_database->updateStory(storyModelFromDto(userId, dto));
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<StoryModel>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown Error");
  return storyDtoFromModel(result[0]);

}

oatpp::Object<StoryDto> StoryService::getStoryByUserIdAndId(const oatpp::String& userId,
                                                            const oatpp::String& id,
                                                            const oatpp::provider::ResourceHandle<oatpp::orm::Connection>& connection)
{

  auto dbResult = m_database->getStoryByUserIdAndId(userId, id, connection);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  OATPP_ASSERT_HTTP(dbResult->hasMoreToFetch(), Status::CODE_404, "User story not found");

  auto result = dbResult->fetch<oatpp::Vector<oatpp::Object<StoryModel>>>();
  OATPP_ASSERT_HTTP(result->size() == 1, Status::CODE_500, "Unknown error");

  return storyDtoFromModel(result[0]);

}

oatpp::Object<PageDto<oatpp::Object<StoryDto>>> StoryService::getAllUserStories(const oatpp::String& userId,
                                                                                const oatpp::UInt32& offset,
                                                                                const oatpp::UInt32& limit)
{

  oatpp::UInt32 countToFetch = limit;

  if(limit > 10) {
    countToFetch = 10;
  }

  auto dbResult = m_database->getAllUserStories(userId, offset, countToFetch);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());

  auto items = dbResult->fetch<oatpp::Vector<oatpp::Object<StoryModel>>>();

  oatpp::Vector<oatpp::Object<StoryDto>> stories({});
  for(auto& item : * items) {
    stories->push_back(storyDtoFromModel(item));
  }

  auto page = PageDto<oatpp::Object<StoryDto>>::createShared();
  page->offset = offset;
  page->limit = countToFetch;
  page->count = stories->size();
  page->items = stories;

  return page;

}

oatpp::Object<StatusDto> StoryService::deleteStoryByUserIdAndId(const oatpp::String& userId, const oatpp::String& id) {
  auto dbResult = m_database->deleteStoryByUserIdAndId(userId, id);
  OATPP_ASSERT_HTTP(dbResult->isSuccess(), Status::CODE_500, dbResult->getErrorMessage());
  auto status = StatusDto::createShared();
  status->status = "OK";
  status->code = 200;
  status->message = "User story was successfully deleted";
  return status;
}