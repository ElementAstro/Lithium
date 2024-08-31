import asyncio
from contextlib import asynccontextmanager
from datetime import datetime
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException, Depends, UploadFile, File, APIRouter
from fastapi.middleware.cors import CORSMiddleware
from fastapi.security import HTTPBasic, HTTPBasicCredentials
from loguru import logger
from config.config import config
from app.connection_manager import ConnectionManager
from app.plugin_manager import load_plugins, start_plugin_watcher, stop_plugin_watcher, update_plugin, install_plugin, get_plugin_info, check_plugin_dependencies

# 配置 loguru 日志系统
logger.add("server.log", level="DEBUG", format="{time} {level} {message}", rotation="10 MB")

# 使用 lifespan 事件处理器替代 startup 和 shutdown 事件
@asynccontextmanager
async def lifespan(app: FastAPI):
    """
    FastAPI lifespan event for startup and shutdown.
    """
    # Startup
    logger.info("Starting up the application")
    load_plugins(config.plugin_directory, app)  # 同步加载插件


    observer = start_plugin_watcher(config.plugin_directory, app)
    app.state.plugin_observer = observer
    logger.info("Plugin watcher started")

    # Shutdown
    yield  # The point between startup and shutdown

    logger.info("Shutting down the application")
    observer = app.state.plugin_observer
    if observer:
        stop_plugin_watcher(observer)
        logger.info("Plugin watcher stopped")

app = FastAPI(lifespan=lifespan)
router = APIRouter()

# Enable CORS for all domains
origins = ["*"]
app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize connection manager
manager = ConnectionManager(max_connections=config.max_connections)

# Security setup
security = HTTPBasic()

def get_current_username(credentials: HTTPBasicCredentials = Depends(security)):
    """
    Basic authentication check.
    """
    correct_username = credentials.username == config.auth_username
    correct_password = credentials.password == config.auth_password
    if not (correct_username and correct_password):
        logger.warning(f"Unauthorized access attempt with username: {credentials.username}")
        raise HTTPException(
            status_code=HTTP_403_FORBIDDEN,
            detail="Incorrect username or password",
            headers={"WWW-Authenticate": "Basic"},
        )
    logger.info(f"Authenticated user: {credentials.username}")
    return credentials.username

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket, username: str = Depends(get_current_username)):
    """
    WebSocket endpoint for handling client connections.
    """
    client_id = await manager.connect(websocket)
    await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} connected"}}')

    try:
        async with websocket:
            while True:
                data = await websocket.receive_text()
                logger.info(f"Received message from {client_id}: {data}")
                await manager.broadcast(data)
    except WebSocketDisconnect:
        manager.disconnect(client_id)
        await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} disconnected"}}')
    except Exception as e:
        logger.error(f"Unexpected error with client {client_id}: {e}")
        manager.disconnect(client_id)
        await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} disconnected due to error"}}')

# Heartbeat function to check if clients are still connected
async def ping():
    """
    Regularly sends a ping to all clients to check if they are still connected.
    """
    while True:
        for client_id, connection in list(manager.active_connections.items()):
            try:
                await connection.send_text("ping")
            except Exception:
                manager.disconnect(client_id)
                await manager.broadcast(f'{{"type": "Server_msg", "message": "Client {client_id} disconnected"}}')
                logger.warning(f"Client {client_id} did not respond to a ping, terminating.")
        await asyncio.sleep(config.broadcast_interval)

@router.post("/upload/")
async def upload_file(file: UploadFile = File(...), username: str = Depends(get_current_username)):
    """
    Endpoint for handling file uploads.
    """
    file_location = f"/dev/shm/{file.filename}"
    try:
        with open(file_location, "wb+") as file_object:
            file_object.write(file.file.read())
        logger.info(f"File '{file.filename}' saved at '{file_location}'")
        await manager.broadcast(f'{{"type": "File_msg", "message": "{file.filename} uploaded"}}')
        return {"info": f"file '{file.filename}' saved at '{file_location}'"}
    except Exception as e:
        logger.error(f"Error saving file {file.filename}: {e}")
        raise HTTPException(status_code=500, detail="File upload failed")

# Register plugin management endpoints
@app.get("/plugins/")
async def list_plugins(username: str = Depends(get_current_username)):
    """
    Lists all currently loaded plugins.
    """
    from app.plugin_manager import list_plugins
    plugins = list_plugins()
    logger.info(f"Loaded plugins: {plugins}")
    return {"loaded_plugins": plugins}

@app.get("/plugins/info/")
async def get_plugin_info_endpoint(plugin_name: str, username: str = Depends(get_current_username)):
    """
    Endpoint to get detailed information about a plugin.
    """
    plugin_info = get_plugin_info(plugin_name)
    logger.info(f"Retrieved info for plugin '{plugin_name}': {plugin_info}")
    return plugin_info

@app.post("/plugins/load/")
async def load_plugin_endpoint(plugin_name: str, username: str = Depends(get_current_username)):
    """
    Endpoint to load a plugin dynamically.
    """
    from app.plugin_manager import load_plugin
    load_plugin(config.plugin_directory, plugin_name, app)
    logger.info(f"Plugin '{plugin_name}' loaded")
    return {"status": f"Plugin {plugin_name} loaded"}

@app.post("/plugins/unload/")
async def unload_plugin_endpoint(plugin_name: str, username: str = Depends(get_current_username)):
    """
    Endpoint to unload a plugin dynamically.
    """
    from app.plugin_manager import unload_plugin
    unload_plugin(plugin_name, app)
    logger.info(f"Plugin '{plugin_name}' unloaded")
    return {"status": f"Plugin {plugin_name} unloaded"}

@app.post("/plugins/reload/")
async def reload_plugin_endpoint(plugin_name: str, username: str = Depends(get_current_username)):
    """
    Endpoint to reload a plugin dynamically.
    """
    from app.plugin_manager import reload_plugin
    reload_plugin(plugin_name, config.plugin_directory, app)
    logger.info(f"Plugin '{plugin_name}' reloaded")
    return {"status": f"Plugin {plugin_name} reloaded"}

@app.post("/plugins/update/")
async def update_plugin_endpoint(plugin_name: str, username: str = Depends(get_current_username)):
    """
    Endpoint to update a plugin dynamically.
    """
    update_plugin(plugin_name, app)
    logger.info(f"Plugin '{plugin_name}' updated")
    return {"status": f"Plugin {plugin_name} updated"}

@app.post("/plugins/install/")
async def install_plugin_endpoint(plugin_name: str, repository: str, username: str = Depends(get_current_username)):
    """
    Endpoint to install a plugin from an external repository.
    """
    install_plugin(plugin_name, repository, app)
    logger.info(f"Plugin '{plugin_name}' installed from {repository}")
    return {"status": f"Plugin {plugin_name} installed from {repository}"}

@app.get("/plugins/dependencies/")
async def check_plugin_dependencies_endpoint(plugin_name: str, username: str = Depends(get_current_username)):
    """
    Endpoint to check the dependencies of a plugin.
    """
    missing_dependencies = check_plugin_dependencies(plugin_name)
    if missing_dependencies:
        logger.warning(f"Plugin '{plugin_name}' is missing dependencies: {missing_dependencies}")
        return {"plugin": plugin_name, "missing_dependencies": missing_dependencies}
    else:
        logger.info(f"Plugin '{plugin_name}' has all dependencies satisfied")
        return {"plugin": plugin_name, "status": "All dependencies satisfied"}



# Include router
app.include_router(router)

if __name__ == "__main__":
    startup_time = datetime.now()  # Track server startup time
    logger.info(f"Server starting at {startup_time}")
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8600)
