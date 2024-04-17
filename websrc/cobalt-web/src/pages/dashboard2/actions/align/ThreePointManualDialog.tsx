import * as React from "react";
import Slider from "react-slick";
import "slick-carousel/slick/slick.css";
import "slick-carousel/slick/slick-theme.css";
import { Modal, Button } from "react-bootstrap";

interface ThreePointManualDialogProps {
  open_dialog: number;
  handleClose: (flag: boolean) => void;
}

const ThreePointManualDialog: React.FC<ThreePointManualDialogProps> = (
  props
) => {
  // ui control
  const [open, set_open] = React.useState(false);

  const settings = {
    dots: true,
  };

  // open dialog
  React.useEffect(() => {
    if (props.open_dialog > 0) {
      set_open(true);
    }
  }, [props.open_dialog]);

  return (
    <Modal
      show={open}
      onHide={() => {
        props.handleClose(false);
      }}
    >
      <Modal.Body>
        <Slider {...settings}>
          <div>
            <img src="http://placekitten.com/g/400/200" alt="Slide 1" />
          </div>
          <div>
            <img src="http://placekitten.com/g/400/200" alt="Slide 2" />
          </div>
        </Slider>
      </Modal.Body>
      <Modal.Footer>
        <Button
          variant="primary"
          onClick={() => {
            set_open(false);
            props.handleClose(true);
          }}
        >
          不再提示
        </Button>
        <Button
          variant="danger"
          onClick={() => {
            set_open(false);
            props.handleClose(false);
          }}
        >
          关闭
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default ThreePointManualDialog;
