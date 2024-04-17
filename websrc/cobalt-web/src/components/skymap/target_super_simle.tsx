import * as React from "react";
import Card from "react-bootstrap/Card";
import Button from "react-bootstrap/Button";
import Row from "react-bootstrap/Row";
import Col from "react-bootstrap/Col";
import Placeholder from "react-bootstrap/Placeholder";
import * as AXIOSOF from "../../services/object_finding_api";

interface TargetSuperSimpleCardProps {
  target_name: string;
  ra: number;
  dec: number;
  target_type: string;
  on_target_selected: (index: number) => void;
  index: number;
}

export const TranslateTargetType = (target_type: string) => {
  /*
      all case types:
      Asterism
      Dark Neb
      Em Neb
      Gal Chain
      Gal Clus
      Gal Group
      Gal-BCD
      Gal-Dwarf
      Gal-Ell
      Gal-Lent
      Gal-Mag
      Galaxy
      Glob Cl
      HH
      HII Neb
      Mol Cld
      Nova
      Open Cl
      PN
      PPN
      Quasar
      Ref Neb
      SNR
      Star
      Star Cld
      WR Neb
      YSO
      */
  switch (target_type) {
    case "Asterism":
      return "星群";
    case "Dark Neb":
      return "暗星云";
    case "Em Neb":
      return "发射星云";
    case "Gal Chain":
      return "星系链";
    case "Gal Clus":
      return "星系团";
    case "Gal Group":
      return "星系群";
    case "Gal-BCD":
      return "致密蓝矮星系";
    case "Gal-Dwarf":
      return "矮星系";
    case "Gal-Ell":
      return "椭圆星系";
    case "Gal-Lent":
      return "透镜状星系";
    case "Gal-Mag":
      return "麦哲伦星系";
    case "Galaxy":
      return "星系";
    case "Glob Cl":
      return "球状星团";
    case "HH":
      return "赫比格-哈罗天体";
    case "HII Neb":
      return "电离氢发射星云";
    case "Mol Cld":
      return "分子云";
    case "Nova":
      return "新星";
    case "Open Cl":
      return "疏散星团";
    case "PN":
      return "行星状星云";
    case "PPN":
      return "原行星云";
    case "Quasar":
      return "类星体";
    case "Ref Neb":
      return "反射星云";
    case "SNR":
      return "超新星遗迹";
    case "Star":
      return "恒星";
    case "Star Cld":
      return "恒星云";
    case "WR Neb":
      return "沃尔夫-拉叶星云";
    case "YSO":
      return "初期恒星体";
    default:
      return "未知";
  }
};

const TargetSuperSimpleCard: React.FC<TargetSuperSimpleCardProps> = (props) => {
  // ui data
  const [target_icon_link, set_target_icon_link] = React.useState("");
  const [display_type, set_display_type] = React.useState("");
  const [updated, set_updated] = React.useState(false);
  const [current_alt, set_current_alt] = React.useState(0);
  const [highest_alt, set_highest_alt] = React.useState(0);
  const [available_time, set_available_time] = React.useState(0);

  const update_simple_data = async () => {
    try {
      let this_result = await AXIOSOF.getSimpleCardInfo(props.ra, props.dec);
      if (this_result.success) {
        set_current_alt(this_result.data.current);
        set_highest_alt(this_result.data.highest);
        set_available_time(this_result.data.available_shoot_time);
      }
      set_updated(true);
    } catch (err) {
      set_updated(true);
    }
  };
  React.useEffect(() => {
    update_simple_data();
    if (process.env.NODE_ENV == "development") {
      set_target_icon_link(
        "/api/file/DSO/DSOObjects/Small/" + props.target_name + ".jpg"
      );
    } else {
      set_target_icon_link(
        "/file/DSO/DSOObjects/Small/" + props.target_name + ".jpg"
      );
    }
    set_display_type(TranslateTargetType(props.target_type));
  }, []);

  const handle_img_error = () => {
    set_target_icon_link("/file/DSO/DSOObjects/Small/default.jpg");
  };

  return (
    <Card style={{ width: "300px" }}>
      <Card.Body>
        <Row>
          <Col xs={3}>
            <Card.Img
              src={target_icon_link}
              onError={handle_img_error}
              alt=""
              style={{ width: "64px", height: "64px", objectFit: "cover" }}
            />
          </Col>
          <Col xs={9}>
            <Card.Subtitle className="mb-2 text-muted">
              {display_type}
            </Card.Subtitle>
            <Card.Title>{props.target_name}</Card.Title>
          </Col>
        </Row>
      </Card.Body>
      <Card.Footer>
        <Row>
          <Col xs={6}>
            <Button
              variant="outline-primary"
              size="sm"
              block
              onClick={() => props.on_target_selected(props.index)}
            >
              选择目标
            </Button>
          </Col>
          <Col xs={6}>
            <Button variant="primary" size="sm" block>
              删除目标
            </Button>
          </Col>
        </Row>
      </Card.Footer>
      <Card.Footer className="bg-light">
        <Row>
          {updated ? (
            <>
              <Col>
                <small className="text-muted">
                  当前{current_alt.toFixed(0)}°
                </small>
              </Col>
              <Col className="border-left border-right">
                <small className="text-muted">
                  最高{highest_alt.toFixed(0)}°
                </small>
              </Col>
              <Col>
                <small className="text-muted">
                  可拍{available_time.toFixed(1)}h
                </small>
              </Col>
            </>
          ) : (
            <Col>
              <Placeholder as={Card.Text} animation="wave">
                <Placeholder xs={12} />
              </Placeholder>
            </Col>
          )}
        </Row>
      </Card.Footer>
    </Card>
  );
};

export default TargetSuperSimpleCard;
