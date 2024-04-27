import * as React from "react";
import { Row, Col } from "react-bootstrap";
import Card from "react-bootstrap/Card";
import {
  ArrowLeft,
  ArrowRight,
  ArrowUp,
  ArrowDown,
} from "react-bootstrap-icons";
import ProgressBar from "react-bootstrap/ProgressBar";

interface ThreePointDisplayProps {
  east_west_angle: number | null;
  up_down_angle: number | null;
  target_radec: ICThreePointTarget | null;
  current_radec: ICThreePointTarget | null;
}

function degToDMS(deg: number) {
  // Get the degrees, minutes, and seconds
  let degrees = Math.floor(deg);
  let minutes = Math.floor((deg - degrees) * 60);
  let seconds = Math.round((deg - degrees - minutes / 60) * 3600);

  // Return the DMS values as a string
  return { degrees, minutes, seconds };
}

const ThreePointDataDisplay: React.FC<ThreePointDisplayProps> = (props) => {
  // ui display data
  const [east_west, set_east_west] = React.useState(false);
  const [up_down, set_up_down] = React.useState(false);
  const [east_west_degree, set_east_west_degree] = React.useState("");
  const [up_down_degree, set_up_down_degree] = React.useState("");
  const [can_show, set_can_show] = React.useState(false);

  React.useEffect(() => {}, []);
  React.useEffect(() => {
    if (props.east_west_angle != null && props.up_down_angle != null) {
      // 注意，在误差标记这块，这里顺时针是正，是东，是true。逆时针的方向，是西，是false。z轴指向天顶。
      // 但是在移动，因为东是顺时针，所以向西移动。
      // 同理，上下，正是朝上，下是朝下，但是修正是如果在上面，要朝下移动。
      // 与后台的算法在确认。
      set_can_show(true);
      if (props.east_west_angle > 0) {
        set_east_west(true);
      } else {
        set_east_west(false);
      }
      let new_east_west = degToDMS(props.east_west_angle);
      set_east_west_degree(
        new_east_west.degrees +
          "°" +
          new_east_west.minutes +
          "'" +
          new_east_west.seconds +
          '"'
      );
      if (props.up_down_angle > 0) {
        set_up_down(true);
      } else {
        set_up_down(false);
      }
      let new_up_down = degToDMS(props.up_down_angle);
      set_up_down_degree(
        new_up_down.degrees +
          "°" +
          new_up_down.minutes +
          "'" +
          new_up_down.seconds +
          '"'
      );
    } else {
      set_can_show(false);
    }
  }, [props.east_west_angle, props.up_down_angle]);
  React.useEffect(() => {}, [props.current_radec]);

  return (
    <>
      <Row>
        <Col xs={6}>
          <Card bg="primary" text="white">
            <Card.Body className="d-flex flex-column align-items-center">
              <div className="position-relative">
                <ProgressBar
                  now={20}
                  style={{
                    width: "100px",
                    height: "100px",
                    borderRadius: "50%",
                  }}
                />
                <div className="position-absolute top-50 start-50 translate-middle">
                  {east_west ? (
                    <ArrowLeft size={24} />
                  ) : (
                    <ArrowRight size={24} />
                  )}
                </div>
              </div>
              <Card.Text className="mt-3">
                {east_west ? "向西移动" : "向东移动"}
              </Card.Text>
              <Card.Title>{east_west_degree}</Card.Title>
            </Card.Body>
          </Card>
        </Col>
        <Col xs={6}>
          <Card bg="primary" text="white">
            <Card.Body className="d-flex flex-column align-items-center">
              <div className="position-relative">
                <ProgressBar
                  now={20}
                  style={{
                    width: "100px",
                    height: "100px",
                    borderRadius: "50%",
                  }}
                />
                <div className="position-absolute top-50 start-50 translate-middle">
                  {up_down ? <ArrowDown size={24} /> : <ArrowUp size={24} />}
                </div>
              </div>
              <Card.Text className="mt-3">
                {up_down ? "向下移动" : "向上移动"}
              </Card.Text>
              <Card.Title>{up_down_degree}</Card.Title>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </>
  );
};

export default ThreePointDataDisplay;
