import * as React from "react";
import { GlobalStore } from "../../../store/globalStore";
import Table from "react-bootstrap/Table";
import Button from "react-bootstrap/Button";
import Modal from "react-bootstrap/Modal";
import Form from "react-bootstrap/Form";
import { Card } from "react-bootstrap";

const GPFilterEasyUseComp: React.FC = () => {
  const global_parameter = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore
  );
  const [showFilterSetDialog, setShowFilterSetDialog] =
    React.useState<boolean>(false);
  const [toSetFilterIndex, setToSetFilterIndex] = React.useState<number>(0);
  const [singleFilterDialogData, setSingleDialogData] =
    React.useState<ISingleFilterInfo>({
      filter_name: "",
      focus_offset: 0,
      af_exposure_time: 1,
    });
  const [filterTouched, setFilterTouched] = React.useState<boolean>(false);

  // functions

  const onSubmitClicked = (filterIndex: number) => {
    // if (filterTouched) {
    //   props.handle_single_filter_setting(filterIndex, singleFilterDialogData);
    // }
    setShowFilterSetDialog(false);
  };

  return (
    <>
      {global_parameter.global_parameter.filter_setting === null ? (
        <Card>
          <Card.Body>全局变量加载失败！</Card.Body>
        </Card>
      ) : (
        <div>
          <Table striped bordered hover>
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
              {global_parameter.global_parameter.filter_setting?.filter_info.map(
                (singleFilterInfo, singleFilterIndex) => {
                  return (
                    <tr key={singleFilterIndex}>
                      <td>{singleFilterIndex + 1}</td>
                      <td>{singleFilterInfo.filter_name}</td>
                      <td>{singleFilterInfo.focus_offset}</td>
                      <td>{singleFilterInfo.af_exposure_time}</td>
                      <td>
                        <Button
                          variant="primary"
                          onClick={() => {
                            // onClickChangeFilter(singleFilterIndex);
                          }}
                        >
                          修改
                        </Button>
                      </td>
                    </tr>
                  );
                }
              )}
            </tbody>
          </Table>
        </div>
      )}

      <Modal
        show={showFilterSetDialog}
        onHide={() => setShowFilterSetDialog(false)}
      >
        <Modal.Header closeButton>
          <Modal.Title>修改滤镜编号{toSetFilterIndex + 1}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form
            onSubmit={(event: React.FormEvent<HTMLFormElement>) => {
              event.preventDefault();
              setShowFilterSetDialog(false);
              onSubmitClicked(toSetFilterIndex);
            }}
          >
            <Form.Group controlId="filterName">
              <Form.Label>滤镜名称</Form.Label>
              <Form.Control
                type="text"
                value={singleFilterDialogData.filter_name}
                onChange={(event) => {
                  setFilterTouched(true);
                  setSingleDialogData({
                    ...singleFilterDialogData,
                    filter_name: event.target.value,
                  });
                }}
              />
            </Form.Group>
            <Form.Group controlId="focusOffset">
              <Form.Label>滤镜对焦偏置值</Form.Label>
              <Form.Control
                type="number"
                value={singleFilterDialogData.focus_offset}
                onChange={(event) => {
                  setFilterTouched(true);
                  setSingleDialogData({
                    ...singleFilterDialogData,
                    focus_offset: parseInt(event.target.value),
                  });
                }}
              />
            </Form.Group>
            <Form.Group controlId="afExposureTime">
              <Form.Label>自动对焦曝光时间倍率</Form.Label>
              <Form.Control
                type="number"
                value={singleFilterDialogData.af_exposure_time}
                min={1}
                max={100}
                onChange={(event) => {
                  setFilterTouched(true);
                  setSingleDialogData({
                    ...singleFilterDialogData,
                    af_exposure_time: parseInt(event.target.value),
                  });
                }}
              />
            </Form.Group>
            <Button variant="primary" type="submit">
              确认
            </Button>
          </Form>
        </Modal.Body>
      </Modal>
    </>
  );
};

export default GPFilterEasyUseComp;
