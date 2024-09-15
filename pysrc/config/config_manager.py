from dataclasses import dataclass
from typing import Any, Dict, Callable, List, Optional
import asyncio
import json
import copy


@dataclass
class Config:
    """Configuration class containing database URL, debug mode, and secret key."""
    database_url: str = "sqlite:///:memory:"
    debug: bool = False
    secret_key: str = "default_secret"


class ConfigManager:
    """Configuration manager class responsible for managing configuration updates, persistence, and subscriptions."""
    _instance = None
    _config: Config = Config()
    _getters: Dict[str, Callable[[], Any]] = {}
    _actions: Dict[str, Callable[..., Any]] = {}
    _subscribers: List[Callable[[Config], None]] = []
    _middleware: List[Callable[[Dict[str, Any], Callable], None]] = []
    _history: List[Config] = []
    _current_index: int = -1
    _persist: bool = False
    _storage_file: str = "config.json"
    _transaction_active: bool = False
    _transaction_backup: Optional[Config] = None
    _events: Dict[str, List[Callable[..., Any]]] = {}
    _updating: bool = False  # Used to avoid infinite loops

    def __new__(cls):
        """Ensure only one instance of ConfigManager exists (singleton pattern)."""
        if cls._instance is None:
            cls._instance = super(ConfigManager, cls).__new__(cls)
            cls._instance._load_persisted_state()
        return cls._instance

    def get_config(self) -> Config:
        """Get the current configuration."""
        return self._config

    def update_config(self, updates: Dict[str, Any], notify: bool = True) -> None:
        """Update the configuration and notify subscribers."""
        if self._transaction_active:
            self._apply_updates(updates)
        else:
            if not self._updating:
                self._updating = True
                self._apply_middleware(updates)
                self._apply_updates(updates)
                self._updating = False

            if notify:
                self._save_history()
                self._notify_subscribers()
                if self._persist:
                    self._persist_state()

    def _apply_updates(self, updates: Dict[str, Any]) -> None:
        """Apply updates to the configuration."""
        for key, value in updates.items():
            if hasattr(self._config, key):
                setattr(self._config, key, value)
            else:
                raise KeyError(f"Config has no attribute '{key}'")

    def _apply_middleware(self, updates: Dict[str, Any]) -> None:
        """Apply registered middleware during updates."""
        for middleware in self._middleware:
            middleware(updates, self._next)

    def _next(self, updates: Dict[str, Any]) -> None:
        """'Next' function used by middleware to continue updates."""
        self.update_config(updates)

    def use(self, plugin: Callable[['ConfigManager'], None]) -> None:
        """Load a plugin, which will receive the configManager instance and extend its functionality."""
        plugin(self)

    def register_middleware(self, middleware: Callable[[Dict[str, Any], Callable], None]) -> None:
        """Register middleware."""
        self._middleware.append(middleware)

    def register_getter(self, name: str, getter: Callable[[], Any]) -> None:
        """Register a getter."""
        self._getters[name] = getter

    def get_getter(self, name: str) -> Any:
        """Get a registered getter."""
        if name in self._getters:
            return self._getters[name]()
        raise KeyError(f"Getter '{name}' not found")

    def register_action(self, name: str, action: Callable[..., Any]) -> None:
        """Register an action."""
        self._actions[name] = action

    def dispatch_action(self, name: str, *args, **kwargs) -> Any:
        """Dispatch an action."""
        if name in self._actions:
            return self._actions[name](*args, **kwargs)
        raise KeyError(f"Action '{name}' not found")

    async def dispatch_async_action(self, name: str, *args, **kwargs) -> Any:
        """Dispatch an asynchronous action."""
        if name in self._actions:
            if asyncio.iscoroutinefunction(self._actions[name]):
                return await self._actions[name](*args, **kwargs)
            raise TypeError(f"Action '{name}' is not async")
        raise KeyError(f"Action '{name}' not found")

    def subscribe(self, subscriber: Callable[[Config], None], keys: Optional[List[str]] = None) -> None:
        """Subscribe to configuration updates, optionally filtering by specific keys."""
        if keys:
            def filtered_subscriber(config: Config):
                updated = {k: getattr(config, k)
                           for k in keys if hasattr(config, k)}
                subscriber(updated)
            self._subscribers.append(filtered_subscriber)
        else:
            self._subscribers.append(subscriber)

    def _save_history(self) -> None:
        """Save the current configuration state to history."""
        self._history.append(copy.deepcopy(self._config))
        self._current_index += 1

    def _notify_subscribers(self) -> None:
        """Notify all subscribers."""
        for subscriber in self._subscribers:
            subscriber(self._config)

    def enable_persistence(self, storage_file: str = "config.json", async_persist: bool = False) -> None:
        """Enable persistence, saving the state to a JSON file."""
        self._persist = True
        self._storage_file = storage_file
        if async_persist:
            asyncio.run(self._persist_state_async())
        else:
            self._persist_state()

    def _persist_state(self) -> None:
        """Persist the state to a JSON file."""
        with open(self._storage_file, "w", encoding="utf-8") as f:
            json.dump(self._config.__dict__, f)

    async def _persist_state_async(self) -> None:
        """Asynchronously persist the state to a JSON file."""
        await asyncio.to_thread(self._persist_state)

    def _load_persisted_state(self) -> None:
        """Load the persisted state from a JSON file."""
        try:
            with open(self._storage_file, "r", encoding="utf-8") as f:
                data = json.load(f)
                for key, value in data.items():
                    if hasattr(self._config, key):
                        setattr(self._config, key, value)
        except FileNotFoundError:
            pass

    def begin_transaction(self) -> None:
        """Begin a new transaction."""
        self._transaction_active = True
        self._transaction_backup = copy.deepcopy(self._config)

    def commit_transaction(self) -> None:
        """Commit the transaction."""
        self._transaction_active = False
        self._transaction_backup = None
        self._notify_subscribers()

    def rollback_transaction(self) -> None:
        """Rollback to the state before the transaction began."""
        if self._transaction_active and self._transaction_backup:
            self._config = self._transaction_backup
            self._transaction_active = False
            self._transaction_backup = None
            self._notify_subscribers()

    def on(self, event_name: str, handler: Callable[..., Any]) -> None:
        """Register an event handler for a specific event."""
        if event_name not in self._events:
            self._events[event_name] = []
        self._events[event_name].append(handler)

    def emit(self, event_name: str, *args, **kwargs) -> None:
        """Emit an event and call all registered handlers."""
        if event_name in self._events:
            for handler in self._events[event_name]:
                handler(*args, **kwargs)

    def register_module(self, module_name: str, module_config: Config) -> None:
        """Register a new module."""
        if not hasattr(self, '_modules'):
            self._modules = {}
        self._modules[module_name] = module_config

    def get_module(self, module_name: str) -> Optional[Config]:
        """Get the configuration of a module."""
        if hasattr(self, '_modules') and module_name in self._modules:
            return self._modules[module_name]
        raise KeyError(f"Module '{module_name}' not found")


def logger_plugin(manager: ConfigManager):
    """A plugin that logs all configuration updates."""
    def log_updates(config: Config):
        print(f"Logger: Config updated to {config}")

    manager.subscribe(log_updates)


def block_debug_middleware(updates: Dict[str, Any], next_middleware: Callable) -> None:
    """A middleware that blocks setting debug to True."""
    if updates.get("debug") is True:
        print("Middleware: Blocked 'debug' from being set to True.")
    else:
        next_middleware(updates)


if __name__ == "__main__":
    config_manager = ConfigManager()

    # Register middleware
    config_manager.register_middleware(block_debug_middleware)

    # Register a subscriber that only cares about database_url changes
    def database_url_subscriber(config: Dict[str, Any]):
        """Subscriber that handles database_url updates."""
        print(f"Database URL updated: {config.get('database_url')}")

    config_manager.subscribe(database_url_subscriber, keys=["database_url"])

    # Use a plugin
    config_manager.use(logger_plugin)

    # Enable persistence, saving configuration asynchronously
    config_manager.enable_persistence(async_persist=True)

    # Batch update configuration, automatically notifying subscribers
    config_manager.update_config(
        {"database_url": "postgresql://localhost", "debug": False})

    # Test middleware by attempting to update a blocked field
    # Middleware will block this update
    config_manager.update_config({"debug": True})

    # Example of module support
    @dataclass
    class CacheConfig:
        """Cache configuration class."""
        enabled: bool = True
        cache_size: int = 100

    cache_config = CacheConfig()
    config_manager.register_module("cache", cache_config)
    # Print the cache module's configuration
    print(config_manager.get_module("cache"))

    # Transaction mechanism, begin a transaction
    config_manager.begin_transaction()
    config_manager.update_config({"secret_key": "transaction_secret"})
    # Print configuration during transaction
    print(f"During transaction: {config_manager.get_config()}")
    config_manager.rollback_transaction()  # Rollback transaction
    # Should revert to original configuration
    print(f"After rollback: {config_manager.get_config()}")

    # Global event bus example
    def on_secret_key_change(new_key: str):
        """Event handler for secret_key changes."""
        print(f"Secret key changed to: {new_key}")

    config_manager.on("secret_key_change", on_secret_key_change)

    # Emit an event
    config_manager.emit("secret_key_change", "new_secret_key")

    # Example of asynchronous action
    async def async_reset_secret_key():
        """Asynchronously reset the secret_key."""
        print("Async: Resetting secret key...")
        await asyncio.sleep(1)
        config_manager.update_config({"secret_key": "default_secret"})

    config_manager.register_action(
        "reset_secret_key_async", async_reset_secret_key)

    # Dispatch asynchronous action
    asyncio.run(config_manager.dispatch_async_action("reset_secret_key_async"))
    print(config_manager.get_config())
