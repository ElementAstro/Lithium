"""
This module provides a CommandDispatcher class for handling commands with rate limiting, permissions, and middlewares.
"""

import asyncio
import inspect
from datetime import datetime, timedelta
from typing import Dict, Any, Callable, Awaitable, List, Optional
from loguru import logger


class Command:
    """
    A class representing a command with rate limiting, aliases, and permissions.
    """

    def __init__(self, func: Callable, name: str, description: str,
                 rate_limit: Optional[int] = None, aliases: Optional[List[str]] = None,
                 permissions: Optional[List[str]] = None):
        """
        Initialize a Command instance.

        Args:
            func (Callable): The function to execute for this command.
            name (str): The name of the command.
            description (str): A brief description of the command.
            rate_limit (Optional[int]): The rate limit in seconds.
            aliases (Optional[List[str]]): A list of aliases for the command.
            permissions (Optional[List[str]]): A list of required permissions.
        """
        self.func = func
        self.name = name
        self.description = description
        self.rate_limit = rate_limit
        self.aliases = aliases or []
        self.permissions = permissions or []
        self.last_called: Dict[str, datetime] = {}

    def can_execute(self, user_id: str) -> bool:
        """
        Check if the command can be executed by the user based on rate limiting.

        Args:
            user_id (str): The ID of the user attempting to execute the command.

        Returns:
            bool: True if the command can be executed, False otherwise.
        """
        if self.rate_limit:
            last_called = self.last_called.get(user_id)
            if last_called and datetime.now() - last_called < timedelta(seconds=self.rate_limit):
                return False
        return True

    def update_last_called(self, user_id: str):
        """
        Update the last called time for the user.

        Args:
            user_id (str): The ID of the user.
        """
        self.last_called[user_id] = datetime.now()


class CommandDispatcher:
    """
    A class for dispatching commands with rate limiting, permissions, and middlewares.
    """

    def __init__(self):
        """
        Initialize a CommandDispatcher instance.
        """
        self.commands: Dict[str, Command] = {}
        self.middlewares: List[Callable] = []
        self.default_command: Optional[Command] = None

    def register(self, name: str, description: str = "", rate_limit: Optional[int] = None,
                 aliases: Optional[List[str]] = None, permissions: Optional[List[str]] = None):
        """
        Register a new command.

        Args:
            name (str): The name of the command.
            description (str): A brief description of the command.
            rate_limit (Optional[int]): The rate limit in seconds.
            aliases (Optional[List[str]]): A list of aliases for the command.
            permissions (Optional[List[str]]): A list of required permissions.

        Returns:
            Callable: A decorator for the command function.
        """
        def decorator(f: Callable[[Dict[str, Any]], Awaitable[Dict[str, Any]]]):
            command = Command(f, name, description,
                              rate_limit, aliases, permissions)
            self.commands[name] = command
            for alias in command.aliases:
                self.commands[alias] = command
            return f
        return decorator

    def set_default(self, f: Callable[[Dict[str, Any]], Awaitable[Dict[str, Any]]]):
        """
        Set the default command handler.

        Args:
            f (Callable): The default command function.
        """
        self.default_command = Command(f, "default", "Default command handler")

    def add_middleware(self, middleware: Callable):
        """
        Add a middleware to be applied to all commands.

        Args:
            middleware (Callable): The middleware function.
        """
        self.middlewares.append(middleware)

    async def dispatch(self, command_name: str, params: Dict[str, Any], user_id: str,
                       user_permissions: Optional[List[str]] = None) -> Dict[str, Any]:
        """
        Dispatch a command.

        Args:
            command_name (str): The name of the command.
            params (Dict[str, Any]): The parameters for the command.
            user_id (str): The ID of the user executing the command.
            user_permissions (Optional[List[str]]): The permissions of the user.

        Returns:
            Dict[str, Any]: The result of the command execution.
        """
        command = self.commands.get(command_name, self.default_command)

        if command is None:
            return {"error": f"Unknown command: {command_name}"}

        # Check permissions
        if command.permissions and not any(perm in user_permissions for perm in command.permissions):
            return {"error": f"Permission denied for command: {command_name}"}

        # Apply rate limiting
        if not command.can_execute(user_id):
            return {"error": f"Rate limit exceeded for command: {command_name}"}
        command.update_last_called(user_id)

        # Apply middlewares
        for middleware in self.middlewares:
            params = await middleware(command_name, params, user_id)

        try:
            # Check if the command function is asynchronous
            if inspect.iscoroutinefunction(command.func):
                result = await asyncio.wait_for(command.func(params), timeout=10)
            else:
                # If it's not async, run it in an executor
                loop = asyncio.get_event_loop()
                result = await asyncio.wait_for(loop.run_in_executor(None, command.func, params), timeout=10)

            # Log command execution
            logger.info(f"Command {command_name} executed by user {
                        user_id} with result: {result}")
            return result
        except asyncio.TimeoutError:
            logger.error(f"Command {command_name} execution timed out")
            return {"error": "Command execution timed out"}
        except (TypeError, ValueError) as e:
            logger.error(f"Error executing command {command_name}: {e}")
            return {"error": f"Command execution failed: {str(e)}"}

    def get_command_list(self) -> List[Dict[str, Any]]:
        """
        Get the list of registered commands.

        Returns:
            List[Dict[str, Any]]: A list of command details.
        """
        return [{"name": cmd.name, "description": cmd.description, "aliases": cmd.aliases, "permissions": cmd.permissions}
                for cmd in self.commands.values()]

    async def batch_dispatch(self, commands: List[Dict[str, Any]], user_id: str,
                             user_permissions: Optional[List[str]] = None) -> List[Dict[str, Any]]:
        """
        Dispatch multiple commands in a batch.

        Args:
            commands (List[Dict[str, Any]]): A list of commands to dispatch.
            user_id (str): The ID of the user executing the commands.
            user_permissions (Optional[List[str]]): The permissions of the user.

        Returns:
            List[Dict[str, Any]]: A list of results from the command executions.
        """
        results = []
        for cmd in commands:
            result = await self.dispatch(cmd.get("command"), cmd.get("params", {}), user_id, user_permissions)
            results.append(result)
        return results


# Example usage:

dispatcher = CommandDispatcher()


@dispatcher.register("echo", "Echoes the input message", rate_limit=5, aliases=["repeat"], permissions=["user"])
async def echo_command(params: Dict[str, Any]) -> Dict[str, Any]:
    """
    Echo the input message.

    Args:
        params (Dict[str, Any]): The parameters for the command.

    Returns:
        Dict[str, Any]: The result of the command.
    """
    return {"result": params.get("message", "No message provided")}


@dispatcher.register("add", "Adds two numbers", permissions=["admin"])
def add_command(params: Dict[str, Any]) -> Dict[str, Any]:
    """
    Add two numbers.

    Args:
        params (Dict[str, Any]): The parameters for the command.

    Returns:
        Dict[str, Any]: The result of the command.
    """
    a = params.get("a", 0)
    b = params.get("b", 0)
    return {"result": a + b}


@dispatcher.set_default
async def default_command(params: Dict[str, Any]) -> Dict[str, Any]:
    """
    Handle unknown commands.

    Args:
        params (Dict[str, Any]): The parameters for the command.

    Returns:
        Dict[str, Any]: The result of the command.
    """
    return {"result": "Unknown command. Use 'help' to see available commands."}


async def log_middleware(command: str, params: Dict[str, Any], user_id: str) -> Dict[str, Any]:
    """
    Log the execution of a command.

    Args:
        command (str): The name of the command.
        params (Dict[str, Any]): The parameters for the command.
        user_id (str): The ID of the user executing the command.

    Returns:
        Dict[str, Any]: The parameters for the command.
    """
    logger.info(f"User {user_id} is executing command: {command}")
    return params

dispatcher.add_middleware(log_middleware)

# Usage example:


async def main():
    """
    Main function to run the command dispatcher.
    """
    result = await dispatcher.dispatch("echo", {"message": "Hello, World!"}, "user1", ["user"])
    print(result)  # {"result": "Hello, World!"}

    result = await dispatcher.dispatch("add", {"a": 5, "b": 3}, "user1", ["admin"])
    print(result)  # {"result": 8}

    result = await dispatcher.dispatch("unknown", {}, "user1", ["user"])
    # {"result": "Unknown command. Use 'help' to see available commands."}
    print(result)

    command_list = dispatcher.get_command_list()
    # [{"name": "echo", "description": "Echoes the input message", "aliases": ["repeat"], "permissions": ["user"]}, {"name": "add", "description": "Adds two numbers", "permissions": ["admin"]}]
    print(command_list)

    batch_results = await dispatcher.batch_dispatch([
        {"command": "echo", "params": {"message": "First command"}},
        {"command": "add", "params": {"a": 10, "b": 20}}
    ], "user1", ["admin"])
    print(batch_results)  # [{"result": "First command"}, {"result": 30}]

if __name__ == "__main__":
    asyncio.run(main())
