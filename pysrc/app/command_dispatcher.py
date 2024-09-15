# app/command_dispatcher.py
from typing import Dict, Any, Callable, Awaitable, List, Optional
from loguru import logger
import inspect
import asyncio
from datetime import datetime, timedelta


class Command:
    def __init__(self, func: Callable, name: str, description: str, rate_limit: Optional[int] = None):
        self.func = func
        self.name = name
        self.description = description
        self.rate_limit = rate_limit
        self.last_called: Dict[str, datetime] = {}


class CommandDispatcher:
    def __init__(self):
        self.commands: Dict[str, Command] = {}
        self.middlewares: List[Callable] = []
        self.default_command: Optional[Command] = None

    def register(self, name: str, description: str = "", rate_limit: Optional[int] = None):
        def decorator(f: Callable[[Dict[str, Any]], Awaitable[Dict[str, Any]]]):
            self.commands[name] = Command(f, name, description, rate_limit)
            return f
        return decorator

    def set_default(self, f: Callable[[Dict[str, Any]], Awaitable[Dict[str, Any]]]):
        self.default_command = Command(f, "default", "Default command handler")

    def add_middleware(self, middleware: Callable):
        self.middlewares.append(middleware)

    async def dispatch(self, command_name: str, params: Dict[str, Any], user_id: str) -> Dict[str, Any]:
        command = self.commands.get(command_name, self.default_command)

        if command is None:
            return {"error": f"Unknown command: {command_name}"}

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
                result = await command.func(params)
            else:
                # If it's not async, run it in an executor
                loop = asyncio.get_event_loop()
                result = await loop.run_in_executor(None, command.func, params)

            return result
        except Exception as e:
            logger.error(f"Error executing command {command_name}: {e}")
            return {"error": f"Command execution failed: {str(e)}"}

    def get_command_list(self) -> List[Dict[str, Any]]:
        return [{"name": cmd.name, "description": cmd.description} for cmd in self.commands.values()]

    async def batch_dispatch(self, commands: List[Dict[str, Any]], user_id: str) -> List[Dict[str, Any]]:
        results = []
        for cmd in commands:
            result = await self.dispatch(cmd.get("command"), cmd.get("params", {}), user_id)
            results.append(result)
        return results

# Example usage:


dispatcher = CommandDispatcher()


@dispatcher.register("echo", "Echoes the input message", rate_limit=5)
async def echo_command(params: Dict[str, Any]) -> Dict[str, Any]:
    return {"result": params.get("message", "No message provided")}


@dispatcher.register("add", "Adds two numbers")
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
    result = await dispatcher.dispatch("echo", {"message": "Hello, World!"}, "user1")
    print(result)  # {"result": "Hello, World!"}

    result = await dispatcher.dispatch("add", {"a": 5, "b": 3}, "user1")
    print(result)  # {"result": 8}

    result = await dispatcher.dispatch("unknown", {}, "user1")
    # {"result": "Unknown command. Use 'help' to see available commands."}
    print(result)

    command_list = dispatcher.get_command_list()
    # [{"name": "echo", "description": "Echoes the input message"}, {"name": "add", "description": "Adds two numbers"}]
    print(command_list)

    batch_results = await dispatcher.batch_dispatch([
        {"command": "echo", "params": {"message": "First command"}},
        {"command": "add", "params": {"a": 10, "b": 20}}
    ], "user1")
    print(batch_results)  # [{"result": "First command"}, {"result": 30}]

if __name__ == "__main__":
    asyncio.run(main())
