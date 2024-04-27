import * as React from "react";
import {
  Modal,
  Button,
  Form,
  FormControl,
  FormLabel,
  FormText,
  FormGroup,
} from "react-bootstrap";

interface ThreePointSettingDialogProps {
  open_dialog: number;
  direction_switch: boolean;
  move_time: number;
  solve_retry_time: number;
  manual_start: boolean;
  search_radius: number;
  on_update_data: (
    direction_switch: boolean,
    move_time: number,
    solve_retry_time: number,
    manual_start: boolean,
    search_radius: number
  ) => void;
}

const ThreePointSettingDialog: React.FC<ThreePointSettingDialogProps> = (
  props
) => {
  // ui control
  const [open, set_open] = React.useState(false);

  // input data
  const [direction_switch, set_direction_switch] = React.useState(false);
  const [move_time, set_move_time] = React.useState(10);
  const [solve_retry_time, set_solve_retry_time] = React.useState(3);
  const [manual_start, set_manual_start] = React.useState(true);
  const [search_radius, set_search_radius] = React.useState(30);

  // useEffect
  // on mount
  React.useEffect(() => {
    set_direction_switch(props.direction_switch);
    set_move_time(props.move_time);
    set_solve_retry_time(props.solve_retry_time);
    set_manual_start(props.manual_start);
    set_search_radius(props.search_radius);
  }, []);
  // open dialog
  React.useEffect(() => {
    if (props.open_dialog > 0) {
      set_open(true);
    }
  }, [props.open_dialog]);

  // functions
  const handleClose = (save_flag: boolean) => {
    if (save_flag) {
      props.on_update_data(
        direction_switch,
        move_time,
        solve_retry_time,
        manual_start,
        search_radius
      );
    }
    set_open(false);
  };

  return (
    <Modal
      show={open}
      onHide={() => {
        handleClose(true);
      }}
    >
      <Modal.Header closeButton>
        <Modal.Title>对极轴初始化设置</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <p>
          注意，为了比较准确的进行极轴校准，请务必进行至少2次以上的极轴校准流程。
          如果没有连接赤道仪和相机，是无法进行本流程的！
        </p>
        <p>
          对极轴过程中会进行拍摄解析，解析的相关设置请到全局变量中进行设置。
        </p>
        <p>开始本流程前，请先进行粗对极轴，并且确认赤道仪的坐标信息正确</p>
        <Form>
          <Form.Check
            type="switch"
            id="direction-switch"
            label={direction_switch ? "望远镜朝东" : "望远镜朝西"}
            checked={direction_switch}
            onChange={() => {
              set_direction_switch(!direction_switch);
            }}
          />
          <FormGroup>
            <FormLabel>赤道仪移动时间</FormLabel>
            <FormControl
              type="number"
              value={move_time}
              onChange={(event) => {
                let new_value = parseInt(event.target.value);
                if (new_value < 3) {
                  new_value = 3;
                } else if (new_value > 20) {
                  new_value = 20;
                }
                set_move_time(new_value);
              }}
            />
            <FormText>单位：秒</FormText>
          </FormGroup>
          <FormGroup>
            <FormLabel>解析失败重试次数</FormLabel>
            <FormControl
              type="number"
              value={solve_retry_time}
              onChange={(event) => {
                let new_value = parseInt(event.target.value);
                if (new_value < 10) {
                  new_value = 10;
                } else if (new_value > 180) {
                  new_value = 180;
                }
                set_solve_retry_time(new_value);
              }}
            />
            <FormText>单位：次</FormText>
          </FormGroup>
          <Form.Check
            type="switch"
            id="manual-start"
            label={manual_start ? "从当前位置开始" : "自动选择开始位置"}
            checked={manual_start}
            onChange={() => {
              set_manual_start(!manual_start);
            }}
          />
          <FormGroup>
            <FormLabel>解析搜索半径</FormLabel>
            <FormControl
              type="number"
              value={search_radius}
              onChange={(event) => {
                set_search_radius(parseInt(event.target.value));
              }}
            />
            <FormText>单位：度</FormText>
          </FormGroup>
        </Form>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="primary" onClick={() => handleClose(true)}>
          确认修改
        </Button>
        <Button variant="danger" onClick={() => handleClose(false)}>
          关闭
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default ThreePointSettingDialog;
