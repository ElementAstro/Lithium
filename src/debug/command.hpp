#ifndef LITHIUM_DEBUG_COMMAND_HPP
#define LITHIUM_DEBUG_COMMAND_HPP

#include <string>

void quit();

void loadSharedComponent(const std::string &compoennt_name,
                         const std::string &module_name);
void unloadSharedComponent(const std::string &compoennt_name);
void reloadSharedComponent(const std::string &compoennt_name);
void reloadAllComponents();
void scanComponents(const std::string &path);
void getComponentInfo(const std::string &name);
void getComponentList();
void getEmbedComponentList();

#endif
