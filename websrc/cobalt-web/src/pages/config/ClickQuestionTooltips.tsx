import React, { useState } from "react";
import Badge from "react-bootstrap/Badge";
import Button from "react-bootstrap/Button";
import OverlayTrigger from "react-bootstrap/OverlayTrigger";
import Popover from "react-bootstrap/Popover";
import { QuestionCircle } from "react-bootstrap-icons";

interface RedDotProps {
  show_changing: boolean;
  tooltip?: string;
}

const GPRedDotComp: React.FC<RedDotProps> = (props) => {
  const [open, setOpen] = useState(false);

  const handleTooltipClose = () => {
    setOpen(false);
  };

  const handleTooltipOpen = () => {
    setOpen(true);
  };

  if (props.show_changing) {
    return (
      <Badge bg="danger">
        <OverlayTrigger
          trigger="click"
          placement="left"
          overlay={<Popover id="popover-basic">{props.tooltip}</Popover>}
          rootClose
        >
          <Button variant="link" onClick={handleTooltipOpen}>
            <QuestionCircle />
          </Button>
        </OverlayTrigger>
      </Badge>
    );
  } else {
    return (
      <OverlayTrigger
        trigger="click"
        placement="left"
        overlay={<Popover id="popover-basic">{props.tooltip}</Popover>}
        rootClose
      >
        <Button variant="link" onClick={handleTooltipOpen}>
          <QuestionCircle />
        </Button>
      </OverlayTrigger>
    );
  }
};

export default GPRedDotComp;
