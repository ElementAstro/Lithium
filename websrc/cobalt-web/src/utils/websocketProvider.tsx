import React, { ReactNode, useContext, useEffect, useState } from "react";
import { Websocket } from "./websocket";

let ws_url = "";
if (process.env.NODE_ENV === "development") {
  ws_url = "ws://localhost:5173/api/ws/indi_client/";
} else {
  ws_url = window.location.origin.replace("http", "ws");
  ws_url += "/ws/indi_client/";
}

export type EchoWebSocketContextType = [
  (event: any, filter?: string) => void,
  (event: any) => void,
  (fn: any) => void,
  boolean
];
export const EchoWebSocketContext =
  React.createContext<EchoWebSocketContextType>([
    () => {},
    () => {},
    () => {},
    false,
  ]);

export const EchoWebSocketProvider: React.FC<IProps> = ({ children }) => {
  const [webSocket, setWebSocket] = useState<WebSocket | null>(null);
  const [webSocketIsOpen, setWebSocketIsOpen] = useState(false); // 新的状态
  const [listeners, setListeners] = useState<{
    [key: string]: ((event: any) => void)[];
  }>({});

  const addListener = (fn: any, filter = "any") => {
    setListeners((currentListeners) => {
      return {
        ...currentListeners,
        [filter]: [...(currentListeners[filter] || []), fn],
      };
    });
  };

  const removeListener = (fn: any) => {
    setListeners((currentListeners) => {
      return {
        ...currentListeners,
        any: currentListeners["any"].filter((listener) => listener !== fn),
      };
    });
  };

  const sendMessage = (message: string) => {
    if (webSocket) {
      webSocket.send(message);
    }
  };

  const onMessage = (event: any) => {
    const { data } = event;
    if (listeners[data]) {
      listeners[data].forEach((listener) => listener(event));
    }

    if (listeners.any) {
      listeners.any.forEach((listener) => listener(event));
    }
  };
  return (
    <EchoWebSocketContext.Provider
      value={[addListener, sendMessage, removeListener, webSocketIsOpen]}
    >
      <Websocket
        url={ws_url}
        onOpen={(_event, socket) => {
          setWebSocket(socket);
          setWebSocketIsOpen(true);
          console.log("Connected to websocket");
        }}
        onClose={() => {
          setWebSocketIsOpen(false); // 这里设置WebSocket连接关闭
          console.log("Disconnected from websocket");
        }}
        onMessage={onMessage}
      />
      {children}
    </EchoWebSocketContext.Provider>
  );
};

export const useEchoWebSocket = (
  listener?: (event: any) => void,
  filter = "any"
) => {
  const [addListener, sendMessage, removeListener, webSocketIsOpen] =
    useContext(EchoWebSocketContext);
  useEffect(() => {
    if (listener) {
      addListener(listener, filter);
    }
  }, []);

  return { sendMessage, removeListener, webSocketIsOpen };
};
