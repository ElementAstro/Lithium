"""
This module provides functionality to load, unload, and manage plugins for a FastAPI application.
"""

import os
import importlib
from typing import Dict, List
from fastapi import APIRouter, HTTPException, FastAPI
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import requests
from loguru import logger

loaded_plugins = {}


def load_plugins(directory: str, app: FastAPI):
    """
    Loads all plugins (Python modules) from a specified directory.
    """
    logger.info("Loading all plugins from directory: {}", directory)
    for filename in os.listdir(directory):
        if filename.endswith(".py") and filename != "__init__.py":
            plugin_name = filename[:-3]  # Strip '.py' extension
            load_plugin(directory, plugin_name, app)
    logger.info("Finished loading plugins from directory: {}", directory)


def load_plugin(directory: str, plugin_name: str, app: FastAPI):
    """
    Loads a single plugin (Python module) from a specified directory and adds its routes to the FastAPI app.
    """
    module_path = f"{directory}.{plugin_name}"

    if plugin_name in loaded_plugins:
        logger.warning("Plugin {} is already loaded.", plugin_name)
        return

    try:
        logger.info("Loading plugin: {}", plugin_name)
        module = importlib.import_module(module_path)
        if hasattr(module, "router") and isinstance(module.router, APIRouter):
            app.include_router(module.router)
            loaded_plugins[plugin_name] = module
            logger.info("Plugin {} loaded successfully.", plugin_name)
        elif isinstance(module.app, FastAPI):
            loaded_plugins[plugin_name] = module
            logger.info("Plugin {} loaded successfully.", plugin_name)
        else:
            logger.warning(
                "Plugin {} does not have a valid 'router' attribute.", plugin_name)
    except ImportError as e:
        logger.error("Failed to load plugin {}: {}", plugin_name, e)


def unload_plugin(plugin_name: str, app: FastAPI):
    """
    Unloads a plugin (Python module) and removes its routes from the FastAPI app.
    """
    if plugin_name not in loaded_plugins:
        logger.warning("Plugin {} is not loaded.", plugin_name)
        return

    try:
        logger.info("Unloading plugin: {}", plugin_name)
        module = loaded_plugins[plugin_name]
        if hasattr(module, "router") and isinstance(module.router, APIRouter):
            app.router.routes = [
                route for route in app.router.routes if route not in module.router.routes
            ]
            logger.info("Plugin {} unloaded successfully.", plugin_name)
        del loaded_plugins[plugin_name]
    except ImportError as e:
        logger.error("Failed to unload plugin {}: {}", plugin_name, e)


def reload_plugin(plugin_name: str, directory: str, app: FastAPI):
    """
    Reloads a plugin by unloading and loading it again.
    """
    logger.info("Reloading plugin: {}", plugin_name)
    unload_plugin(plugin_name, app)
    load_plugin(directory, plugin_name, app)
    logger.info("Plugin {} reloaded successfully.", plugin_name)


def list_plugins() -> List[str]:
    """
    Lists all currently loaded plugins.

    Returns:
        List[str]: A list of names of currently loaded plugins.
    """
    logger.info("Listing all loaded plugins.")
    return list(loaded_plugins.keys())


def get_plugin_status(plugin_name: str) -> Dict[str, str]:
    """
    Returns the status of a plugin (loaded or not loaded).

    Args:
        plugin_name (str): The name of the plugin to check.

    Returns:
        dict: A dictionary containing the plugin name and its status.
    """
    status = "loaded" if plugin_name in loaded_plugins else "not loaded"
    logger.info("Plugin {} status: {}", plugin_name, status)
    return {"plugin_name": plugin_name, "status": status}


def start_plugin_watcher(directory: str, app: FastAPI) -> Observer:
    """
    Starts a watchdog observer to watch for plugin file changes.
    """
    logger.info("Starting plugin watcher for directory: {}", directory)
    event_handler = PluginEventHandler(directory, app)
    observer = Observer()
    observer.schedule(event_handler, path=directory, recursive=False)
    observer.start()
    logger.info("Started watching for plugins in directory: {}", directory)
    return observer


def stop_plugin_watcher(observer: Observer):
    """
    Stops the plugin watcher.
    """
    logger.info("Stopping plugin watcher.")
    observer.stop()
    observer.join()
    logger.info("Stopped watching for plugins.")


def get_plugin_info(plugin_name: str) -> Dict:
    """
    Retrieves information about a loaded plugin, such as its available routes.

    Args:
        plugin_name (str): The name of the plugin to get info for.

    Returns:
        dict: A dictionary containing information about the plugin.
    """
    if plugin_name not in loaded_plugins:
        logger.error("Plugin {} not found.", plugin_name)
        raise HTTPException(
            status_code=404, detail=f"Plugin {plugin_name} not found")

    plugin = loaded_plugins[plugin_name]
    info = {
        "name": plugin_name,
        "routes": [route.path for route in plugin.router.routes] if hasattr(plugin, "router") else []
    }
    logger.info("Retrieved info for plugin {}: {}", plugin_name, info)
    return info


def update_plugin(plugin_name: str, app: FastAPI):
    """
    Forces an update or reloading of the plugin by re-importing the module.
    """
    if plugin_name not in loaded_plugins:
        logger.error("Plugin {} not found.", plugin_name)
        raise HTTPException(status_code=404, detail=f"Plugin {
                            plugin_name} not found")

    logger.info("Updating plugin: {}", plugin_name)
    # Unload the plugin first
    unload_plugin(plugin_name, app)

    # Reload the plugin
    directory = os.path.dirname(os.path.abspath(
        loaded_plugins[plugin_name].__file__))
    load_plugin(directory, plugin_name, app)
    logger.info("Plugin {} updated and reloaded.", plugin_name)


def install_plugin(plugin_name: str, repository: str, app: FastAPI):
    """
    Installs a plugin from a repository or external source.
    This could involve downloading the plugin file and placing it in the plugins directory.

    Args:
        plugin_name (str): The name of the plugin to install.
        repository (str): The URL or path to the repository from where to download the plugin.
    """
    plugin_url = f"{repository}/{plugin_name}.py"
    try:
        logger.info("Installing plugin {} from repository: {}",
                    plugin_name, repository)
        response = requests.get(plugin_url, timeout=10)
        response.raise_for_status()

        # Save the downloaded plugin file to the plugins directory
        plugin_path = os.path.join("plugins", f"{plugin_name}.py")
        with open(plugin_path, "wb") as f:
            f.write(response.content)

        logger.info("Plugin {} installed successfully from {}",
                    plugin_name, repository)
        load_plugin("plugins", plugin_name, app)
    except requests.RequestException as e:
        logger.error("Failed to install plugin {} from {}: {}",
                     plugin_name, repository, e)
        raise HTTPException(
            status_code=500, detail="Failed to install plugin") from e


def check_plugin_dependencies(plugin_name: str) -> List[str]:
    """
    Checks for any dependencies required by the plugin and verifies if they are met.

    Args:
        plugin_name (str): The name of the plugin to check dependencies for.

    Returns:
        List[str]: A list of missing dependencies, if any.
    """
    if plugin_name not in loaded_plugins:
        logger.error("Plugin {} not found.", plugin_name)
        raise HTTPException(status_code=404, detail=f"Plugin {
                            plugin_name} not found")

    module = loaded_plugins[plugin_name]
    if hasattr(module, "dependencies"):
        missing_dependencies = [
            dep for dep in module.dependencies if not _is_dependency_installed(dep)]
        logger.info("Checked dependencies for plugin {}: {}",
                    plugin_name, missing_dependencies)
        return missing_dependencies
    return []


def _is_dependency_installed(dependency: str) -> bool:
    """
    Helper function to check if a dependency is installed.

    Args:
        dependency (str): The name of the dependency to check.

    Returns:
        bool: True if the dependency is installed, False otherwise.
    """
    try:
        importlib.import_module(dependency)
        return True
    except ImportError:
        return False


class PluginEventHandler(FileSystemEventHandler):
    """
    Handles file system events to dynamically load or unload plugins.
    """

    def __init__(self, directory: str, app: FastAPI):
        self.directory = directory
        self.app = app

    def on_created(self, event):
        """
        Triggered when a new file is created.
        """
        if event.src_path.endswith(".py"):
            plugin_name = os.path.basename(event.src_path)[:-3]
            logger.info("Detected new plugin file created: {}", plugin_name)
            load_plugin(self.directory, plugin_name, self.app)

    def on_deleted(self, event):
        """
        Triggered when a file is deleted.
        """
        if event.src_path.endswith(".py"):
            plugin_name = os.path.basename(event.src_path)[:-3]
            logger.info("Detected plugin file deleted: {}", plugin_name)
            unload_plugin(plugin_name, self.app)

    def on_modified(self, event):
        """
        Triggered when a file is modified.
        """
        if event.src_path.endswith(".py"):
            plugin_name = os.path.basename(event.src_path)[:-3]
            logger.info("Detected plugin file modified: {}", plugin_name)
            reload_plugin(plugin_name, self.directory, self.app)
