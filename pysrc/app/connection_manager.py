# connection_manager.py
import asyncio
import uuid
import logging
from typing import List, Dict
from datetime import datetime
from fastapi import WebSocket, HTTPException
from fastapi import status
from websockets.exceptions import ConnectionClosedOK
from pydantic import BaseModel

logger = logging.getLogger(__name__)


class ClientConnection(BaseModel):
    """
    Data model for a client connection.
    """
    client_id: str
    connect_time: datetime


class ConnectionManager:
    """
    Manages WebSocket client connections.
    """

    def __init__(self, max_connections: int):
        self.active_connections: Dict[str, WebSocket] = {}
        self.max_connections = max_connections
        self.message_count: int = 0

    async def connect(self, websocket: WebSocket) -> str:
        """
        Accepts a WebSocket connection and stores it with a unique client ID.
        """
        if len(self.active_connections) >= self.max_connections:
            await websocket.close()
            logger.warning("Connection refused: Maximum connections reached.")
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Maximum connections reached"
            )

        await websocket.accept()
        client_id = str(uuid.uuid4())
        self.active_connections[client_id] = websocket
        logger.info(f"Client {client_id} connected")
        return client_id

    def disconnect(self, client_id: str):
        """
        Removes a WebSocket connection based on the client ID.
        """
        if client_id in self.active_connections:
            del self.active_connections[client_id]
            logger.info(f"Client {client_id} disconnected")

    async def broadcast(self, message: str):
        """
        Sends a message to all connected clients.
        """
        for connection in self.active_connections.values():
            await connection.send_text(message)
        self.message_count += 1
        logger.info(f"Broadcasted message: {message}")

    def get_online_clients(self) -> List[ClientConnection]:
        """
        Retrieves a list of currently online clients and their connection times.
        """
        return [
            ClientConnection(client_id=client_id, connect_time=datetime.now())
            for client_id in self.active_connections.keys()
        ]
