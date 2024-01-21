import React from "react";
import { DragDropContext, Droppable, Draggable } from "react-beautiful-dnd";
import "./style.less";
import Container from "react-bootstrap/Container";
import Row from "react-bootstrap/Row";
import Col from "react-bootstrap/Col";
import Button from "react-bootstrap/Button";

import { BoxArrowInDownRight } from "react-bootstrap-icons";

// 测试用数据
const DEVICES_ITEMS = ["1", "2", "3", "4", "5", "6", "7", "8", "9"];

const SELECT_ITEMS = ["", "", "", "", "", ""];

const Helper = () => {
  const [init_items, setInitItems] = React.useState(DEVICES_ITEMS);
  const [select_items, setSelectItems] = React.useState(SELECT_ITEMS);

  // 组件结束拖动，如果是卡片区到卡片槽，则更新两块，否则不处理
  const onDragEnd = (result) => {
    const { source, destination } = result;
    if (!destination) {
      return;
    }
    if (source.droppableId !== destination.droppableId) {
      const removed = init_items.splice(source.index, 1)[0];
      const recover = select_items.splice(destination.index, 1, removed)[0];
      if (recover !== "") {
        init_items.push(recover);
        init_items.sort();
      }
      setSelectItems([...select_items]);
      setInitItems([...init_items]);
    }
  };

  const onDragStart = () => {
    console.log("is dragging!");
  };

  return (
    <DragDropContext onDragEnd={onDragEnd} onDragStart={onDragStart}>
      <Container fluid>
        <Row>
          <Col xs={6}>
            {/* 卡片槽部分 */}
            <Row>
              {select_items.map((item, index) => (
                <Col xs={6} key={index}>
                  {/* 针对每一种可能的brand，建立一个droppable */}
                  <Droppable
                    droppableId={`select-droppable-${index}`}
                    key={`select-droppable-${index}`}
                  >
                    {(provided, snapshot) => (
                      // 背景 样式
                      <div
                        {...provided.droppableProps}
                        ref={provided.innerRef}
                        style={{
                          width: "80%",
                          height: "100px",
                          padding: "8px",
                          backgroundColor: "#E5E7EB",
                          display: "grid",
                          gridTemplateColumns: "1fr 1fr",
                          gap: "8px",
                        }}
                      >
                        <Draggable
                          draggableId={`select-draggable-${index}`}
                          index={index}
                          key={`select-draggable-${index}`}
                          isDragDisabled
                        >
                          {(provided, snapshot) => (
                            // 卡片槽样式，注意组件一定要position: absolute
                            <div
                              ref={provided.innerRef}
                              {...provided.draggableProps}
                              {...provided.dragHandleProps}
                              style={{ position: "relative", width: "15%" }}
                            >
                              <div
                                style={{
                                  position: "absolute",
                                  margin: "8px",
                                  width: "100%",
                                  backgroundColor: "#111827",
                                  padding: "8px",
                                }}
                              >
                                <div
                                  style={{
                                    display: "flex",
                                    justifyContent: "space-between",
                                    alignItems: "center",
                                  }}
                                >
                                  <span style={{ color: "#F9FAFB" }}>
                                    {item}
                                  </span>
                                  <Button
                                    variant="outline-light"
                                    size="sm"
                                    onClick={() => {
                                      if (select_items[index] !== "") {
                                        init_items.push(select_items[index]);
                                        select_items[index] = "";
                                        init_items.sort();
                                        setSelectItems([...select_items]);
                                        setInitItems([...init_items]);
                                      }
                                    }}
                                  >
                                    清空
                                  </Button>
                                </div>
                              </div>
                            </div>
                          )}
                        </Draggable>
                        {provided.placeholder}
                      </div>
                    )}
                  </Droppable>
                </Col>
              ))}
            </Row>
          </Col>
          <Col xs={6}>
            {/* 卡片池 */}
            <Droppable droppableId="init_droppable">
              {(provided: { droppableProps }, snapshot) => (
                <div
                  style={{
                    width: "100%",
                    padding: "8px",
                    backgroundColor: "#F9FAFB",
                    display: "grid",
                    gridTemplateColumns: "1fr 1fr",
                    gap: "8px",
                  }}
                  {...provided.droppableProps}
                  ref={provided.innerRef}
                >
                  {init_items.map((item, index) => (
                    <Draggable
                      draggableId={`init-draggable-${index}`}
                      index={index}
                      key={`init-draggable-${index}`}
                    >
                      {(provided, snapshot) => (
                        <div
                          ref={provided.innerRef}
                          {...provided.draggableProps}
                          {...provided.dragHandleProps}
                          style={{ marginBottom: "8px" }}
                        >
                          <div
                            style={{
                              backgroundColor: "#E5E7EB",
                              padding: "8px",
                              display: "flex",
                              justifyContent: "center",
                              alignItems: "center",
                              height: "60px",
                              lineHeight: "60px",
                            }}
                          >
                            {item}
                          </div>
                        </div>
                      )}
                    </Draggable>
                  ))}
                  {provided.placeholder}
                </div>
              )}
            </Droppable>
          </Col>
        </Row>
      </Container>
    </DragDropContext>
  );
};

export default Helper;
