import * as React from "react";
import { QuestionCircle } from "react-bootstrap-icons";
import Badge from "react-bootstrap/Badge";
import Button from "react-bootstrap/Button";
import OverlayTrigger from "react-bootstrap/OverlayTrigger";
import Tooltip from "react-bootstrap/Tooltip";

interface RedDotProps {
  show_changing: boolean;
  tooltip?: string;
}

const GPRedDotComp: React.FC<RedDotProps> = (props) => {
  const renderTooltip = (tooltip: string) => (
    <Tooltip id="button-tooltip">{tooltip}</Tooltip>
  );

  if (props.show_changing) {
    return (
      <Badge className="danger" pill>
        <OverlayTrigger
          placement="left"
          overlay={renderTooltip(props.tooltip || "")}
        >
          <Button variant="link">
            <QuestionCircle />
          </Button>
        </OverlayTrigger>
      </Badge>
    );
  } else {
    return (
      <OverlayTrigger
        placement="left"
        overlay={renderTooltip(props.tooltip || "")}
      >
        <Button variant="link">
          <QuestionCircle />
        </Button>
      </OverlayTrigger>
    );
  }
};

export default GPRedDotComp;
