#ifndef LITHIUM_CLIENT_INDI_QHYCCD_HPP
#define LITHIUM_CLIENT_INDI_QHYCCD_HPP

#include "client/indi/camera.hpp"

class QHYCamera : public INDICamera {
public:
    QHYCamera(std::string deviceName);
    ~QHYCamera() override;

    auto watchAdditionalProperty() -> bool override;
};

#endif
