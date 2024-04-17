import * as React from "react";
import { ListGroup, Card, Container, Row, Col } from "react-bootstrap";
import { Modal } from "react-bootstrap";

interface LightStarSelectionProps {
  open: boolean;
  handleClose: () => void;
  all_star_lists: [
    Array<ILightStarInfo>,
    Array<ILightStarInfo>,
    Array<ILightStarInfo>,
    Array<ILightStarInfo>
  ];
  on_star_selected: (star_area_index: number, star_array_index: number) => void;
}

const LightStartSelection: React.FC<LightStarSelectionProps> = (props) => {
  const area_categoary = ["东天区", "西天区", "南天区", "北天区"];

  return (
    <Modal show={props.open} onHide={props.handleClose} size="lg">
      <Modal.Header closeButton>
        <Modal.Title>亮星选择</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <Container>
          <Row>
            {area_categoary.map((categoryName, categoryIndex) => (
              <Col xs={3} key={categoryIndex}>
                <Card>
                  <Card.Header>{categoryName}</Card.Header>
                  <ListGroup variant="flush">
                    {props.all_star_lists[categoryIndex].map(
                      (star_info, index) => (
                        <ListGroup.Item
                          key={index}
                          action
                          onClick={() => {
                            props.on_star_selected(categoryIndex, index);
                          }}
                        >
                          {star_info.show_name}: 高度角{" "}
                          {star_info.alt.toFixed(1)}°
                        </ListGroup.Item>
                      )
                    )}
                  </ListGroup>
                </Card>
              </Col>
            ))}
          </Row>
        </Container>
      </Modal.Body>
    </Modal>
  );
};

export default LightStartSelection;
