import React, { useEffect, useState } from 'react';

export interface WebsocketProps {
  url: string;
  onMessage: (data:any) => void;
  onOpen: (event: Event, websocket: WebSocket) => void;
  keepAliveInterval?: number;
  onClose?: (event: Event, websocket: WebSocket) => void;
}

const keepAlive = (websocket: WebSocket, interval: number) => {
  setTimeout(() => {
    websocket.send(
      JSON.stringify({
        method: 'ping',
      }),
    );
    keepAlive(websocket, interval);
  }, interval);
};

export const Websocket = ({
  url,
  onMessage,
  onOpen,
  keepAliveInterval,
  onClose,
}: WebsocketProps) => {
  const [websocket, setWebsocket] = useState<any>(null);

  useEffect(() => {
    const websocket = new WebSocket(url);
    websocket.onmessage = (event: MessageEvent) => {
      const data = JSON.parse(event.data);
      onMessage(data);
    };
    if (onClose) {
      websocket.onclose = (event: Event) => {
        onClose(event, websocket);
      };
    }

    websocket.onopen = (event: Event) => {
      setWebsocket(websocket);
      onOpen(event, websocket);
      if (keepAliveInterval) {
        keepAlive(websocket, keepAliveInterval);
      }
    };
  }, []);

  useEffect(() => {
    if (websocket) {
      websocket.onmessage = (event: MessageEvent) => {
        const data = JSON.parse(event.data);
        onMessage(data);
      };
    }
  }, [onMessage]);

  return null;
};