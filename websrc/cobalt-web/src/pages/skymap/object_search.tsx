import * as React from "react";
import {
  Container,
  Row,
  Col,
  Form,
  Button,
  Collapse,
  ListGroup,
  Alert,
} from "react-bootstrap";
import { useImmer } from "use-immer";
import { Filter } from "react-bootstrap-icons";
import * as AXIOSOF from "../../services/object_finding_api";

import TargetSmallCardHori from "../../components/skymap/target_smallv2";
import TargetSmallCard from "../../components/skymap/target_small";

interface ObjectSearchProps {
  on_choice_maken: (() => void) | null;
}

const ObjectSearch: React.FC<ObjectSearchProps> = (props) => {
  // ui control
  const [expand_filter, set_expand_filter] = React.useState(false);
  const [alert_variant, set_alert_variant] = React.useState<
    "info" | "warning" | "danger"
  >("info");
  const [alert_text, set_alert_text] =
    React.useState("请在左侧输入框输入需要搜索的信息");
  // data
  const [to_search_text, set_to_search_text] = React.useState("-");
  const [found_target_result, set_found_target_result] = React.useState<
    Array<IDSOObjectDetailedInfo>
  >([]);
  const [filter_settings, update_filter_settings] = useImmer({
    angular_size: 1, // in arc degree
  });

  const handleFilterExpandClick = () => {
    set_expand_filter(!expand_filter);
  };

  const handleSearchTextOnBlur = async () => {
    if (to_search_text === "-") {
      return null;
    }
    set_found_target_result([]);
    set_alert_variant("info");
    set_alert_text("查询中");
    try {
      const found_targets = await AXIOSOF.findTargetByName(to_search_text);
      console.log(found_targets);
      if (found_targets.success) {
        if (found_targets.data.length > 0) {
          set_found_target_result(found_targets.data);
        } else {
          set_alert_variant("warning");
          set_alert_text("没有找到相关目标");
        }
      } else {
        set_alert_variant("danger");
        set_alert_text("没有找到相关目标!!!");
      }
    } catch (err) {
      return null;
    }
  };

  return (
    <Container fluid>
      <Row>
        <Col xs={3}>
          {/* search and filter part */}
          <Form.Group controlId="searchInput" onBlur={handleSearchTextOnBlur}>
            <Form.Control
              type="text"
              placeholder="输入搜索关键词"
              value={to_search_text}
              onChange={(event) => set_to_search_text(event.target.value)}
              className="mt-3"
            />
          </Form.Group>
          <ListGroup>
            <ListGroup.Item action onClick={handleFilterExpandClick}>
              <Filter className="mr-2" />
              高级搜索
              {expand_filter ? (
                <span className="ml-2">▲</span>
              ) : (
                <span className="ml-2">▼</span>
              )}
            </ListGroup.Item>
            <Collapse in={expand_filter}>
              <ListGroup.Item>
                <h6>开发中</h6>
              </ListGroup.Item>
            </Collapse>
          </ListGroup>
        </Col>
        <Col xs={9}>
          {/* display cards part */}
          <div style={{ overflowY: "auto", overflowX: "hidden" }}>
            {found_target_result.length === 0 ? (
              <Alert variant={alert_variant}>{alert_text}</Alert>
            ) : (
              <Row>
                {found_target_result.map((one_dso_target_info, index) => {
                  return (
                    <Col xs={12} key={index}>
                      <TargetSmallCard
                        target_info={one_dso_target_info}
                        on_card_clicked={null}
                        card_index={index}
                        on_choice_maken={props.on_choice_maken}
                      />
                    </Col>
                  );
                })}
              </Row>
            )}
          </div>
        </Col>
      </Row>
    </Container>
  );
};

ObjectSearch.defaultProps = {
  on_choice_maken: null,
};

export default ObjectSearch;
