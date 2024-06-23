import time
import asyncio
from functools import wraps
from typing import Callable, Dict, Any, List, Optional, Union
from dataclasses import dataclass, field
from loguru import logger

@dataclass
class Command:
    handler: Callable[..., Any]
    description: str = ""
    aliases: List[str] = field(default_factory=list)
    permissions: List[str] = field(default_factory=list)
    cooldown: float = 0.0
    last_used: float = 0.0

class CommandDispatcher:
    def __init__(self):
        self._commands: Dict[str, Command] = {}
        self._middleware: List[Callable] = []
        self._events: Dict[str, List[Callable]] = {}

    def register_command(self, command_name: str, handler: Callable[..., Any], description: str = "", 
                         aliases: List[str] = [], permissions: List[str] = [], cooldown: float = 0.0):
        if command_name in self._commands:
            raise ValueError(f"Command {command_name} is already registered")
        self._commands[command_name] = Command(handler, description, aliases, permissions, cooldown)
        for alias in aliases:
            if alias in self._commands:
                raise ValueError(f"Alias {alias} is already used")
            self._commands[alias] = self._commands[command_name]
        logger.info(f"Registered command: {command_name} with aliases: {aliases}")

    def unregister_command(self, command_name: str):
        if command_name not in self._commands:
            raise ValueError(f"Command {command_name} is not registered")
        command = self._commands[command_name]
        del self._commands[command_name]
        for alias in command.aliases:
            del self._commands[alias]
        logger.info(f"Unregistered command: {command_name}")

    async def execute_command(self, command_name: str, *args, user_permissions: List[str] = [], **kwargs) -> Any:
        if command_name not in self._commands:
            raise ValueError(f"Command {command_name} is not registered")
        
        command = self._commands[command_name]
        
        if not set(command.permissions).issubset(set(user_permissions)):
            raise PermissionError(f"User doesn't have required permissions to execute {command_name}")
        
        current_time = time.time()
        if current_time - command.last_used < command.cooldown:
            raise ValueError(f"Command {command_name} is on cooldown")
        
        for middleware in self._middleware:
            args, kwargs = await self._execute_callable(middleware, *args, **kwargs)
        
        result = await self._execute_callable(command.handler, *args, **kwargs)
        command.last_used = current_time
        logger.info(f"Executed command: {command_name} with result: {result}")
        return result

    async def _execute_callable(self, callable: Callable, *args, **kwargs) -> Any:
        if asyncio.iscoroutinefunction(callable):
            return await callable(*args, **kwargs)
        else:
            return callable(*args, **kwargs)

    def get_command_info(self, command_name: str) -> Optional[Command]:
        return self._commands.get(command_name)

    def list_commands(self) -> Dict[str, str]:
        return {name: cmd.description for name, cmd in self._commands.items() if not name in sum((c.aliases for c in self._commands.values()), [])}

    def command(self, name: str, description: str = "", aliases: List[str] = [], permissions: List[str] = [], cooldown: float = 0.0):
        def decorator(func: Callable[..., Any]):
            self.register_command(name, func, description, aliases, permissions, cooldown)
            @wraps(func)
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)
            return wrapper
        return decorator

    def auto_register(self, module):
        import inspect
        for name, obj in inspect.getmembers(module):
            if inspect.isfunction(obj) and hasattr(obj, '_is_command'):
                self.register_command(name, obj, getattr(obj, '_description', ""), 
                                      getattr(obj, '_aliases', []), getattr(obj, '_permissions', []),getattr(obj, '_cooldown', 0.0))

    def add_middleware(self, middleware: Callable):
        self._middleware.append(middleware)
        logger.info(f"Added middleware: {middleware}")

    def remove_middleware(self, middleware: Callable):
        self._middleware.remove(middleware)
        logger.info(f"Removed middleware: {middleware}")

    def batch_execute(self, commands: List[Dict[str, Union[str, List, Dict]]]) -> List[Any]:
        async def execute_all():
            tasks = [
                self.execute_command(
                    cmd['name'], *cmd.get('args', []),
                    user_permissions=cmd.get('permissions', []),
                    **cmd.get('kwargs', {})
                ) for cmd in commands
            ]
            results = await asyncio.gather(*tasks, return_exceptions=True)
            return results

        results = asyncio.run(execute_all())
        for idx, result in enumerate(results):
            if isinstance(result, Exception):
                logger.error(f"Error executing command {commands[idx]['name']}: {str(result)}")
                results[idx] = None
        return results

    def add_event_listener(self, event_name: str, listener: Callable):
        if event_name not in self._events:
            self._events[event_name] = []
        self._events[event_name].append(listener)
        logger.info(f"Added event listener for event: {event_name}")

    def remove_event_listener(self, event_name: str, listener: Callable):
        if event_name in self._events and listener in self._events[event_name]:
            self._events[event_name].remove(listener)
            logger.info(f"Removed event listener for event: {event_name}")

    async def dispatch_event(self, event_name: str, *args, **kwargs):
        if event_name in self._events:
            for listener in self._events[event_name]:
                await self._execute_callable(listener, *args, **kwargs)
            logger.info(f"Dispatched event: {event_name} with args: {args}, kwargs: {kwargs}")

# Global instance of CommandDispatcher
command_dispatcher = CommandDispatcher()

def command(name: str, description: str = "", aliases: List[str] = [], permissions: List[str] = [], cooldown: float = 0.0):
    def decorator(func: Callable[..., Any]):
        func._is_command = True
        func._description = description
        func._aliases = aliases
        func._permissions = permissions
        func._cooldown = cooldown
        @wraps(func)
        def wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        return wrapper
    return decorator

def some_safe_function():
    return "This is a safe function accessible from the exec."

# Example middleware
async def logging_middleware(*args, **kwargs):
    logger.info(f"Command is being executed with args: {args}, kwargs: {kwargs}")
    return args, kwargs

command_dispatcher.add_middleware(logging_middleware)

# Example event listener
async def on_hello_executed(name: str):
    logger.info(f"Hello command executed with name: {name}")

command_dispatcher.add_event_listener("hello_executed", on_hello_executed)

# Example command registration
@command_dispatcher.command("hello", "Say hello", ["hi", "greet"], permissions=["basic"], cooldown=5.0)
async def hello(name: str = "World"):
    await asyncio.sleep(1)  # Simulate async operation
    return f"Hello, {name}!"