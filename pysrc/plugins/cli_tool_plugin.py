import subprocess
from fastapi import APIRouter, HTTPException, Query, Body, BackgroundTasks
from typing import Optional, List, Dict, Any
from loguru import logger
import os
import time
import json

router = APIRouter()

# 插件配置：定义不同CLI工具的路径和默认命令
CLI_TOOLS = {
    "tool1": {
        "path": "/usr/bin/tool1",  # 替换为实际的命令行工具路径
        "default_command": "status"
    },
    "tool2": {
        "path": "/usr/bin/tool2",  # 替换为实际的命令行工具路径
        "default_command": "info"
    }
}
MAX_RETRIES = 3  # 定义命令执行的最大重试次数

# 用于保存命令历史记录
command_history: List[Dict[str, Any]] = []

# 命令别名和权限
command_aliases = {
    "status": "status",
    "info": "status",
    "start": "run",
    "stop": "terminate"
}

command_permissions = {
    "status": ["admin", "user"],
    "run": ["admin"],
    "terminate": ["admin"]
}

# 用于保存正在执行的命令
running_commands: Dict[str, subprocess.Popen] = {}


@router.get("/cli-tool/")
async def execute_cli_tool(
    tool_name: str = Query(..., description="Name of the CLI tool to use"),
    command: Optional[str] = None,
    timeout: Optional[int] = Query(
        default=30, description="Command execution timeout in seconds"),
    retries: Optional[int] = Query(
        default=0, description="Number of retries on failure"),
    env_vars: Optional[Dict[str, str]] = Body(
        default=None, description="Environment variables for the command"),
    output_format: Optional[str] = Query(
        default="text", description="Output format: 'text' or 'json'"),
    user_role: Optional[str] = Query(
        default="user", description="User role for permission check"),
):
    """
    Endpoint to execute a CLI tool with a given command.

    Args:
        tool_name (str): The name of the CLI tool to use.
        command (str): The command to execute with the CLI tool.
        timeout (int): The execution timeout in seconds.
        retries (int): Number of retries on failure.
        env_vars (dict): Custom environment variables to use during command execution.
        output_format (str): The format of the command output ('text' or 'json').
        user_role (str): The role of the user executing the command.

    Returns:
        dict: A dictionary containing the CLI tool's output or error message.
    """
    if tool_name not in CLI_TOOLS:
        raise HTTPException(status_code=404, detail="CLI tool not found")

    tool_path = CLI_TOOLS[tool_name]["path"]
    default_command = CLI_TOOLS[tool_name]["default_command"]
    command = command or default_command

    # 检查命令别名
    command = command_aliases.get(command, command)

    # 检查命令权限
    if command not in command_permissions or user_role not in command_permissions[command]:
        raise HTTPException(
            status_code=403, detail="Permission denied for this command")

    retries = min(retries, MAX_RETRIES)
    env = os.environ.copy()
    if env_vars:
        env.update(env_vars)
        logger.debug(f"Custom environment variables: {env_vars}")

    # 构建完整的命令
    full_command = [tool_path] + command.split()

    for attempt in range(retries + 1):
        try:
            logger.info(f"Executing command: {' '.join(
                full_command)}, attempt {attempt + 1}")

            # 使用 subprocess.Popen 运行命令行工具
            process = subprocess.Popen(
                full_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, env=env)
            running_commands[command] = process

            try:
                stdout, stderr = process.communicate(timeout=timeout)
            except subprocess.TimeoutExpired:
                process.kill()
                stdout, stderr = process.communicate()
                raise subprocess.TimeoutExpired(
                    cmd=full_command, timeout=timeout)

            if process.returncode != 0:
                raise subprocess.CalledProcessError(
                    returncode=process.returncode, cmd=full_command, output=stdout, stderr=stderr)

            logger.info(f"Command executed successfully: {
                        ' '.join(full_command)}")

            # 保存命令历史记录
            command_history.append({
                "tool_name": tool_name,
                "command": ' '.join(full_command),
                "output": stdout,
                "success": True,
                "attempt": attempt + 1,
                "timestamp": time.time()
            })

            # 格式化输出
            if output_format == "json":
                return {"output": json.loads(stdout)}
            else:
                return {"output": stdout}

        except subprocess.TimeoutExpired:
            logger.error(f"Command '{' '.join(full_command)}' timed out after {
                         timeout} seconds, attempt {attempt + 1}")
            command_history.append({
                "tool_name": tool_name,
                "command": ' '.join(full_command),
                "output": "Timeout",
                "success": False,
                "attempt": attempt + 1,
                "timestamp": time.time()
            })
            if attempt >= retries:
                raise HTTPException(
                    status_code=504, detail="Command execution timed out")

        except subprocess.CalledProcessError as e:
            logger.error(f"Command failed with error: {
                         e}, attempt {attempt + 1}")
            command_history.append({
                "tool_name": tool_name,
                "command": ' '.join(full_command),
                "output": e.stderr,
                "success": False,
                "attempt": attempt + 1,
                "timestamp": time.time()
            })
            if attempt >= retries:
                raise HTTPException(
                    status_code=500, detail=f"Command execution failed: {e.stderr}")

        except FileNotFoundError:
            logger.error(f"CLI tool not found at path: {tool_path}")
            command_history.append({
                "tool_name": tool_name,
                "command": ' '.join(full_command),
                "output": "CLI tool not found",
                "success": False,
                "attempt": attempt + 1,
                "timestamp": time.time()
            })
            raise HTTPException(status_code=404, detail="CLI tool not found")

        finally:
            if command in running_commands:
                del running_commands[command]


@router.get("/cli-tool/history")
async def get_command_history(tool_name: Optional[str] = None, command: Optional[str] = None):
    """
    Retrieve the history of executed commands and their outputs.

    Args:
        tool_name (str): Optional tool name to filter history by.
        command (str): Optional command to filter history by.

    Returns:
        dict: A dictionary containing the history of executed commands.
    """
    logger.debug("Fetching command history")
    filtered_history = command_history
    if tool_name:
        filtered_history = [
            entry for entry in filtered_history if entry["tool_name"] == tool_name]
    if command:
        filtered_history = [
            entry for entry in filtered_history if command in entry["command"]]
    logger.info(f"Found {len(filtered_history)} entries for tool: {
                tool_name}, command: {command}")
    return {"history": filtered_history}


@router.delete("/cli-tool/history")
async def clear_command_history():
    """
    Clears the history of executed commands.

    Returns:
        dict: A success message confirming history deletion.
    """
    global command_history
    logger.debug("Clearing command history")
    command_history = []
    logger.info("Command history cleared")
    return {"status": "success", "message": "Command history cleared"}


@router.delete("/cli-tool/cancel")
async def cancel_command(command: str):
    """
    Cancels a running command.

    Args:
        command (str): The command to cancel.

    Returns:
        dict: A success message confirming command cancellation.
    """
    if command in running_commands:
        process = running_commands[command]
        process.terminate()
        logger.info(f"Cancelled command: {command}")
        return {"status": "success", "message": f"Cancelled command: {command}"}
    else:
        logger.warning(f"Command not found or not running: {command}")
        raise HTTPException(
            status_code=404, detail="Command not found or not running")


@router.get("/cli-tool/tools")
async def get_cli_tools():
    """
    Retrieve the list of available CLI tools.

    Returns:
        dict: A dictionary containing the available CLI tools.
    """
    logger.debug("Fetching CLI tools list")
    tools_list = [{"name": name, "path": info["path"],
                   "default_command": info["default_command"]} for name, info in CLI_TOOLS.items()]
    logger.info(f"Available CLI tools: {tools_list}")
    return {"tools": tools_list}
