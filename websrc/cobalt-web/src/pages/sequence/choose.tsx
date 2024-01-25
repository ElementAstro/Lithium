import React, { useEffect, useState } from "react";
import {
  Container,
  Row,
  Col,
  ListGroup,
  Image,
  Button,
  ListGroupItem,
} from "react-bootstrap";

import {
  TrashFill as DeleteIcon,
  List as DetailIcon,
  Bootstrap as WelcomeIcon,
} from "react-bootstrap-icons";

import { GlobalStore } from "../../store/globalStore";

function generateObjects() {
  return [
    {
      object_name: "PAA_First_Object",
      image: WelcomeIcon,
      date: "2023-11-16",
      time: "20:45:33",
      detail: "This is the detail of the first Object",
    },
    {
      object_name: "PAA_Second_Object",
      image: WelcomeIcon,
      date: "2023-11-17",
      time: "14:30:45",
      detail: "Another object with a different date and time.",
    },
    {
      object_name: "PAA_Third_Object",
      image: WelcomeIcon,
      date: "2023-11-18",
      time: "08:15:20",
      detail: "Here's a third object with a unique date and time.",
    },
    {
      object_name: "PAA_Fourth_Object",
      image: WelcomeIcon,
      date: "2023-11-19",
      time: "12:00:00",
      detail: "Adding a fourth object with its own date and time.",
    },
    {
      object_name: "PAA_Fifth_Object",
      image: WelcomeIcon,
      date: "2023-11-20",
      time: "18:45:55",
      detail: "Fifth object description with a specific date and time.",
    },
  ];
}

interface ObjectProp {
  object_name: string;
  image: string;
  date: string;
  time: string;
  detail: string;
}

function ObjectItem(props: { task: ObjectProp; onEdit: any; onDelete: any }) {
  let task = props.task;
  return (
    <div>
      <ListGroup horizontal>
        <ListGroup.Item style={{ width: "100px", height: "80px" }}>
          <Image
            src={task.image}
            alt={task.object_name}
            style={{ width: "100%", height: "100%", objectFit: "cover" }}
          />
        </ListGroup.Item>
        <ListGroup.Item>
          <h5>{task.object_name}</h5>
          <p>Detail: {task.detail}</p>
          <p>Date: {task.date}</p>
          <p>Time: {task.time}</p>
          <Button variant="primary" onClick={props.onEdit}>
            <DetailIcon />
          </Button>
          <Button variant="danger" onClick={props.onDelete}>
            <DeleteIcon />
          </Button>
        </ListGroup.Item>
      </ListGroup>
    </div>
  );
}

export default function PAAChoosePage() {
  const [objects, setObjects] = useState<Array<ObjectProp>>([]);

  useEffect(() => {
    // 生成任务数组
    setObjects(generateObjects);
  }, []);

  const toEditPage = (index: number) => {
    GlobalStore.actions.PAA.setState({ setting_mode: 1 });
    GlobalStore.actions.PAA.setState({ get_object: true });
    // GlobalStore.actions.PAA.setState({object_index: index})
  };

  const testFunction = () => {
    console.log(GlobalStore.actions.PAA.post_start_PAA());
  };

  const deleteObject = (index: number) => {
    // Implement your logic to delete the object based on the index
    const updatedObjects = [...objects];
    updatedObjects.splice(index, 1);
    setObjects(updatedObjects);
  };
  return (
    <Container fluid>
      <Row>
        <Col xs={2}></Col>
        <Col xs={8}>
          <div style={{ padding: "16px", height: "90%", margin: "20px 10px" }}>
            <h3>可选星体</h3>
            <ListGroup style={{ overflow: "auto", maxHeight: "100%" }}>
              {objects.map((task, index) => (
                <ListGroupItem key={index} style={{ width: "100%" }}>
                  <ObjectItem
                    task={task}
                    onEdit={() => toEditPage(index)}
                    onDelete={() => deleteObject(index)}
                  />
                </ListGroupItem>
              ))}
            </ListGroup>
          </div>
        </Col>
      </Row>
    </Container>
  );
}
