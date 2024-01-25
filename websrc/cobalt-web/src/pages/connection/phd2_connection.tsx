import React, { useState, useEffect } from "react";
import { useTranslation } from "react-i18next";
import {
  Modal,
  Button,
  Form,
  FormControl,
  FormGroup,
  FormLabel,
  Accordion,
  Card,
} from "react-bootstrap";
import { GlobalStore } from "../../store/globalStore";
import { initial } from "lodash";

const Phd2ConfigEditor = ({ open, onClose }) => {
  const { t } = useTranslation();

  const [isAdvancedConfigOpen, setIsAdvancedConfigOpen] = useState(false);

  const [formData, setFormData] = useState(
    GlobalStore.useAppState((state) => state.connect).phd2_config
  );

  const { global_parameter } = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore
  );

  const { device_selections } = GlobalStore.useAppState(
    (state) => state.connect
  );

  useEffect(() => {
    function init() {
      setFormData({
        ...formData,
        pixel_size: global_parameter.guider_camera_info
          ? global_parameter.guider_camera_info.CCD_PIXEL_SIZE
          : 0,
        focal_length: global_parameter.telescope_info
          ? global_parameter.telescope_info.focal_length
          : 0,
      });
    }
    init();
  }, []);

  const handleSubmit = () => {
    // 提交
    GlobalStore.actions.connect.setState({
      phd2_config: formData,
    });
    onClose();
  };

  const handleToggleAdvancedConfig = () => {
    setIsAdvancedConfigOpen((prev) => !prev);
  };

  return (
    <Modal show={open} onHide={onClose}>
      <Modal.Header closeButton>
        <Modal.Title>{t("phd2ConfigEditor.title")}</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <Form>
          <FormGroup>
            <FormLabel>{t("phd2ConfigEditor.name")}</FormLabel>
            <FormControl
              type="text"
              value={formData.name}
              onChange={(e) =>
                setFormData({ ...formData, name: e.target.value })
              }
            />
          </FormGroup>
          <FormGroup>
            <FormLabel>{t("phd2ConfigEditor.cameraDriver")}</FormLabel>
            <FormControl
              type="text"
              disabled
              value={device_selections.guider.device_driver_exec}
              onChange={(e) =>
                setFormData({ ...formData, camera: e.target.value })
              }
            />
          </FormGroup>
          <FormGroup>
            <FormLabel>{t("phd2ConfigEditor.cameraCCD")}</FormLabel>
            <FormControl
              type="text"
              disabled
              value={formData.camera_ccd}
              onChange={(e) =>
                setFormData({ ...formData, camera_ccd: e.target.value })
              }
            />
          </FormGroup>
          <FormGroup>
            <FormLabel>{t("phd2ConfigEditor.pixelSize")}</FormLabel>
            <FormControl
              type="number"
              value={formData.pixel_size}
              onChange={(e) =>
                setFormData({
                  ...formData,
                  pixel_size: Number(e.target.value),
                })
              }
            />
          </FormGroup>
          <FormGroup>
            <FormLabel>{t("phd2ConfigEditor.telescopeDriver")}</FormLabel>
            <FormControl
              type="text"
              disabled
              value={device_selections.telescope.device_driver_exec}
              onChange={(e) =>
                setFormData({ ...formData, telescope: e.target.value })
              }
            />
          </FormGroup>
          <FormGroup>
            <FormLabel>{t("phd2ConfigEditor.telescopeFocalLength")}</FormLabel>
            <FormControl
              type="number"
              disabled
              value={formData.focal_length}
              onChange={(e) =>
                setFormData({
                  ...formData,
                  focal_length: Number(e.target.value),
                })
              }
            />
          </FormGroup>
          <Accordion activeKey={isAdvancedConfigOpen ? "0" : ""}>
            <Card>
              <Accordion.Header as={Card.Header} eventKey="0">
                {t("phd2ConfigEditor.advancedSettings")}
                <Button onClick={handleToggleAdvancedConfig}>
                  {isAdvancedConfigOpen
                    ? t("phd2ConfigEditor.hideAdvancedSettings")
                    : t("phd2ConfigEditor.advancedSettings")}
                </Button>
              </Accordion.Header>
              <Accordion.Collapse eventKey="0">
                <Card.Body>
                  <FormGroup>
                    <FormLabel>
                      {t("phd2ConfigEditor.enableCentroidDetection")}
                    </FormLabel>
                    <FormControl
                      type="checkbox"
                      checked={formData.mass_change_flag}
                      onChange={(e) =>
                        setFormData({
                          ...formData,
                          mass_change_flag: !formData.mass_change_flag,
                        })
                      }
                    />
                  </FormGroup>
                  <FormGroup>
                    <FormLabel>
                      {t("phd2ConfigEditor.centroidDetectionThreshold")}
                    </FormLabel>
                    <FormControl
                      type="number"
                      value={formData.mass_change_threshold}
                      onChange={(e) =>
                        setFormData({
                          ...formData,
                          mass_change_threshold: Number(e.target.value),
                        })
                      }
                    />
                  </FormGroup>
                  <FormGroup>
                    <FormLabel>
                      {t("phd2ConfigEditor.calibrationDuration")}
                    </FormLabel>
                    <FormControl
                      type="number"
                      value={formData.calibration_duration}
                      onChange={(e) =>
                        setFormData({
                          ...formData,
                          calibration_duration: Number(e.target.value),
                        })
                      }
                    />
                  </FormGroup>
                  <FormGroup>
                    <FormLabel>
                      {t("phd2ConfigEditor.calibrationDistance")}
                    </FormLabel>
                    <FormControl
                      type="number"
                      value={formData.calibration_distance}
                      onChange={(e) =>
                        setFormData({
                          ...formData,
                          calibration_distance: Number(e.target.value),
                        })
                      }
                    />
                  </FormGroup>
                </Card.Body>
              </Accordion.Collapse>
            </Card>
          </Accordion>
        </Form>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={onClose}>
          {t("phd2ConfigEditor.cancel")}
        </Button>
        <Button variant="primary" onClick={handleSubmit}>
          {t("phd2ConfigEditor.submit")}
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default Phd2ConfigEditor;
