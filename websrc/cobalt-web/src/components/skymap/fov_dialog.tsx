import React from "react";
import { useState, useEffect } from "react";
import { Modal, Button, Form, Row, Col, InputGroup } from "react-bootstrap";
import { Person, Arrow90degDown, Arrow90degRight } from "react-bootstrap-icons";
import { useTranslation } from "react-i18next";

interface FovDataType {
  x_pixels: number;
  x_pixel_size: number;
  y_pixels: number;
  y_pixel_size: number;
  focal_length: number;
}

interface FOVDialogProps {
  fov_data: FovDataType;
  rotation: number;
  open_dialog: number;
  on_fov_change: (fov_data: FovDataType) => void;
  on_rotation_change: (rotation: number) => void;
}

const FOVSettingDialog: React.FC<FOVDialogProps> = (props) => {
  const { t } = useTranslation();
  const [open, set_open] = useState(false);
  const [x_pixels, set_x_pixels] = useState(0);
  const [x_pixels_size, set_x_pixels_size] = useState(0);
  const [y_pixels, set_y_pixels] = useState(0);
  const [y_pixel_size, set_y_pixels_size] = useState(0);
  const [focal_length, set_focal_length] = useState(0);
  const [rotation, set_rotation] = useState(0);

  const [x_tiles, set_x_tiles] = useState(1);
  const [y_tiles, set_y_tiles] = useState(1);
  const [overlap, set_overlap] = useState(20); // in percent

  const [modified, set_modified] = useState(false);

  const rotation_marks = [
    {
      value: 0,
      label: "0°",
    },
    {
      value: 90,
      label: "90°",
    },
    {
      value: 180,
      label: "180°",
    },
    {
      value: 270,
      label: "270°",
    },
  ];

  useEffect(() => {
    set_x_pixels(props.fov_data.x_pixels);
    set_y_pixels(props.fov_data.y_pixels);
    set_x_pixels_size(props.fov_data.x_pixel_size);
    set_y_pixels_size(props.fov_data.y_pixel_size);
    set_focal_length(props.fov_data.focal_length);
    set_rotation(props.rotation);
  }, []);

  useEffect(() => {
    set_x_pixels(props.fov_data.x_pixels);
    set_y_pixels(props.fov_data.y_pixels);
    set_x_pixels_size(props.fov_data.x_pixel_size);
    set_y_pixels_size(props.fov_data.y_pixel_size);
    set_focal_length(props.fov_data.focal_length);
  }, [props.fov_data]);

  useEffect(() => {
    set_rotation(props.rotation);
  }, [props.rotation]);

  useEffect(() => {
    if (props.open_dialog > 0) {
      set_open(true);
    }
  }, [props.open_dialog]);

  const handleClose = (update_flag: boolean = false) => {
    if (update_flag) {
      let new_fov_data: FovDataType = {
        x_pixels: x_pixels,
        x_pixel_size: x_pixels_size,
        y_pixels: y_pixels,
        y_pixel_size: y_pixel_size,
        focal_length: focal_length,
      };
      props.on_fov_change(new_fov_data);
      console.log("in dialog, changing fov", new_fov_data);
      props.on_rotation_change(rotation);
    } else {
      if (modified) {
        // jump一个提示
      }
    }
    set_open(false);
  };

  const update_fove_modified = () => {
    set_modified(true);
  };

  const handleSliderChange = (newValue: number) => {
    set_rotation(newValue);
  };

  return (
    <>
      <Modal show={open} onHide={() => handleClose(false)}>
        <Modal.Header closeButton>
          <Modal.Title>视角设置</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>
            默认数据都是根据全局变量的参数自动获取的。如果需要查看不同光学配置下的视场大小，可以通过以下参数进行修改。
            注意，计算出的视场大小仅供参考，因为光学畸变以及光学系统本身对的限制，实际有效的拍摄范围并不一定是计算出的结果！
          </p>
          <Form>
            <Row className="mb-3">
              <Form.Group as={Col}>
                <Form.Label>相机x方向的像素数量</Form.Label>
                <InputGroup>
                  <Form.Control
                    type="number"
                    value={x_pixels}
                    onChange={(event) =>
                      set_x_pixels(parseInt(event.target.value))
                    }
                  />
                  <InputGroup.Text>个</InputGroup.Text>
                </InputGroup>
              </Form.Group>
              <Form.Group as={Col}>
                <Form.Label>相机x方向的像素大小</Form.Label>
                <InputGroup>
                  <Form.Control
                    type="number"
                    value={x_pixels_size}
                    onChange={(event) =>
                      set_x_pixels_size(parseFloat(event.target.value))
                    }
                  />
                  <InputGroup.Text>um</InputGroup.Text>
                </InputGroup>
              </Form.Group>
            </Row>
            <Row className="mb-3">
              <Form.Group as={Col}>
                <Form.Label>相机y方向的像素数量</Form.Label>
                <InputGroup>
                  <Form.Control
                    type="number"
                    value={y_pixels}
                    onChange={(event) =>
                      set_y_pixels(parseInt(event.target.value))
                    }
                  />
                  <InputGroup.Text>个</InputGroup.Text>
                </InputGroup>
              </Form.Group>
              <Form.Group as={Col}>
                <Form.Label>相机y方向的像素大小</Form.Label>
                <InputGroup>
                  <Form.Control
                    type="number"
                    value={y_pixel_size}
                    onChange={(event) =>
                      set_y_pixels_size(parseFloat(event.target.value))
                    }
                  />
                  <InputGroup.Text>um</InputGroup.Text>
                </InputGroup>
              </Form.Group>
            </Row>
            <Row className="mb-3">
              <Form.Group as={Col}>
                <Form.Label>望远镜焦距</Form.Label>
                <InputGroup>
                  <Form.Control
                    type="number"
                    value={focal_length}
                    onChange={(event) =>
                      set_focal_length(parseFloat(event.target.value))
                    }
                  />
                  <InputGroup.Text>mm</InputGroup.Text>
                </InputGroup>
              </Form.Group>
            </Row>
            <Row className="mb-3">
              <Form.Group as={Col}>
                <Button variant="primary">解析获取视场角度</Button>
              </Form.Group>
            </Row>
            <Row className="mb-3">
              <Form.Group as={Col} xs={2}>
                <Form.Label>相机旋转角度</Form.Label>
              </Form.Group>
              <Form.Group as={Col} xs={8}>
                <Form.Range
                  min={0}
                  max={360}
                  step={1}
                  value={rotation}
                  onChange={(event) =>
                    handleSliderChange(parseInt(event.target.value))
                  }
                />
              </Form.Group>
              <Form.Group as={Col} xs={2}>
                <Form.Label>这是要不要输入？</Form.Label>
              </Form.Group>
            </Row>
            <Row className="mb-3">
              <Form.Group as={Col}>
                {/* Add more form controls here */}
              </Form.Group>
            </Row>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="primary" onClick={() => handleClose(true)}>
            确认修改
          </Button>
          <Button variant="secondary" onClick={() => handleClose(false)}>
            关闭
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

export default FOVSettingDialog;
