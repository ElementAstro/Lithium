import React, { useState, useEffect } from 'react';
import WebSocket from 'isomorphic-ws';
import {
    Container,
    Row,
    Col,
    Form,
    Button,
    ListGroup,
    Alert,
} from 'react-bootstrap';

function WebSocketClient() {
    const [messages, setMessages] = useState([]);
    const [serverUrl, setServerUrl] = useState('ws://localhost:3000');
    const [inputMessage, setInputMessage] = useState('');
    const [connected, setConnected] = useState(false);
    const [socket, setSocket] = useState(null);
    const [error, setError] = useState(null);

    useEffect(() => {
        if (socket) {
            socket.addEventListener('open', () => {
                console.log('Connected');
                setConnected(true);
                setError(null);
            });
            socket.addEventListener('close', () => {
                console.log('Disconnected');
                setConnected(false);
                setError(null);
                // 断线重连
                setTimeout(connectToServer, 5000);
            });
            socket.addEventListener('message', (event) => {
                const message = event.data;
                setMessages((prevMessages) => [...prevMessages, message]);
                // 进行消息过滤
                if (message.includes('important')) {
                    // 发送事件通知
                    sendNotification('Important message received');
                }
            });
            socket.addEventListener('error', (event) => {
                console.error('WebSocket error:', event);
                setError('WebSocket error');
                disconnectFromServer();
            });
        }
    }, [socket]);

    function connectToServer() {
        const newSocket = new WebSocket(serverUrl);
        // 设置超时时间为10秒钟
        const timeout = setTimeout(() => {
            console.error('Connection timeout');
            setError('Connection timeout');
            newSocket.close();
        }, 10000);
        newSocket.addEventListener('open', () => {
            clearTimeout(timeout); // 连接建立后清除超时定时器
        });
        setSocket(newSocket);
    }

    function disconnectFromServer() {
        if (socket) {
            socket.close();
            setSocket(null);
        }
    }

    function sendMessage(event) {
        event.preventDefault();
        if (socket) {
            socket.send(inputMessage);
            setInputMessage('');
        }
    }

    function sendNotification(message) {
        // 发送事件通知给用户
        console.log('Notification:', message);
    }

    return (
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
    );
}

export default WebSocketClient;
