import React, { useState, useEffect } from "react";
import Table from "react-bootstrap/Table";
import Button from "react-bootstrap/Button";
import Modal from "react-bootstrap/Modal";
import Form from "react-bootstrap/Form";
import { useImmer } from "use-immer";
import { PencilSquare, Check } from "react-bootstrap-icons";

interface SingleFilterProps {
  filters_info: ISingleFilterInfo[];
  tooltip?: string;
  handle_single_filter_setting: (
    filter_number: number,
    setting_value: ISingleFilterInfo
  ) => void;
  parameter_explain: IGPParameterExplain;
}

const GlobalParameterFilterSetting: React.FC<SingleFilterProps> = (props) => {
  const [error, setError] = useState(false);
  const [helperText, setHelperText] = useState(props.tooltip);

  const [filter_table_data, set_filter_table_data] = useState<
    ISingleFilterInfo[]
  >([]);
  useEffect(() => {
    if (props.filters_info !== undefined) {
      set_filter_table_data(props.filters_info);
    }
  }, [props.filters_info]);

  const [open_filter_set_dialog, set_open_filter_set_dialog] =
    React.useState<boolean>(false);
  const [to_set_filer_index, set_to_set_filter_index] =
    React.useState<number>(0);
  const [single_filter_dialog_data, set_single_dialog_data] =
    useImmer<ISingleFilterInfo>({
      filter_name: "",
      focus_offset: 0,
      af_exposure_time: 1,
    });
  const [filter_touched, set_filter_touched] = React.useState<boolean>(false);

  const on_click_change_filter = (filter_index: number) => {
    set_filter_touched(false);
    set_to_set_filter_index(filter_index);
    set_single_dialog_data((draft) => {
      draft = props.filters_info[filter_index];
    });
    set_open_filter_set_dialog(true);
  };

  const on_submit_clicked = (filter_index: number) => {
    if (filter_touched) {
      props.handle_single_filter_setting(
        filter_index,
        single_filter_dialog_data
      );
    }
    set_open_filter_set_dialog(false);
  };

  return (
    <div>
      <Table responsive bordered>
        <thead>
          <tr>
            <th>滤镜编号</th>
            <th>滤镜名称</th>
            <th>滤镜对焦偏置值</th>
            <th>自动对焦曝光时间倍率</th>
            <th></th>
          </tr>
        </thead>
        <tbody>
          {filter_table_data.map((single_filter_info, single_filter_index) => (
            <tr key={single_filter_index}>
              <td>{single_filter_index + 1}</td>
              <td>{single_filter_info.filter_name}</td>
              <td>{single_filter_info.focus_offset}</td>
              <td>{single_filter_info.af_exposure_time}</td>
              <td>
                <Button
                  variant="primary"
                  onClick={() => {
                    on_click_change_filter(single_filter_index);
                  }}
                >
                  <PencilSquare />
                </Button>
              </td>
            </tr>
          ))}
        </tbody>
      </Table>

      <Modal
        show={open_filter_set_dialog}
        onHide={() => set_open_filter_set_dialog(false)}
      >
        <Modal.Header closeButton>
          <Modal.Title>修改滤镜编号{to_set_filer_index + 1}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form
            onSubmit={(event: React.FormEvent<HTMLFormElement>) => {
              event.preventDefault();
              set_open_filter_set_dialog(false);
              on_submit_clicked(to_set_filer_index);
            }}
          >
            <Form.Group controlId="filterName">
              <Form.Label>滤镜名称</Form.Label>
              <Form.Control
                type="text"
                value={single_filter_dialog_data.filter_name}
                onChange={(event) => {
                  set_filter_touched(true);
                  set_single_dialog_data((draft) => {
                    draft.filter_name = event.target.value;
                  });
                }}
                isInvalid={error}
              />
            </Form.Group>
            <Form.Group controlId="focusOffset">
              <Form.Label>滤镜对焦偏置值</Form.Label>
              <Form.Control
                type="number"
                value={single_filter_dialog_data.focus_offset}
                onChange={(event) => {
                  set_filter_touched(true);
                  set_single_dialog_data((draft) => {
                    draft.focus_offset = parseInt(event.target.value);
                  });
                }}
                isInvalid={error}
              />
            </Form.Group>
            <Form.Group controlId="afExposureTime">
              <Form.Label>自动对焦曝光时间倍率</Form.Label>
              <Form.Control
                type="number"
                value={single_filter_dialog_data.af_exposure_time}
                min={1}
                max={100}
                step={1}
                onChange={(event) => {
                  set_filter_touched(true);
                  set_single_dialog_data((draft) => {
                    draft.af_exposure_time = parseInt(event.target.value);
                  });
                }}
                isInvalid={error}
              />
            </Form.Group>
            <Button variant="primary" type="submit">
              <Check /> 确认
            </Button>
          </Form>
        </Modal.Body>
      </Modal>
    </div>
  );
};

export default GlobalParameterFilterSetting;
