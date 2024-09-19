from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Depends
from app.connection_manager import ConnectionManager
from app.dependence import get_current_username
from app.command_dispatcher import CommandDispatcher
from loguru import logger
import json
import asyncio
from typing import Dict, Any
from datetime import datetime

router = APIRouter()
manager = ConnectionManager()
command_dispatcher = CommandDispatcher()


@router.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket, username: str = Depends(get_current_username)):
    client_id = await manager.connect(websocket)
    await manager.broadcast(json.dumps({"type": "Server_msg", "message": f"Client {client_id} connected"}))

    try:
        # Start a background task for handling heartbeats
        heartbeat_task = asyncio.create_task(
            handle_heartbeat(client_id, websocket))

        while True:
            data = await websocket.receive_text()
            await process_message(client_id, data)
    except WebSocketDisconnect:
        await handle_disconnect(client_id, "Client disconnected")
    except Exception as e:
        logger.error(f"Unexpected error with client {client_id}: {e}")
        await handle_disconnect(client_id, f"Client disconnected due to error: {str(e)}")
    finally:
        heartbeat_task.cancel()


async def process_message(client_id: str, data: str):
    logger.info(f"Received message from {client_id}: {data}")
    try:
        message = json.loads(data)
        if isinstance(message, dict) and "command" in message:
            response = await command_dispatcher.dispatch(message["command"], message.get("params", {}))
            await manager.send_personal_message(json.dumps(response), client_id)
        else:
            await manager.broadcast(data)
    except json.JSONDecodeError:
        logger.warning(f"Received invalid JSON from client {client_id}")
        await manager.send_personal_message(json.dumps({"error": "Invalid JSON"}), client_id)


async def handle_disconnect(client_id: str, message: str):
    manager.disconnect(client_id)
    await manager.broadcast(json.dumps({"type": "Server_msg", "message": message}))


async def handle_heartbeat(client_id: str, websocket: WebSocket):
    while True:
        try:
            await asyncio.sleep(30)  # Send heartbeat every 30 seconds
            await websocket.send_text(json.dumps({"type": "heartbeat"}))
        except Exception as e:
            logger.error(f"Heartbeat failed for client {client_id}: {e}")
            break

# Register commands


@command_dispatcher.register("echo")
async def echo_command(params: Dict[str, Any]) -> Dict[str, Any]:
    return {"result": params.get("message", "No message provided")}


@command_dispatcher.register("get_active_clients")
async def get_active_clients_command(params: Dict[str, Any]) -> Dict[str, Any]:
    return {"result": len(manager.active_connections)}


@command_dispatcher.register("get_server_time")
async def get_server_time_command(params: Dict[str, Any]) -> Dict[str, Any]:
    return {"result": datetime.utcnow().isoformat()}


@command_dispatcher.register("broadcast_message")
async def broadcast_message_command(params: Dict[str, Any]) -> Dict[str, Any]:
    message = params.get("message", "")
    await manager.broadcast(json.dumps({"type": "broadcast", "message": message}))
    return {"result": "Message broadcasted"}


@command_dispatcher.register("get_client_status")
async def get_client_status_command(params: Dict[str, Any]) -> Dict[str, Any]:
    client_id = params.get("client_id")
    if client_id in manager.active_connections:
        return {"result": "Client is connected"}
    else:
        return {"result": "Client is not connected"}
