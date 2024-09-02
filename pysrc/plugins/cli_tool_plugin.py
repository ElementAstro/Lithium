import subprocess
from fastapi import APIRouter, HTTPException, Query, Body
from typing import Optional, List, Dict, Any
from loguru import logger
import os

router = APIRouter()

# 插件配置：定义默认的CLI工具路径和默认命令
CLI_TOOL_PATH = "/usr/bin/mycli"  # 替换为实际的命令行工具路径
DEFAULT_COMMAND = "status"  # 替换为实际的默认命令
MAX_RETRIES = 3  # 定义命令执行的最大重试次数

# 用于保存命令历史记录
command_history: List[Dict[str, Any]] = []


@router.get("/cli-tool/")
async def execute_cli_tool(
    command: Optional[str] = DEFAULT_COMMAND,
    tool_path: Optional[str] = Query(default=CLI_TOOL_PATH, description="Path to the CLI tool"),
    timeout: Optional[int] = Query(default=30, description="Command execution timeout in seconds"),
    retries: Optional[int] = Query(default=0, description="Number of retries on failure"),
    env_vars: Optional[Dict[str, str]] = Body(default=None, description="Environment variables for the command")
):
    """
    Endpoint to execute a CLI tool with a given command.

    Args:
        command (str): The command to execute with the CLI tool.
        tool_path (str): The path to the CLI tool.
        timeout (int): The execution timeout in seconds.
        retries (int): Number of retries on failure.
        env_vars (dict): Custom environment variables to use during command execution.

    Returns:
        dict: A dictionary containing the CLI tool's output or error message.
    """
    retries = min(retries, MAX_RETRIES)
    env = os.environ.copy()
    if env_vars:
        env.update(env_vars)
        logger.debug(f"Custom environment variables: {env_vars}")

    # 构建完整的命令
    full_command = [tool_path] + command.split()

    for attempt in range(retries + 1):
        try:
            logger.info(f"Executing command: {' '.join(full_command)}, attempt {attempt + 1}")

            # 使用 subprocess 运行命令行工具
            result = subprocess.run(full_command, capture_output=True, text=True, check=True, timeout=timeout, env=env)
            logger.info(f"Command executed successfully: {' '.join(full_command)}")

            # 保存命令历史记录
            command_history.append({
                "command": ' '.join(full_command),
                "output": result.stdout,
                "success": True,
                "attempt": attempt + 1
            })

            return {"output": result.stdout}

        except subprocess.TimeoutExpired:
            logger.error(f"Command '{' '.join(full_command)}' timed out after {timeout} seconds, attempt {attempt + 1}")
            command_history.append({
                "command": ' '.join(full_command),
                "output": "Timeout",
                "success": False,
                "attempt": attempt + 1
            })
            if attempt >= retries:
                raise HTTPException(status_code=504, detail="Command execution timed out")

        except subprocess.CalledProcessError as e:
            logger.error(f"Command failed with error: {e}, attempt {attempt + 1}")
            command_history.append({
                "command": ' '.join(full_command),
                "output": e.stderr,
                "success": False,
                "attempt": attempt + 1
            })
            if attempt >= retries:
                raise HTTPException(status_code=500, detail=f"Command execution failed: {e.stderr}")

        except FileNotFoundError:
            logger.error(f"CLI tool not found at path: {tool_path}")
            command_history.append({
                "command": ' '.join(full_command),
                "output": "CLI tool not found",
                "success": False,
                "attempt": attempt + 1
            })
            raise HTTPException(status_code=404, detail="CLI tool not found")


@router.get("/cli-tool/history")
async def get_command_history(command: Optional[str] = None):
    """
    Retrieve the history of executed commands and their outputs.

    Args:
        command (str): Optional command to filter history by.

    Returns:
        dict: A dictionary containing the history of executed commands.
    """
    logger.debug("Fetching command history")
    if command:
        filtered_history = [entry for entry in command_history if command in entry["command"]]
        logger.info(f"Found {len(filtered_history)} entries for command: {command}")
        return {"history": filtered_history}
    else:
        logger.info(f"Returning full command history, {len(command_history)} entries")
        return {"history": command_history}


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
