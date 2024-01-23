import React from "react";

import {
  Container,
  Row,
  Col,
  Card,
  Button,
  Alert,
  Form,
  InputGroup,
  Modal,
} from "react-bootstrap";
import { Trash, PencilSquare } from "react-bootstrap-icons";

import { useImmer } from "use-immer";

import { GlobalStore } from "../../store/globalStore";
import * as AXIOSOF from "../../services/object_finding_api";
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
  // const [dso_detailed_list, update_dso_detailed_list] = useImmer<Array<IDSOObjectDetailedInfo | null>>([]);
  // const [dso_detail_updated, update_dso_detail_updated] = useImmer<Array<boolean | null>>([]);
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
  // const one_dso_card_need_update = () => {
  //   if (current_checked != null){
  //     let new_card_info = dso_detailed_list[current_checked];
  //     if (new_card_info != null){
  //       let new_name = target_store.all_saved_targets[current_checked].name;
  //       update_dso_detailed_list((draft) => {
  //         draft[current_checked].name = new_name;
  //       })
  //     }
  //   }
  // };
  // const construct_all_dso_card_data = () => {
  //   let new_dso_detail_list = Array(target_store.all_saved_target_store.all_saved_targets.length);
  //   let new_dos_update_list = Array(target_store.all_saved_target_store.all_saved_targets.length);
  //   for (let i = 0; i < new_dos_update_list.length; i++) {
  //     new_dos_update_list[i] = false;
  //   }
  //   update_dso_detailed_list((draft) => {
  //     draft=new_dso_detail_list
  //   });
  //   update_dso_detail_updated((draft) => {
  //     draft=new_dos_update_list;
  //   });
  //   for( let i = 0; i < target_store.all_saved_target_store.all_saved_targets.length; i++){
  //     construct_framing_info2card_info(target_store.all_saved_targets[i], i);
  //   }
  // };
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
    } else {
      // pop up a warning
      set_popup_text("没有选中任何待拍摄目标！");
      set_popup_dialog(true);
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
  // const construct_framing_info2card_info = async (card_info: IDSOFramingObjectInfo, index: number) => {
  //   try{
  //     const new_curve_data = await AXIOSOF.getTargetALtCurveOnly(card_info.ra, card_info.dec);
  //     if (new_curve_data.success){
  //       let new_target_frame_info: IDSOObjectDetailedInfo = {
  //         name: card_info.name,
  //         ra: card_info.ra,
  //         dec: card_info.dec,
  //         target_type: card_info.target_type,
  //         bmag: card_info.bmag,
  //         vmag: card_info.vmag,
  //         size: card_info.size,
  //         moon_distance: new_curve_data.data.moon_distance,
  //         altitude: new_curve_data.data.altitude,
  //       }
  //       update_dso_detail_updated((draft) => {
  //         draft[index] = true;
  //       });
  //       update_dso_detailed_list((draft) => {
  //         draft[index] = new_target_frame_info;
  //       })
  //     }else{
  //       return null;
  //     }
  //   }
  //   catch (err) {
  //     return null;
  //   }
  // }
  const handle_rename_close = (save: boolean) => {
    if (rename_text != "" && current_checked != null && save) {
      store_target_rename({
        index: current_checked,
        update_string: rename_text,
      });
      // update_dso_detailed_list((draft) => {
      //   draft[current_checked].name = rename_text;
      // })
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
  const handle_tag_selection = (event: SelectChangeEvent<string>) => {
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
    <Container>
      <Row>
        <Col xs={8}>
          {/* objects card */}
          <Container>
            {target_store.all_saved_targets.length === 0 ? (
              <Alert variant="info">拍摄列表中还没有目标</Alert>
            ) : (
              target_store.all_saved_targets.map((one_target_info, index) => (
                <Row key={index} className="my-2">
                  <Col>
                    <TargetSmallCard
                      target_info={one_target_info}
                      on_card_clicked={on_card_checked}
                      card_index={index}
                      on_choice_maken={props.on_choice_maken}
                      in_manage={true}
                    />
                  </Col>
                </Row>
              ))
            )}
          </Container>
        </Col>
        <Col xs={4}>
          {/* controls */}
          <Card>
            <Card.Header>
              <Card.Title>操作选中的目标</Card.Title>
            </Card.Header>
            <Card.Body>
              <Button
                variant="contained"
                onClick={rename_selected_target}
                disabled={current_checked == null}
              >
                重命名
              </Button>
              <Button
                variant="contained"
                onClick={on_focus_center_target_clicked}
                disabled={current_checked == null}
              >
                选中的目标进行构图
              </Button>

              <Button variant="contained" disabled={current_checked == null}>
                增加tag
              </Button>
              <Button
                variant="contained"
                onClick={on_add_flag_clicked}
                disabled={current_checked == null}
              >
                增加flag
              </Button>

              <Button
                variant="contained"
                onClick={on_delete_clicked}
                color="error"
                disabled={current_checked == null}
              >
                删除选中的目标
              </Button>
            </Card.Body>
          </Card>

          <Card className="mt-3">
            <Card.Body>
              <Form>
                <Form.Group>
                  <Form.Label>根据tag筛选</Form.Label>
                  <Form.Control as="select" disabled>
                    <option>-</option>
                    {/* Render tag options */}
                  </Form.Control>
                </Form.Group>
                <Form.Group>
                  <Form.Label>根据flag筛选</Form.Label>
                  <InputGroup>
                    <Form.Control type="text" disabled />
                    <Button variant="primary" disabled>
                      搜索
                    </Button>
                  </InputGroup>
                </Form.Group>
              </Form>
            </Card.Body>
          </Card>
        </Col>
      </Row>

      {/* Rename Text Dialog */}
      <Modal
        show={rename_text_dialog}
        onHide={() => handle_rename_close(false)}
      >
        <Modal.Header closeButton>
          <Modal.Title>重命名该目标</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group>
              <Form.Label>重命名为</Form.Label>
              <Form.Control
                autoFocus
                type="text"
                value={rename_text}
                onChange={(event) => set_rename_text(event.target.value)}
              />
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="primary" onClick={() => handle_rename_close(true)}>
            确认
          </Button>
          <Button
            variant="secondary"
            onClick={() => handle_rename_close(false)}
          >
            取消
          </Button>
        </Modal.Footer>
      </Modal>

      {/* Flag Dialog */}
      <Modal show={flag_dialog} onHide={() => handle_flag_close(false)}>
        <Modal.Header closeButton>
          <Modal.Title>修改flag</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group>
              <Form.Label>Flag</Form.Label>
              <Form.Control
                autoFocus
                type="text"
                value={flag_text}
                onChange={(event) => set_flag_text(event.target.value)}
              />
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="primary" onClick={() => handle_flag_close(true)}>
            确认
          </Button>
          <Button variant="secondary" onClick={() => handle_flag_close(false)}>
            取消
          </Button>
        </Modal.Footer>
      </Modal>

      {/* Tag Dialog */}
      <Modal show={tag_dialog} onHide={handle_tag_close}>
        <Modal.Header closeButton>
          <Modal.Title>修改目标tag</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group>
              <Form.Label>目标tag</Form.Label>
              <Form.Control
                as="select"
                value={tag_value}
                onChange={(event) => set_tag_value(event.target.value)}
              >
                <option>-</option>
                {/* Render tag options */}
              </Form.Control>
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="primary" onClick={handle_tag_close}>
            确认
          </Button>
        </Modal.Footer>
      </Modal>

      {/* Popup Dialog */}
      <Modal
        show={popup_dialog}
        onHide={() => {
          set_popup_dialog(false);
        }}
      >
        <Modal.Header closeButton>
          <Modal.Title>提示</Modal.Title>
        </Modal.Header>
        <Modal.Body>{/* Popup message content */}</Modal.Body>
        <Modal.Footer>
          <Button
            variant="primary"
            onClick={() => {
              set_popup_dialog(false);
            }}
            autoFocus
          >
            关闭
          </Button>
        </Modal.Footer>
      </Modal>
    </Container>
  );
};

ObjectManagement.defaultProps = {
  on_choice_maken: null,
};

export default ObjectManagement;
