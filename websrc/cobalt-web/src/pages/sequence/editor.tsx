import React, { useEffect, useState } from "react";
import {
  Container,
  Row,
  Col,
  ListGroup,
  ListGroupItem,
  Collapse,
  Badge,
  Button,
} from "react-bootstrap";
import {
  BuildingFillGear as DetailIcon,
  CcCircle as LoopIcon,
  Heart as ConditionIcon,
  Messenger as StepIcon,
  Info as InfoIcon,
  XCircle as ErrorIcon,
  ExclamationCircle as WarningIcon,
  SkipStart as StartIcon,
  Stop as StopIcon,
  Camera as CameraIcon,
} from "react-bootstrap-icons";
import { GlobalStore } from "../../store/globalStore";
import { useEchoWebSocket } from "../../utils/websocketProvider";

function generateTasks() {
  return [
    {
      device_name: "PAA",
      instruction: "ManyExposure",
      type: "step",
      message: "messages1",
      data: "data1",
    },
  ];
}

interface TaskProp {
  instruction: string;
  device: string;
  params?: {};
}

interface TaskItemProps {
  task: TaskProp;
  isCurrentTask: boolean;
}

const listItemTextStyle = {
  paddingLeft: "3rem",
  fontSize: "0.8rem",
  color: "rgba(0, 0, 0, 0.6)",
  whiteSpace: "pre-line",
};

function TaskItem({ task, isCurrentTask }: TaskItemProps) {
  const [open, setOpen] = useState(false);

  const toggleOpen = () => {
    setOpen(!open);
  };

  let typeIcon;
  switch (task.device) {
    case "camera":
      typeIcon = <CameraIcon />;
      break;
    case "condition":
      typeIcon = <ConditionIcon />;
      break;
    case "loop":
      typeIcon = <LoopIcon />;
      break;
    default:
      typeIcon = null;
  }
  return (
    <div>
      <ListGroup.Item
        action
        className="d-flex justify-content-between align-items-center"
        onClick={toggleOpen}
      >
        <div className="d-flex align-items-center">
          <span>{typeIcon}</span>
          {isCurrentTask ? (
            <span className="mx-2">
              <Badge bg="primary">Running</Badge>
            </span>
          ) : null}
          <span>{task.instruction}</span>
        </div>
        <span>
          <DetailIcon />
        </span>
      </ListGroup.Item>
      <Collapse in={open}>
        <ListGroup.Item>
          <div className="d-flex justify-content-between align-items-center">
            <span>Device: {task.device}</span>
            {task.params !== undefined && typeof task.params === "object" ? (
              <span>
                Params:
                <ul>
                  {Object.entries(task.params as Record<string, any>).map(
                    ([key, value]) => (
                      <li key={key}>
                        {key}: {value.toString()}
                      </li>
                    )
                  )}
                </ul>
              </span>
            ) : null}
          </div>
        </ListGroup.Item>
      </Collapse>
    </div>
  );
}

function LogItem({ task, isCurrentTask }: TaskItemProps) {
  const [open, setOpen] = useState(false);

  const toggleOpen = () => {
    setOpen(!open);
  };
  let typeIcon;
  let IconColor;
  switch (task.type) {
    case "info":
      typeIcon = <InfoIcon />;
      IconColor = "#90EE90";
      break;
    case "warning":
      typeIcon = <WarningIcon />;
      IconColor = "#CCCC00";
      break;
    case "error":
      typeIcon = <ErrorIcon />;
      IconColor = "#DF4C5B";
      break;
    default:
      typeIcon = null;
  }

  return (
    <div>
      <ListGroup.Item
        action
        className="d-flex justify-content-between align-items-center"
        onClick={toggleOpen}
      >
        <div className="d-flex align-items-center">
          <span>
            <Badge bg={IconColor}>{typeIcon}</Badge>
          </span>
          <span className="mx-2">{task.message}</span>
        </div>
        <span>
          <DetailIcon />
        </span>
      </ListGroup.Item>
      <Collapse in={open}>
        <ListGroup.Item>
          <span>Messages: {task.message}</span>
        </ListGroup.Item>
      </Collapse>
    </div>
  );
}

export default function PAAEditPage() {
  const [tasks, setTasks] = useState<Array<TaskProp>>([]);
  const [logs, setLogs] = useState<Array<TaskProp>>([]);
  const [currentTaskIndex, setCurrentTaskIndex] = React.useState(0);

  const process_ws_message = (msg: any): void => {
    console.log(1);
    let response_data = msg;
    console.log(response_data);
    if (response_data.device_name == "PAA") {
      console.log(response_data);
    }
  };

  useEffect(() => {
    // 生成任务数组
    const generatedTasks = generateTasks();

    // 根据instruction进行分类
    const categorizedTasks = generatedTasks.reduce(
      (result: any, task) => {
        if (task.instruction === "log") {
          result.logs.push(task);
        } else {
          result.tasks.push(task);
        }
        return result;
      },
      { tasks: [], logs: [] }
    );

    // 更新状态
    setTasks(categorizedTasks.tasks);
    setLogs(categorizedTasks.logs);
  }, []);

  const { sendMessage, removeListener } = useEchoWebSocket(process_ws_message);

  const { current_script_data, current_script_info } = GlobalStore.useAppState(
    (state) => state.PAA
  );

  React.useEffect(() => {
    // 获取当前脚本
    GlobalStore.actions.PAA.get_current_script();
  }, []);

  return (
    <Container fluid>
      <Row>
        <Col xs={8}>
          <div className="my-2">
            <h5>当前运行脚本</h5>
            <ListGroup>
              <ListGroupItem
                action
                className="d-flex justify-content-between align-items-center"
              >
                <div className="d-flex align-items-center">
                  <span>{current_script_info.file_name}</span>
                  <span className="mx-2">
                    Modify Time: {current_script_info.modified_time}
                  </span>
                </div>
                <span>
                  <DetailIcon />
                </span>
              </ListGroupItem>
            </ListGroup>
          </div>

          <div className="my-2">
            <h5>PAA流程控制</h5>
            <Row>
              <Col xs={6}>
                <Button
                  variant="primary"
                  className="my-1 w-100"
                  onClick={() => {
                    GlobalStore.actions.PAA.post_start_PAA();
                  }}
                >
                  启动PAA流程
                </Button>
                <Button
                  variant="outline-primary"
                  className="my-1 w-100"
                  onClick={() => {
                    GlobalStore.actions.PAA.get_PAA_status();
                  }}
                >
                  查看PAA状态
                </Button>
              </Col>
              <Col xs={6}>
                <Button
                  variant="outline-secondary"
                  className="my-1 w-100"
                  onClick={() => {
                    GlobalStore.actions.PAA.post_stop_PAA();
                  }}
                >
                  暂停PAA流程
                </Button>
                <Button
                  variant="primary"
                  className="my-1 w-100"
                  onClick={() => {
                    GlobalStore.actions.PAA.post_stop_PAA();
                  }}
                >
                  终止PAA流程
                </Button>
              </Col>
            </Row>
          </div>

          <div className="my-2">
            <h5>日志界面</h5>
            <ListGroup>
              {logs.map((task, index) => (
                <LogItem
                  key={index}
                  task={task}
                  isCurrentTask={false}
                ></LogItem>
              ))}
            </ListGroup>
          </div>
        </Col>
        <Col xs={4}>
          <div className="my-2">
            <h5>流程图</h5>
            <div style={{ height: "30rem" }}>
              <img
                alt="流程图"
                src="https://via.placeholder.com/480x640.png?text=Flow+Chart"
                style={{ maxWidth: "100%", maxHeight: "100%" }}
              />
            </div>
          </div>
          <div className="my-2">
            <h5>任务队列</h5>
            <ListGroup>
              {tasks.map((task, index) => (
                <TaskItem
                  key={index}
                  task={task}
                  isCurrentTask={index === currentTaskIndex}
                ></TaskItem>
              ))}
            </ListGroup>
          </div>
        </Col>
      </Row>
    </Container>
  );
}
