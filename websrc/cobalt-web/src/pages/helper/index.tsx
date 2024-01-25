import React, { LegacyRef } from "react";
import { DragDropContext, Droppable, Draggable } from "react-beautiful-dnd";
import Container from "react-bootstrap/Container";
import Row from "react-bootstrap/Row";
import Col from "react-bootstrap/Col";
import Button from "react-bootstrap/Button";
import { BoxArrowInDownRight } from "react-bootstrap-icons";
import {
  HelperPage,
  SelectBox,
  SelectItem,
  InitBox,
  InitItem,
  TempTitle,
} from "./style";

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

  const clearAllCards = () => {
    setInitItems(DEVICES_ITEMS);
    setSelectItems(SELECT_ITEMS);
  };

  const resetCards = () => {
    setInitItems(DEVICES_ITEMS);
    setSelectItems(SELECT_ITEMS);
  };

  const shuffleCards = () => {
    const shuffled = DEVICES_ITEMS.sort(() => Math.random() - 0.5);
    setInitItems(shuffled);
  };

  return (
    <DragDropContext onDragEnd={onDragEnd} onDragStart={onDragStart}>
      <HelperPage>
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
                      <SelectBox
                        {...provided.droppableProps}
                        ref={provided.innerRef}
                      >
                        <Draggable
                          draggableId={`select-draggable-${index}`}
                          index={index}
                          key={`select-draggable-${index}`}
                          isDragDisabled
                        >
                          {(provided, snapshot) => (
                            // 卡片槽样式，注意组件一定要position: absolute
                            <SelectItem
                              ref={provided.innerRef}
                              {...provided.draggableProps}
                              {...provided.dragHandleProps}
                            >
                              <div>
                                <div>
                                  <span>{item}</span>
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
                            </SelectItem>
                          )}
                        </Draggable>
                        {provided.placeholder}
                      </SelectBox>
                    )}
                  </Droppable>
                </Col>
              ))}
            </Row>
          </Col>
          <Col xs={6}>
            {/* 卡片池 */}
            <Droppable droppableId="init_droppable">
              {(
                provided: {
                  innerRef: LegacyRef<HTMLDivElement> | undefined;
                  droppableProps;
                },
                snapshot
              ) => (
                <InitBox {...provided.droppableProps} ref={provided.innerRef}>
                  <Button variant="secondary" onClick={clearAllCards}>
                    清空所有卡片
                  </Button>
                  <Button variant="secondary" onClick={resetCards}>
                    重置卡片槽和卡片池
                  </Button>
                  <Button variant="secondary" onClick={shuffleCards}>
                    随机排序卡片池
                  </Button>
                  {init_items.map((item, index) => (
                    <Draggable
                      draggableId={`init-draggable-${index}`}
                      index={index}
                      key={`init-draggable-${index}`}
                    >
                      {(provided, snapshot) => (
                        <InitItem
                          ref={provided.innerRef}
                          {...provided.draggableProps}
                          {...provided.dragHandleProps}
                        >
                          <div>{item}</div>
                        </InitItem>
                      )}
                    </Draggable>
                  ))}
                  {provided.placeholder}
                </InitBox>
              )}
            </Droppable>
          </Col>
        </Row>
      </HelperPage>
    </DragDropContext>
  );
};

export default Helper;
