"""
broadcast.py

This module sets up an HTTP and WebSocket server using FastAPI. The server allows multiple clients to connect
via WebSocket, exchange messages, and supports file uploads via HTTP. It also manages client connections,
broadcasts messages, and performs regular heartbeat checks to ensure clients are still connected.

Dependencies:
- FastAPI: Web framework for building APIs with Python.
- websockets: Library for building WebSocket servers and clients in Python.
- subprocess: Built-in module for spawning and managing additional processes.

Author: [Your Name]
Date: [Current Date]
"""

import asyncio
import uuid
import subprocess
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, UploadFile, File
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from typing import List, Dict
from websockets.exceptions import ConnectionClosedOK
from datetime import datetime

app = FastAPI()

# Enable CORS for all domains
origins = ["*"]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Set up static file directory
app.mount("/images", StaticFiles(directory="/dev/shm"), name="images")

class ConnectionManager:
    """
    Manages WebSocket client connections.
    """
    def __init__(self):
        self.active_connections: Dict[str, Dict] = {}

    async def connect(self, websocket: WebSocket) -> str:
        """
        Accepts a WebSocket connection and stores it with a unique client ID.

        Args:
            websocket (WebSocket): The WebSocket connection to accept.

        Returns:
            str: The unique client ID assigned to the connection.
        """
        await websocket.accept()
        client_id = str(uuid.uuid4())
        self.active_connections[client_id] = {
            "websocket": websocket,
            "connect_time": datetime.now()
        }
        return client_id

    def disconnect(self, client_id: str):
        """
        Removes a WebSocket connection based on the client ID.

        Args:
            client_id (str): The unique client ID to remove.
        """
        if client_id in self.active_connections:
            del self.active_connections[client_id]

    async def broadcast(self, message: str):
        """
        Sends a message to all connected clients.

        Args:
            message (str): The message to broadcast.
        """
        for connection in self.active_connections.values():
            await connection["websocket"].send_text(message)

    def get_online_clients(self) -> List[Dict]:
        """
        Retrieves a list of currently online clients and their connection times.

        Returns:
            List[Dict]: A list of dictionaries with client IDs and connection times.
        """
        return [
            {"client_id": client_id, "connect_time": info["connect_time"]}
            for client_id, info in self.active_connections.items()
        ]

manager = ConnectionManager()

# Path to the executable to be run
executable_path = './broadcast_server'  # Replace with the path to your executable

# Start the executable as a subprocess
process = subprocess.Popen(executable_path, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

async def read_process_output():
    """
    Reads the standard output from the subprocess and prints it.
    """
    while True:
        output = process.stdout.readline()
        if output:
            print(f"childProcess output: {output.strip()}")
        await asyncio.sleep(0.1)

async def read_process_error():
    """
    Reads the error output from the subprocess and prints it.
    """
    while True:
        error = process.stderr.readline()
        if error:
            print(f"childProcess error: {error.strip()}")
        await asyncio.sleep(0.1)

@app.on_event("startup")
async def startup_event():
    """
    Event handler for FastAPI startup. Starts reading subprocess output and error streams.
    """
    asyncio.create_task(read_process_output())
    asyncio.create_task(read_process_error())

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """
    WebSocket endpoint for handling client connections.

    Args:
        websocket (WebSocket): The WebSocket connection to handle.
    """
    client_id = await manager.connect(websocket)
    await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} connected"}}')
    print(f"Client {client_id} connected")

    try:
        while True:
            data = await websocket.receive_text()
            print(f"Received message from {client_id}: {data}")
            await manager.broadcast(data)
    except WebSocketDisconnect:
        manager.disconnect(client_id)
        await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} disconnected"}}')
        print(f"Client {client_id} disconnected")
    except ConnectionClosedOK:
        manager.disconnect(client_id)
        await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} disconnected"}}')
        print(f"Client {client_id} disconnected")

# Heartbeat function to check if clients are still connected
async def ping():
    """
    Regularly sends a ping to all clients to check if they are still connected.
    """
    while True:
        for client_id, connection in list(manager.active_connections.items()):
            try:
                await connection["websocket"].send_text("ping")
            except:
                manager.disconnect(client_id)
                await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} disconnected"}}')
                print(f"Client {client_id} did not respond to a ping, terminating.")
        await asyncio.sleep(3)  # Set to 3 seconds, adjust as needed

@app.on_event("startup")
async def startup_event():
    """
    Event handler for FastAPI startup. Starts the heartbeat ping.
    """
    asyncio.create_task(ping())

@app.post("/upload/")
async def upload_file(file: UploadFile = File(...)):
    """
    Endpoint for handling file uploads.

    Args:
        file (UploadFile): The file to upload.

    Returns:
        dict: Information about the saved file.
    """
    file_location = f"/dev/shm/{file.filename}"
    with open(file_location, "wb+") as file_object:
        file_object.write(file.file.read())
    await manager.broadcast(f'{{"type": "File_msg", "message": "{file.filename} uploaded"}}')
    return {"info": f"file '{file.filename}' saved at '{file_location}'"}

@app.get("/clients/")
async def get_clients():
    """
    Endpoint for retrieving a list of currently connected clients.

    Returns:
        List[Dict]: A list of currently connected clients and their connection times.
    """
    return manager.get_online_clients()

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8600)
