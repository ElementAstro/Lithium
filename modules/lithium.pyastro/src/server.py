from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from typing import List, Optional, Dict, Any
import uvicorn
import PyIndi

app = FastAPI()

class IndiDevice:
    def __init__(self, name: str, client: PyIndi.BaseClient):
        self.name = name
        self.client = client
        self.device = None

    def connect(self):
        self.device = self.client.getDevice(self.name)
        if not self.device:
            print(f"Device {self.name} not found")
            return False
        self.client.connectDevice(self.name)
        print(f"Connected to {self.name}")
        return True

    def disconnect(self):
        if self.device:
            self.client.disconnectDevice(self.name)
            print(f"Disconnected from {self.name}")

    def send_command(self, command: str, params: Optional[Dict[str, Any]] = None):
        if not self.device:
            print(f"Device {self.name} not connected")
            return
        print(f"Sending command {command} to device {self.name} with params {params}")
        if command == "update_property" and params:
            property_name = params.get("property_name")
            element_name = params.get("element_name")
            value = params.get("value")
            if property_name and element_name and value is not None:
                property = self.device.getNumber(property_name)
                if property:
                    element = property[element_name]
                    if element:
                        element.setValue(value)
                        self.client.sendNewNumber(property)

class IndiClient(PyIndi.BaseClient):
    def __init__(self):
        super().__init__()
        self.devices: Dict[str, IndiDevice] = {}

    def newDevice(self, d):
        print(f"New device {d.getDeviceName()}")

    def newProperty(self, p):
        print(f"New property {p.getName()} for device {p.getDeviceName()}")

    def removeProperty(self, p):
        print(f"Remove property {p.getName()} for device {p.getDeviceName()}")

    def newBLOB(self, bp):
        print(f"New BLOB {bp.name}")

    def newSwitch(self, svp):
        print(f"New Switch {svp.name}")

    def newNumber(self, nvp):
        print(f"New Number {nvp.name}")

    def newText(self, tvp):
        print(f"New Text {tvp.name}")

    def newLight(self, lvp):
        print(f"New Light {lvp.name}")

    def newMessage(self, d, m):
        print(f"New Message from {d.getDeviceName()}: {m.message}")

    def serverConnected(self):
        print("Server connected")

    def serverDisconnected(self, code):
        print("Server disconnected")

    def add_device(self, device_name: str):
        device = IndiDevice(device_name, self)
        if device.connect():
            self.devices[device_name] = device

indi_client = IndiClient()
indi_client.setServer("localhost", 7624)  # Update with your INDI server address and port
if not indi_client.connectServer():
    print("No INDI server running on localhost:7624")
else:
    print("Connected to INDI server")

class Device(BaseModel):
    id: int
    name: str
    status: str
    description: Optional[str] = None

class DeviceCommand(BaseModel):
    command: str
    params: Optional[dict] = None

class DeviceGroup(BaseModel):
    id: int
    name: str
    device_ids: List[int]

devices: Dict[int, Device] = {}
device_groups: Dict[int, DeviceGroup] = {}
device_logs: Dict[int, List[str]] = {}

# Device Management Endpoints
@app.get("/devices", response_model=List[Device])
async def read_devices():
    return list(devices.values())

@app.get("/devices/{device_id}", response_model=Device)
async def read_device(device_id: int):
    device = devices.get(device_id)
    if device:
        return device
    raise HTTPException(status_code=404, detail="Device not found")

@app.post("/devices", response_model=Device)
async def create_device(device: Device):
    if device.id in devices:
        raise HTTPException(status_code=400, detail="Device with this ID already exists")
    devices[device.id] = device
    device_logs[device.id] = []
    await message_bus.broadcast(f"New device added: {device.name}")
    return device

@app.put("/devices/{device_id}", response_model=Device)
async def update_device(device_id: int, updated_device: Device):
    if device_id in devices:
        devices[device_id] = updated_device
        await message_bus.broadcast(f"Device updated: {updated_device.name}")
        return updated_device
    raise HTTPException(status_code=404, detail="Device not found")

@app.delete("/devices/{device_id}", response_model=Device)
async def delete_device(device_id: int):
    if device_id in devices:
        deleted_device = devices.pop(device_id)
        device_logs.pop(device_id, None)
        await message_bus.broadcast(f"Device deleted: {deleted_device.name}")
        return deleted_device
    raise HTTPException(status_code=404, detail="Device not found")

# Device Group Management Endpoints
@app.get("/device_groups", response_model=List[DeviceGroup])
async def read_device_groups():
    return list(device_groups.values())

@app.get("/device_groups/{group_id}", response_model=DeviceGroup)
async def read_device_group(group_id: int):
    group = device_groups.get(group_id)
    if group:
        return group
    raise HTTPException(status_code=404, detail="Group not found")

@app.post("/device_groups", response_model=DeviceGroup)
async def create_device_group(group: DeviceGroup):
    if group.id in device_groups:
        raise HTTPException(status_code=400, detail="Group with this ID already exists")
    device_groups[group.id] = group
    return group

@app.put("/device_groups/{group_id}", response_model=DeviceGroup)
async def update_device_group(group_id: int, updated_group: DeviceGroup):
    if group_id in device_groups:
        device_groups[group_id] = updated_group
        return updated_group
    raise HTTPException(status_code=404, detail="Group not found")

@app.delete("/device_groups/{group_id}", response_model=DeviceGroup)
async def delete_device_group(group_id: int):
    if group_id in device_groups:
        return device_groups.pop(group_id)
    raise HTTPException(status_code=404, detail="Group not found")

# Device Log Endpoints
@app.get("/devices/{device_id}/logs", response_model=List[str])
async def read_device_logs(device_id: int):
    return device_logs.get(device_id, [])

# Message Bus for WebSocket communication
class MessageBus:
    def __init__(self):
        self.active_connections: Dict[int, WebSocket] = {}

    async def connect(self, websocket: WebSocket, device_id: int):
        await websocket.accept()
        self.active_connections[device_id] = websocket
        await self.broadcast(f"Device {device_id} connected")

    def disconnect(self, device_id: int):
        self.active_connections.pop(device_id, None)

    async def send_command(self, device_id: int, command: str, params: Optional[dict] = None):
        if device_id in self.active_connections:
            message = {"command": command, "params": params}
            await self.active_connections[device_id].send_json(message)

    async def broadcast(self, message: str):
        for connection in self.active_connections.values():
            await connection.send_text(message)

    async def send_message(self, sender_id: int, receiver_id: int, message: str):
        if receiver_id in self.active_connections:
            await self.active_connections[receiver_id].send_text(message)
            log_entry = f"Message from {sender_id} to {receiver_id}: {message}"
            device_logs[receiver_id].append(log_entry)
            await self.broadcast(log_entry)

message_bus = MessageBus()

@app.websocket("/ws/{device_id}")
async def websocket_endpoint(websocket: WebSocket, device_id: int):
    await message_bus.connect(websocket, device_id)
    try:
        while True:
            data = await websocket.receive_json()
            command = data.get("command")
            params = data.get("params")
            if command:
                log_entry = f"Command received: {command} with params {params}"
                device_logs[device_id].append(log_entry)
                await message_bus.broadcast(log_entry)
            message = data.get("message")
            receiver_id = data.get("receiver_id")
            if message and receiver_id:
                await message_bus.send_message(device_id, receiver_id, message)
    except WebSocketDisconnect:
        message_bus.disconnect(device_id)

# Device Control Endpoints
@app.post("/devices/{device_id}/control", response_model=str)
async def control_device(device_id: int, command: DeviceCommand):
    if device_id not in devices:
        raise HTTPException(status_code=404, detail="Device not found")
    
    device = devices[device_id]
    if device.name in indi_client.devices:
        indi_device = indi_client.devices[device.name]
        indi_device.send_command(command.command, command.params)
        await message_bus.send_command(device_id, command.command, command.params)
        return f"Command {command.command} sent to INDI device {device.name}"
    else:
        await message_bus.send_command(device_id, command.command, command.params)
        return f"Command {command.command} sent to device {device_id}"

@app.post("/device_groups/{group_id}/control", response_model=str)
async def control_device_group(group_id: int, command: DeviceCommand):
    group = device_groups.get(group_id)
    if not group:
        raise HTTPException(status_code=404, detail="Group not found")
    for device_id in group.device_ids:
        if device_id in devices:
            device = devices[device_id]
            if device.name in indi_client.devices:
                indi_device = indi_client.devices[device.name]
                indi_device.send_command(command.command, command.params)
            await message_bus.send_command(device_id, command.command, command.params)
    return f"Command {command.command} sent to group {group_id}"

# Device Messaging Endpoints
@app.post("/devices/{sender_id}/message/{receiver_id}", response_model=str)
async def send_message(sender_id: int, receiver_id: int, message: str):
    if sender_id not in devices or receiver_id not in devices:
        raise HTTPException(status_code=404, detail="Device not found")
    await message_bus.send_message(sender_id, receiver_id, message)
    return f"Message sent from device {sender_id} to device {receiver_id}"

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)