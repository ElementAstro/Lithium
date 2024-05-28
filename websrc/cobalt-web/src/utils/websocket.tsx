import React, { useEffect, useState, useCallback, useRef } from "react";

export interface WebsocketProps {
  url: string;
  onMessage: (data: any) => void;
  onOpen: (event: Event, websocket: WebSocket) => void;
  keepAliveInterval?: number;
  onClose?: (event: Event, websocket: WebSocket) => void;
}

const keepAlive = (websocket: WebSocket, interval: number) => {
  setTimeout(() => {
    websocket.send(
      JSON.stringify({
        method: "ping",
      })
    );
    keepAlive(websocket, interval);
  }, interval);
};

/*
{
  url,
  onMessage,
  onOpen,
  keepAliveInterval,
  onClose,
}
*/
export const Websocket = (props: WebsocketProps) => {
  // const [websocket, setWebsocket] = useState<any>(null);

  // useEffect(() => {
  //   const websocket = new WebSocket(url);
  //   websocket.onmessage = (event: MessageEvent) => {
  //     const data = JSON.parse(event.data);
  //     onMessage(data);
  //   };
  //   if (onClose) {
  //     websocket.onclose = (event: Event) => {
  //       onClose(event, websocket);
  //     };
  //   }

  //   websocket.onopen = (event: Event) => {
  //     setWebsocket(websocket);
  //     onOpen(event, websocket);
  //     if (keepAliveInterval) {
  //       keepAlive(websocket, keepAliveInterval);
  //     }
  //   };

  //   websocket.onclose = (event: Event) => {
  //     console.log('WebSocket connection closed:', event);
  //     if (onClose) {
  //      onClose(event, websocket);
  //     }
  //     console.log('Reconnecting to WebSocket...');
  //     setTimeout(() => setWebsocket(new WebSocket(url)), 5000);
  //   };

  //   websocket.onerror = (error: Event) => {
  //     console.log('WebSocket error:', error);
  //     console.log('Reconnecting to WebSocket...');
  //     setTimeout(() => setWebsocket(new WebSocket(url)), 5000);
  //   };
  // }, []);

  // useEffect(() => {
  //   if (websocket) {
  //     websocket.onmessage = (event: MessageEvent) => {
  //       const data = JSON.parse(event.data);
  //       onMessage(data);
  //     };
  //   }
  // }, [onMessage]);
  const propsRef = useRef(props);
  propsRef.current = props;

  const connect = useCallback(() => {
    const { url } = props;
    const websocket = new WebSocket(url);

    websocket.onopen = (event) => {
      console.log("WebSocket is open now.");
      propsRef.current.onOpen(event, websocket);
      if (props.keepAliveInterval) {
        keepAlive(websocket, props.keepAliveInterval);
      }
    };

    websocket.onmessage = (event) => {
      console.log("WebSocket message received:", event);
      const data = JSON.parse(event.data);
      const onMessage = propsRef.current.onMessage;
      onMessage(data);
    };

    websocket.onerror = (error) => {
      console.log("WebSocket error: ", error);
    };

    websocket.onclose = (event) => {
      console.log("WebSocket closed, retrying in 5000ms", event);
      setTimeout(connect, 5000);
    };
  }, [props]);

  useEffect(() => {
    console.log("Setting up a new WebSocket connection...");
    connect();
  }, []);

  return null;
};
