import React, {
  useState,
  useEffect,
  useRef,
  useCallback,
  createContext,
  useContext,
} from "react";
import WebSocket from "isomorphic-ws";
import {
  Container,
  Row,
  Col,
  Form,
  Button,
  ListGroup,
  Alert,
} from "react-bootstrap";

// 创建 WebSocketContext 作为全局的 WebSocket 连接上下文
const WebSocketContext = createContext("");

function WebSocketClient() {
  const [serverUrl, setServerUrl] = useState("ws://localhost:3000");
  const [messages, setMessages] = useState([]);
  const [inputMessage, setInputMessage] = useState("");
  const [connected, setConnected] = useState(false);
  const [socket, setSocket] = useState(null);
  const [error, setError] = useState("");

  // 使用 useRef 来保存 socket 实例
  const socketRef = useRef(null);

  // 使用 useCallback 对 connectToServer 和 disconnectFromServer 进行优化
  const connectToServer = useCallback(() => {
    const newSocket = new WebSocket(serverUrl);
    // 设置超时时间为10秒钟
    const timeout = setTimeout(() => {
      console.error("Connection timeout");
      setError("Connection timeout");
      newSocket.close();
    }, 10000);
    newSocket.addEventListener("open", () => {
      clearTimeout(timeout); // 连接建立后清除超时定时器
    });
    socketRef.current = newSocket;
  }, [serverUrl]);

  const disconnectFromServer = useCallback(() => {
    if (socketRef.current) {
      socketRef.current.close();
      socketRef.current = null;
    }
  }, []);

  // 使用 useEffect 监听 socket 状态变化
  useEffect(() => {
    const socket = socketRef.current;
    if (socket) {
      socket.addEventListener("open", () => {
        console.log("Connected");
        setError(null);
      });
      socket.addEventListener("close", () => {
        console.log("Disconnected");
        setError(null);
        // 断线重连
        setTimeout(connectToServer, 5000);
      });
      socket.addEventListener("message", (event) => {
        const message = event.data;
        setMessages((prevMessages) => [...prevMessages, message]);
        // 进行消息过滤
        if (message.includes("important")) {
          // 发送事件通知
          sendNotification("Important message received");
        }
      });
      socket.addEventListener("error", (event) => {
        console.error("WebSocket error:", event);
        setError("WebSocket error");
        disconnectFromServer();
      });
    }
  }, [connectToServer, disconnectFromServer]);

  const sendMessage = (event) => {
    event.preventDefault();
    if (socketRef.current) {
      socketRef.current.send(inputMessage);
      setInputMessage("");
    }
  };

  function sendNotification(message) {
    // 发送事件通知给用户
    console.log("Notification:", message);
  }

  // ... 其他代码保持不变

  return (
    <WebSocketContext.Provider
      value={{ connectToServer, disconnectFromServer }}
    >
      <Container>
        <Row className="my-4">
          <Col>
            <Form>
              <Form.Group controlId="serverUrl">
                <Form.Control
                  type="text"
                  placeholder="Enter server URL"
                  value={serverUrl}
                  onChange={(e) => setServerUrl(e.target.value)}
                />
              </Form.Group>
              <Button
                variant="primary"
                onClick={connectToServer}
                disabled={connected}
              >
                Connect
              </Button>
              <Button
                variant="danger"
                onClick={disconnectFromServer}
                disabled={!connected}
              >
                Disconnect
              </Button>
            </Form>
          </Col>
        </Row>
        {error && (
          <Row>
            <Col>
              <Alert variant="danger">{error}</Alert>
            </Col>
          </Row>
        )}
        <Row>
          <Col>
            <ListGroup>
              {messages.map((message, index) => (
                <ListGroup.Item key={index}>{message}</ListGroup.Item>
              ))}
            </ListGroup>
          </Col>
        </Row>
        <Row className="my-4">
          <Col>
            <Form onSubmit={sendMessage}>
              <Form.Group controlId="inputMessage">
                <Form.Control
                  type="text"
                  placeholder="Enter message"
                  value={inputMessage}
                  onChange={(e) => setInputMessage(e.target.value)}
                  disabled={!connected}
                />
              </Form.Group>
              <Button variant="primary" type="submit" disabled={!connected}>
                Send
              </Button>
            </Form>
          </Col>
        </Row>
      </Container>
    </WebSocketContext.Provider>
  );
}

// 在其他组件中使用全局 WebSocket 连接状态
function SomeOtherComponent() {
  const { connectToServer, disconnectFromServer } =
    useContext(WebSocketContext);
  // ...
}

export default WebSocketClient;
