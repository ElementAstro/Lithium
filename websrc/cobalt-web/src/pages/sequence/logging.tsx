import React, { useEffect, useState } from "react";
import {
  ListGroup,
  ListGroupItem,
  Collapse,
  Badge,
  Container,
  Row,
  Col,
} from "react-bootstrap";

import {
  ListUl as DetailIcon,
  InfoCircle as InfoIcon,
  ExclamationTriangle as WarningIcon,
  XOctagonFill as ErrorIcon,
} from "react-bootstrap-icons";

function generateTasks() {
  return [
    {
      device_name: "PAA",
      instruction: "ManyExposure",
      type: "step",
      message: "messages1",
      data: "data1",
    },
    {
      device_name: "PAA",
      instruction: "Camera",
      type: "loop",
      message: "messages2",
      data: "data2",
    },
    {
      device_name: "PAA",
      instruction: "ManyExposure",
      type: "step",
      message: "messages1",
      data: "data1",
    },
    {
      device_name: "PAA",
      instruction: "Camera",
      type: "loop",
      message: "\ncheck_value: 10\nresult: true",
      data: "data2",
    },
    {
      device_name: "PAA",
      instruction: "ManyExposure",
      type: "step",
      message: "messages1",
      data: "data1",
    },
    {
      device_name: "PAA",
      instruction: "Camera",
      type: "loop",
      message: "messages2",
      data: "data2",
    },
    {
      device_name: "PAA",
      instruction: "ManyExposure",
      type: "step",
      message: "messages1",
      data: "data1",
    },
    {
      device_name: "PAA",
      instruction: "Camera",
      type: "loop",
      message: "messages2",
      data: "data2",
    },
    {
      device_name: "PAA",
      instruction: "ManyExposure",
      type: "step",
      message: "messages1",
      data: "data1",
    },
    {
      device_name: "PAA",
      instruction: "Camera",
      type: "loop",
      message: "messages2",
      data: "data2",
    },
    {
      device_name: "PAA",
      instruction: "Filter Switch",
      type: "condition",
      message: "messages3",
      data: "data3",
    },
    {
      device_name: "PAA",
      instruction: "log",
      type: "info",
      message: "info: entry found props changed guider_aperture",
      data: "",
    },
    {
      device_name: "PAA",
      instruction: "log",
      type: "error",
      message:
        'error: The pseudo class ":first-child" is potentially unsafe when doing server-side rendering. Try changing it to ":first-of-type".',
      data: "",
    },
    {
      device_name: "PAA",
      instruction: "log",
      type: "warning",
      message:
        'warning: react-jsx-dev-runtime.development.js:87: Each child in a list should have a unique "key" prop.',
      data: "",
    },
  ];
}

interface TaskProp {
  instruction: string;
  type: string;
  message: string;
  data: string;
}

interface TaskItemProps {
  task: TaskProp;
  isCurrentTask: boolean;
}

const listItemTextStyle = {
  paddingLeft: "45px",
  fontSize: "0.8rem",
  color: "rgba(0, 0, 0, 0.6)",
  whiteSpace: "pre-line",
};

function LogItem({ task, isCurrentTask }: TaskItemProps) {
  const [open, setOpen] = useState(false);

  const toggleOpen = () => {
    setOpen(!open);
  };

  let typeIcon;
  let iconColor;

  switch (task.type) {
    case "info":
      typeIcon = <InfoIcon />;
      iconColor = "#90EE90";
      break;
    case "warning":
      typeIcon = <WarningIcon />;
      iconColor = "#CCCC00";
      break;
    case "error":
      typeIcon = <ErrorIcon />;
      iconColor = "#DF4C5B";
      break;
    default:
      typeIcon = null;
  }

  return (
    <div>
      <ListGroupItem
        action
        onClick={toggleOpen}
        style={{ backgroundColor: isCurrentTask ? "#e0f7fa" : "white" }}
      >
        <Container fluid>
          <Row className="align-items-center">
            <Col xs={1}>
              <Badge bg={iconColor}>{typeIcon}</Badge>
            </Col>
            <Col xs={11}>
              <p style={{ marginBottom: "0px" }}>{task.message}</p>
            </Col>
          </Row>
        </Container>
      </ListGroupItem>
      <Collapse in={open}>
        <ListGroup>
          <ListGroupItem style={listItemTextStyle}>
            Messages: {task.message}
          </ListGroupItem>
        </ListGroup>
      </Collapse>
    </div>
  );
}

export default function PAADebugLogging() {
  const [logs, setLogs] = useState<Array<TaskProp>>([]);

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

    setLogs(categorizedTasks.logs);
  }, []);

  return (
    <Container fluid>
      <Row className="justify-content-center" lg={12}>
        <Col xs={8}>
          <ListGroup style={{ height: "80%", margin: "20px 10px" }}>
            <ListGroupItem active>日志界面</ListGroupItem>
            {logs.map((task, index) => (
              <LogItem key={index} task={task} isCurrentTask={false} />
            ))}
          </ListGroup>
        </Col>
      </Row>
    </Container>
  );
}
