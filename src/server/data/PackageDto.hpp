// src/PackageJsonDto.hpp

#ifndef PackageJsonDto_hpp
#define PackageJsonDto_hpp

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class RepositoryDto : public oatpp::DTO {
    DTO_INIT(RepositoryDto, DTO)

    DTO_FIELD_INFO(type) {
        info->description = "Repository type";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(url) {
        info->description = "Repository URL";
    }
    DTO_FIELD(String, url);
};

class BugsDto : public oatpp::DTO {
    DTO_INIT(BugsDto, DTO)

    DTO_FIELD_INFO(type) {
        info->description = "Bugs type";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(url) {
        info->description = "Where to report bugs";
    }
    DTO_FIELD(String, url);
};

class HomepageDto : public oatpp::DTO {
    DTO_INIT(HomepageDto, DTO)

    DTO_FIELD_INFO(type) {
        info->description = "Homepage type";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(url) {
        info->description = "Homepage URL";
    }
    DTO_FIELD(String, url);
};

class ModuleDto : public oatpp::DTO {
    DTO_INIT(ModuleDto, DTO)

    DTO_FIELD_INFO(name) {
        info->description = "Module name";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(entry) {
        info->description = "Module entry, just the C function name";
    }
    DTO_FIELD(String, entry);
};

class DependenceDto : public oatpp::DTO {
    DTO_INIT(DependenceDto, DTO)

    DTO_FIELD_INFO(name) {
        info->description = "Dependence name";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(version) {
        info->description = "Dependence version, just like npm's version string";
    }
    DTO_FIELD(String, version);
};

class PackageJsonDto : public oatpp::DTO {
    DTO_INIT(PackageJsonDto, DTO)

    DTO_FIELD_INFO(name) {
        info->description = "Package name";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(version) {
        info->description = "Package version";
    }
    DTO_FIELD(String, version);

    DTO_FIELD_INFO(type) {
        info->description = "Package type";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(description) {
        info->description = "Package description";
    }
    DTO_FIELD(String, description);

    DTO_FIELD_INFO(license) {
        info->description = "Package license";
    }
    DTO_FIELD(String, license);

    DTO_FIELD_INFO(author) {
        info->description = "Package author";
    }
    DTO_FIELD(String, author);

    DTO_FIELD_INFO(repository) {
        info->description = "Package repository";
    }
    DTO_FIELD(Object<RepositoryDto>, repository);

    DTO_FIELD_INFO(bugs) {
        info->description = "Package bugs";
    }
    DTO_FIELD(Object<BugsDto>, bugs);

    DTO_FIELD_INFO(homepage) {
        info->description = "Package homepage";
    }
    DTO_FIELD(Object<HomepageDto>, homepage);

    DTO_FIELD_INFO(keywords) {
        info->description = "Package keywords";
    }
    DTO_FIELD(Vector<String>, keywords);

    DTO_FIELD_INFO(scripts) {
        info->description = "Package scripts";
    }
    DTO_FIELD(Fields<String>, scripts);

    DTO_FIELD_INFO(modules) {
        info->description = "Package modules";
    }
    DTO_FIELD(Vector<Object<ModuleDto>>, modules);

    DTO_FIELD_INFO(dependencies) {
        info->description = "Package dependencies";
    }
    DTO_FIELD(List<DependenceDto>, dependencies);
};

#include OATPP_CODEGEN_END(DTO)

#endif // PackageJsonDto_hpp
