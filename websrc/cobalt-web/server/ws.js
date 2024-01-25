const WebSocket = require("ws");

// 创建WebSocket服务器
const wss = new WebSocket.Server({ port: 8080 });

// 存储客户端信息的对象
const clients = {};

// 定义派发器
const dispatcher = {
  messageType1: handleMessageType1,
  messageType2: handleMessageType2,
  // 添加更多的消息类型和对应的处理函数
};

// 监听连接事件
wss.on("connection", (ws) => {
  console.log("Client connected");

  // 生成随机的客户端ID
  const clientId = generateClientId();

  // 为每个客户端存储信息
  clients[clientId] = {
    id: clientId,
    socket: ws,
    isAuth: false, // 身份验证标志，默认为未验证
  };

  // 发送欢迎消息给客户端
  ws.send(`Welcome! Your client ID is ${clientId}`);

  // 监听消息事件
  ws.on("message", (message) => {
    console.log(`Received message from client ${clientId}: ${message}`);

    // 判断是否已经进行身份验证
    if (!clients[clientId].isAuth) {
      // 进行身份验证
      if (authenticateClient(clientId, message)) {
        clients[clientId].isAuth = true; // 设置身份验证标志为已验证
        ws.send("Authentication successful");
      } else {
        ws.send("Authentication failed");
        ws.close(); // 关闭连接
      }
    } else {
      // 解析消息，获取消息类型和数据
      let parsedMessage;
      try {
        parsedMessage = JSON.parse(message);
      } catch (error) {
        console.error(
          `Error parsing message from client ${clientId}: ${message}`
        );
        return;
      }

      const { messageType, data } = parsedMessage;

      // 使用派发器调用对应的处理函数
      if (dispatcher.hasOwnProperty(messageType)) {
        dispatcher[messageType](clientId, data);
      } else {
        console.warn(`Unknown message type: ${messageType}`);
      }
    }
  });

  // 监听关闭事件
  ws.on("close", () => {
    console.log(`Client ${clientId} disconnected`);
    delete clients[clientId]; // 删除客户端信息
  });

  // 发送心跳消息给客户端
  setInterval(() => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send("Heartbeat");
    }
  }, 5000);
});

// 广播消息给所有客户端
function broadcastMessage(message) {
  Object.values(clients).forEach((client) => {
    if (client.socket.readyState === WebSocket.OPEN) {
      client.socket.send(message);
    }
  });
}

// 向指定客户端发送消息
function sendMessageToClient(clientId, message) {
  const client = clients[clientId];
  if (client && client.socket.readyState === WebSocket.OPEN) {
    client.socket.send(message);
  }
}

// 进行身份验证的逻辑
function authenticateClient(clientId, message) {
  // 在这里进行身份验证和授权的逻辑判断，返回 true 或 false
  // 例如，可以检查客户端提供的 API 密钥是否有效
  // 返回 true 表示验证成功，否则验证失败
  return message === "validApiKey";
}

// 处理消息类型1的函数
function handleMessageType1(clientId, data) {
  // 处理消息类型1的业务逻辑
  console.log(`Handling MessageType1 from client ${clientId}:`, data);

  // 回复客户端
  sendMessageToClient(clientId, "MessageType1 handled");
}

// 处理消息类型2的函数
function handleMessageType2(clientId, data) {
  // 处理消息类型2的业务逻辑
  console.log(`Handling MessageType2 from client ${clientId}:`, data);

  // 回复客户端
  sendMessageToClient(clientId, "MessageType2 handled");
}

// 生成随机的客户端ID
function generateClientId() {
  return Math.random().toString(36).substr(2, 8);
}
