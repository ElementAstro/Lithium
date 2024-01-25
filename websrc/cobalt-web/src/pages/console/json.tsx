import React, { useEffect, useState } from "react";
import { Card, Container } from "react-bootstrap";
import { useTranslation } from "react-i18next";

import { ScrollablePaper } from "./style";

interface DebugJsonDisplayerProps {
  title: string;
  json_struct: Array<any>;
  update: number;
}

interface SingleTypoJsonProps {
  json_struct: any;
}

const SingleTypoJson: React.FC<SingleTypoJsonProps> = (props) => {
  const { t } = useTranslation();
  const [showDanger, setShowDanger] = useState(false);

  useEffect(() => {
    if (props.json_struct.type === "error") {
      setShowDanger(true);
    }
  }, []);

  return (
    <Container>
      <Card className="mb-3" bg={showDanger ? "danger" : "light"}>
        <Card.Body>
          <Card.Title>{props.json_struct.device_name}</Card.Title>
          <Card.Subtitle className="mb-2 text-muted">
            {props.json_struct.instruction}
          </Card.Subtitle>
          <Card.Text>{props.json_struct.message}</Card.Text>
          {props.json_struct.type === "step" && (
            <Card.Text>{JSON.stringify(props.json_struct.data)}</Card.Text>
          )}
        </Card.Body>
      </Card>
    </Container>
  );
};

const DebugWebsocketJsonDisplayer: React.FC<DebugJsonDisplayerProps> = (
  props
) => {
  return (
    <ScrollablePaper>
      <Container style={{ height: 300 }}>
        <Card>
          <Card.Header>{props.title}</Card.Header>
          <Card.Body>
            {props.json_struct.map((single_json, index) => {
              return <SingleTypoJson key={index} json_struct={single_json} />;
            })}
          </Card.Body>
        </Card>
      </Container>
    </ScrollablePaper>
  );
};

export default DebugWebsocketJsonDisplayer;
