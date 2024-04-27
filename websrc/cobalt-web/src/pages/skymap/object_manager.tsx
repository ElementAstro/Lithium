import * as React from "react";
import { GlobalStore } from "../../store/globalStore";
import {
  Card,
  Button,
  Alert,
  Container,
  Row,
  Col,
  FormControl,
  InputGroup,
} from "react-bootstrap";
import * as AXIOSOF from "../../services/object_finding_api";
import Modal from "react-bootstrap/Modal";
import Form from "react-bootstrap/Form";

import TargetSmallCardHori from "../../components/skymap/target_smallv2";
import TargetSmallCard from "../../components/skymap/target_small";

interface ObjectManagementProps {
  on_choice_maken: (() => void) | null;
}

const ObjectManagement: React.FC<ObjectManagementProps> = (props) => {
  // store handler
  const target_store = GlobalStore.useAppState(
    (state) => state.TargetListStore
  );
  const clear_all_checked =
    GlobalStore.actions.TargetListStore.clear_all_checked;
  const remove_one_target = GlobalStore.actions.TargetListStore.remove_target;
  const save_all_targets = GlobalStore.actions.TargetListStore.save_all_targets;
  const change_saved_focus_target =
    GlobalStore.actions.TargetListStore.change_saved_focus_target;
  const store_target_set_flag =
    GlobalStore.actions.TargetListStore.target_set_flag;
  const store_target_set_tag =
    GlobalStore.actions.TargetListStore.target_set_tag;
  const store_target_rename = GlobalStore.actions.TargetListStore.target_rename;
  const store_check_one_target =
    GlobalStore.actions.TargetListStore.check_one_target;
  // data
  const [current_checked, set_current_checked] = React.useState<number | null>(
    null
  );
  // extra input data
  const [rename_text_dialog, set_rename_text_dialog] = React.useState(false);
  const [rename_text, set_rename_text] = React.useState("");
  const [flag_dialog, set_flag_dialog] = React.useState(false);
  const [flag_text, set_flag_text] = React.useState("");
  const [popup_dialog, set_popup_dialog] = React.useState(false);
  const [popup_text, set_popup_text] = React.useState("");
  const [tag_dialog, set_tag_dialog] = React.useState(false);
  const [tag_value, set_tag_value] = React.useState<string>("");

  // on mount
  React.useEffect(() => {
    // construct_all_dso_card_data();
  }, []);

  // functions
  const on_card_checked = (card_index: number, checked: boolean) => {
    // console.log('card clicked', card_index, checked);
    if (checked) {
      clear_all_checked();
      store_check_one_target(card_index);
      set_current_checked(card_index);
    } else {
      clear_all_checked();
      set_current_checked(null);
    }
  };
  const on_focus_center_target_clicked = () => {
    if (current_checked != null) {
      change_saved_focus_target(current_checked);
      if (props.on_choice_maken != null) {
        props.on_choice_maken();
      }
    }
  };
  const rename_selected_target = () => {
    if (current_checked != null) {
      set_rename_text("");
      set_rename_text_dialog(true);
    } else {
      // pop up a warning
      set_popup_text("没有选中任何待拍摄目标！");
      set_popup_dialog(true);
    }
  };
  const on_add_tag_clicked = () => {
    if (current_checked != null) {
      set_tag_dialog(true);
    } else {
      // pop up a warning
    }
  };
  const on_add_flag_clicked = () => {
    if (current_checked != null) {
      set_flag_text("");
      set_flag_dialog(true);
    } else {
      // pop up a warning
      set_popup_text("没有选中任何待拍摄目标！");
      set_popup_dialog(true);
    }
  };
  const on_delete_clicked = () => {
    if (current_checked != null) {
      if (target_store.all_saved_targets[current_checked].checked) {
        remove_one_target(current_checked);
        save_all_targets();
      } else {
        console.log(
          current_checked,
          " is not checked in store",
          target_store.all_saved_targets[current_checked]
        );
      }
    } else {
      // pop up a warning
      set_popup_text("没有选中任何待拍摄目标！");
      set_popup_dialog(true);
    }
  };
  const handle_rename_close = (save: boolean) => {
    if (rename_text != "" && current_checked != null && save) {
      store_target_rename({
        index: current_checked,
        update_string: rename_text,
      });
    }
    set_rename_text("");
    set_rename_text_dialog(false);
  };
  const handle_flag_close = (save: boolean) => {
    if (flag_text != "" && current_checked != null && save) {
      store_target_set_flag({
        index: current_checked,
        update_string: flag_text,
      });
    }
    set_flag_text("");
    set_flag_dialog(false);
  };
  const handle_tag_selection = (
    event: React.ChangeEvent<HTMLSelectElement>
  ) => {
    set_tag_value(event.target.value);
  };
  const handle_tag_close = () => {
    if (current_checked != null) {
      store_target_set_tag({
        index: current_checked,
        update_string: tag_value,
      });
    }
    set_tag_value("");
    set_tag_dialog(false);
  };

  return (
    <>
      <Container fluid>
        <Row>
          <Col xs={8}>
            {/* objects card */}
            <Row>
              {target_store.all_saved_targets.length == 0 ? (
                <Col xs={12}>
                  <Alert variant="info">拍摄列表中还没有目标</Alert>
                </Col>
              ) : (
                target_store.all_saved_targets.map((one_target_info, index) => {
                  return (
                    <Col xs={12} key={index}>
                      <TargetSmallCard
                        target_info={one_target_info}
                        on_card_clicked={on_card_checked}
                        card_index={index}
                        on_choice_maken={props.on_choice_maken}
                        in_manage={true}
                      />
                    </Col>
                  );
                })
              )}
            </Row>
          </Col>
          <Col xs={4}>
            {/* controls */}
            <Card>
              <Card.Header>
                <h6>操作选中的目标</h6>
              </Card.Header>
              <Card.Body>
                <Button
                  variant="primary"
                  onClick={rename_selected_target}
                  disabled={current_checked == null}
                >
                  重命名
                </Button>{" "}
                <Button
                  variant="primary"
                  onClick={on_focus_center_target_clicked}
                  disabled={current_checked == null}
                >
                  选中的目标进行构图
                </Button>
                <br />
                <br />
                <Button
                  variant="primary"
                  onClick={on_add_tag_clicked}
                  disabled={current_checked == null}
                >
                  增加tag
                </Button>{" "}
                <Button
                  variant="primary"
                  onClick={on_add_flag_clicked}
                  disabled={current_checked == null}
                >
                  增加flag
                </Button>
                <br />
                <br />
                <Button
                  variant="danger"
                  onClick={on_delete_clicked}
                  disabled={current_checked == null}
                >
                  删除选中的目标
                </Button>
              </Card.Body>
            </Card>
            <hr />
            <Card>
              <Card.Body>
                <Button variant="secondary">根据tag筛选</Button>{" "}
                <Button variant="secondary">根据flag筛选</Button>
              </Card.Body>
            </Card>
          </Col>
        </Row>
      </Container>

      <Modal
        show={rename_text_dialog}
        onHide={() => handle_rename_close(false)}
      >
        <Modal.Header closeButton>
          <Modal.Title>重命名该目标</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form.Control
            type="text"
            placeholder="重命名为"
            value={rename_text}
            onChange={(event) => {
              set_rename_text(event.target.value);
            }}
          />
        </Modal.Body>
        <Modal.Footer>
          <Button
            variant="secondary"
            onClick={() => handle_rename_close(false)}
          >
            取消
          </Button>
          <Button variant="primary" onClick={() => handle_rename_close(true)}>
            确认
          </Button>
        </Modal.Footer>
      </Modal>

      <Modal show={flag_dialog} onHide={() => handle_flag_close(false)}>
        <Modal.Header closeButton>
          <Modal.Title>修改flag</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form.Control
            type="text"
            placeholder="flag"
            value={flag_text}
            onChange={(event) => {
              set_flag_text(event.target.value);
            }}
          />
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={() => handle_flag_close(false)}>
            取消
          </Button>
          <Button variant="primary" onClick={() => handle_flag_close(true)}>
            确认
          </Button>
        </Modal.Footer>
      </Modal>

      <Modal show={tag_dialog} onHide={() => handle_tag_close()}>
        <Modal.Header closeButton>
          <Modal.Title>修改目标tag</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form.Select value={tag_value} onChange={handle_tag_selection}>
            <option value="">-</option>
            {target_store.all_tags.map((one_tag, index) => {
              return (
                <option value={one_tag} key={index}>
                  {one_tag}
                </option>
              );
            })}
          </Form.Select>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="primary" onClick={() => handle_tag_close()}>
            确认
          </Button>
        </Modal.Footer>
      </Modal>

      <Modal
        show={popup_dialog}
        onHide={() => {
          set_popup_dialog(false);
        }}
      >
        <Modal.Header closeButton>
          <Modal.Title>提示</Modal.Title>
        </Modal.Header>
        <Modal.Body>{popup_text}</Modal.Body>
        <Modal.Footer>
          <Button
            variant="secondary"
            onClick={() => {
              set_popup_dialog(false);
            }}
          >
            关闭
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

ObjectManagement.defaultProps = {
  on_choice_maken: null,
};

export default ObjectManagement;
