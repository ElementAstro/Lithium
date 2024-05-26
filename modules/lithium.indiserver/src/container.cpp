#include "container.hpp"

INDIDeviceContainer::INDIDeviceContainer(const std::string &name, const std::string &label, const std::string &version,
                                         const std::string &binary, const std::string &family,
                                         const std::string &skeleton, bool custom)
    : name(name), label(label), version(version), binary(binary),
      family(family), skeleton(skeleton), custom(custom) {}
