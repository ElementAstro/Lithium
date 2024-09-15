from typing import Dict, Any, Callable, Awaitable, List, Optional
from loguru import logger
import inspect
import asyncio
from datetime import datetime, timedelta


class Command:
    def __init__(self, func: Callable, name: str, description: str, rate_limit: Optional[int] = None, aliases: Optional[List[str]] = None, permissions: Optional[List[str]] = None):
        self.func = func
        self.name = name
        self.description = description
        self.rate_limit = rate_limit
        self.aliases = aliases or []
        self.permissions = permissions or []
        self.last_called: Dict[str, datetime] = {}


class CommandDispatcher:
    def __init__(self):
        self.commands: Dict[str, Command] = {}
        self.middlewares: List[Callable] = []
        self.default_command: Optional[Command] = None

    def register(self, name: str, description: str = "", rate_limit: Optional[int] = None, aliases: Optional[List[str]] = None, permissions: Optional[List[str]] = None):
        def decorator(f: Callable[[Dict[str, Any]], Awaitable[Dict[str, Any]]]):
            command = Command(f, name, description,
                              rate_limit, aliases, permissions)
            self.commands[name] = command
            for alias in command.aliases:
                self.commands[alias] = command
            return f
        return decorator

    def set_default(self, f: Callable[[Dict[str, Any]], Awaitable[Dict[str, Any]]]):
        self.default_command = Command(f, "default", "Default command handler")

    def add_middleware(self, middleware: Callable):
        self.middlewares.append(middleware)

    async def dispatch(self, command_name: str, params: Dict[str, Any], user_id: str, user_permissions: Optional[List[str]] = None) -> Dict[str, Any]:
        command = self.commands.get(command_name, self.default_command)

        if command is None:
            return {"error": f"Unknown command: {command_name}"}

        # Check permissions
        if command.permissions and not any(perm in user_permissions for perm in command.permissions):
            return {"error": f"Permission denied for command: {command_name}"}

        # Apply rate limiting
        if command.rate_limit:
            last_called = command.last_called.get(user_id)
            if last_called and datetime.now() - last_called < timedelta(seconds=command.rate_limit):
                return {"error": f"Rate limit exceeded for command: {command_name}"}
            command.last_called[user_id] = datetime.now()

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
            return {"error": f"Command execution timed out"}
        except Exception as e:
            logger.error(f"Error executing command {command_name}: {e}")
            return {"error": f"Command execution failed: {str(e)}"}

    def get_command_list(self) -> List[Dict[str, Any]]:
        return [{"name": cmd.name, "description": cmd.description, "aliases": cmd.aliases, "permissions": cmd.permissions} for cmd in self.commands.values()]

    async def batch_dispatch(self, commands: List[Dict[str, Any]], user_id: str, user_permissions: Optional[List[str]] = None) -> List[Dict[str, Any]]:
        results = []
        for cmd in commands:
            result = await self.dispatch(cmd.get("command"), cmd.get("params", {}), user_id, user_permissions)
            results.append(result)
        return results

# Example usage:


dispatcher = CommandDispatcher()


@dispatcher.register("echo", "Echoes the input message", rate_limit=5, aliases=["repeat"], permissions=["user"])
async def echo_command(params: Dict[str, Any]) -> Dict[str, Any]:
    return {"result": params.get("message", "No message provided")}


@dispatcher.register("add", "Adds two numbers", permissions=["admin"])
def add_command(params: Dict[str, Any]) -> Dict[str, Any]:
    a = params.get("a", 0)
    b = params.get("b", 0)
    return {"result": a + b}


@dispatcher.set_default
async def default_command(params: Dict[str, Any]) -> Dict[str, Any]:
    return {"result": "Unknown command. Use 'help' to see available commands."}


async def log_middleware(command: str, params: Dict[str, Any], user_id: str) -> Dict[str, Any]:
    logger.info(f"User {user_id} is executing command: {command}")
    return params

dispatcher.add_middleware(log_middleware)

# Usage example:


async def main():
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
