from fastapi import UploadFile, File, HTTPException
from fastapi.responses import JSONResponse
from . import app
from .models import CommandModel, ExecPythonRequest
from .dispatch import command_dispatcher, some_safe_function

@app.post("/commands/execute", response_model=Dict[str, Any])
async def execute_command(command: CommandModel):
    try:
        result = await command_dispatcher.execute_command(command.name, *command.args, user_permissions=command.permissions, **command.kwargs)
        return {"result": result}
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))

@app.get("/commands/list", response_model=Dict[str, str])
async def list_commands():
    return command_dispatcher.list_commands()

@app.post("/commands/register", response_model=Dict[str, Any])
async def register_command(command_name: str, description: str = "", aliases: List[str] = [], permissions: List[str] = [], cooldown: float = 0.0):
    async def dummy_command(*args, **kwargs):
        return "This is a dummy command"

    try:
        command_dispatcher.register_command(command_name, dummy_command, description, aliases, permissions, cooldown)
        return {"message": f"Command {command_name} registered successfully"}
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))

@app.post("/commands/unregister", response_model=Dict[str, Any])
async def unregister_command(command_name: str):
    try:
        command_dispatcher.unregister_command(command_name)
        return {"message": f"Command {command_name} unregistered successfully"}
    except ValueError as e:
        raise HTTPException(status_code=400, detail=str(e))

@app.post("/commands/load_module")
async def load_module(code: str, module_name: str):
    from types import ModuleType
    import sys

    new_module = ModuleType(module_name)
    try:
        exec(code, new_module.__dict__)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Failed to execute code: {str(e)}")

    command_dispatcher.auto_register(new_module)
    return {"message": f"Module {module_name} loaded and commands registered successfully"}

@app.post("/exec_python")
async def exec_python(request: ExecPythonRequest):
    local_vars = {}
    global_vars = {
        "__builtins__": {},
        "allowed_function": some_safe_function
    }

    try:
        exec(request.code, global_vars, local_vars)
    except Exception as e:
        return {"error": f"Failed to execute code: {str(e)}"}

    return {"result": local_vars.get("result", "No result returned by code.")}

@app.post("/load_module")
async def c_load_module(file: UploadFile = File(...)):
    import importlib.util
    import os
    import sys

    module_name = file.filename.rsplit('.', 1)[0]
    module_path = f"/tmp/{file.filename}"

    with open(module_path, 'wb') as f:
        f.write(await file.read())

    try:
        spec = importlib.util.spec_from_file_location(module_name, module_path)
        module = importlib.util.module_from_spec(spec)
        sys.modules[module_name] = module
        spec.loader.exec_module(module)
        os.remove(module_path)
        return {"message": f"Module {module_name} loaded successfully"}
    except Exception as e:
        return JSONResponse(status_code=400, content={"message": f"Error loading module: {str(e)}"})
