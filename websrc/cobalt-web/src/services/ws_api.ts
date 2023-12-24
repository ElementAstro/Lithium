interface WebSocketConnection {
  onmessage: (message: MessageEvent) => void;
  send: (data: any) => void;
}

interface DeviceControlResponseMessage {  // which is the response of data.
  type: 'data' | 'error' | 'signal' | 'message',
  message: string,
  data: object,
}

const createWebSocketConnection = (url: string): WebSocketConnection => {
  const socket = new WebSocket(url);

  const connection: WebSocketConnection = {
    onmessage: (message: MessageEvent) => {
      console.log(`Received message: ${message.data}`);
    },
    send: (data: any) => {
      socket.send(JSON.stringify(data));
    },
  };

  socket.addEventListener("message", connection.onmessage);

  return connection;
};

const DeviceControlWebSocket = createWebSocketConnection('');

export default DeviceControlWebSocket;
