import React, {
  createContext,
  FC,
  useContext,
  useEffect,
  useMemo,
  useState,
} from "react";
import { Websocket } from "./websocket";

let ws_url = "";
if (process.env.NODE_ENV === "development") {
  ws_url = "ws://localhost:5173/api/ws/indi_client/";
} else {
  ws_url = window.location.origin.replace("http", "ws");
  ws_url += "/ws/indi_client/";
}

type EchoWebSocketContextType = [
  addListener: (event: any, filter?: string) => void,
  sendMessage: (event: any) => void,
  removeListener: (fn: any) => void
];
const EchoWebSocketContext = createContext<EchoWebSocketContextType>([
  () => {},
  () => {},
  () => {},
]);

interface IProps {}

export const EchoWebSocketProvider: FC<IProps> = ({ children }) => {
  const [webSocket, setWebSocket] = useState<WebSocket | null>(null);
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
        any: currentListeners.any.filter((listener) => listener !== fn),
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

  const webSocketValue = useMemo(
    () => [addListener, sendMessage, removeListener],
    [addListener, sendMessage, removeListener]
  );

  useEffect(() => {
    if (webSocket) {
      return () => {
        webSocket.close();
      };
    }
  }, [webSocket]);

  return (
    <EchoWebSocketContext.Provider value={webSocketValue}>
      <Websocket
        url={ws_url}
        onOpen={(_event, socket) => {
          setWebSocket(socket);
          console.log("Connected to websocket");
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
  const [addListener, sendMessage, removeListener] =
    useContext(EchoWebSocketContext);

  useEffect(() => {
    if (listener) {
      addListener(listener, filter);
    }
    return () => {
      if (listener) {
        removeListener(listener);
      }
    };
  }, []);

  return { sendMessage, removeListener };
};
