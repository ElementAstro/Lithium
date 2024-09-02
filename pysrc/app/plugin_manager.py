import os
import importlib
import logging
from typing import Dict, List
from fastapi import APIRouter, HTTPException, FastAPI
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

logger = logging.getLogger(__name__)

loaded_plugins = {}


def load_plugins(directory: str, app):
    """
    Loads all plugins (Python modules) from a specified directory.
    """
    global loaded_plugins
    for filename in os.listdir(directory):
        if filename.endswith(".py") and filename != "__init__.py":
            plugin_name = filename[:-3]  # Strip '.py' extension
            load_plugin(directory, plugin_name, app)


def load_plugin(directory: str, plugin_name: str, app):
    """
    Loads a single plugin (Python module) from a specified directory and adds its routes to the FastAPI app.
    """
    global loaded_plugins
    module_path = f"{directory}.{plugin_name}"

    if plugin_name in loaded_plugins:
        logger.warning(f"Plugin {plugin_name} is already loaded.")
        return

    try:
        module = importlib.import_module(module_path)
        if (hasattr(module, "router") and isinstance(module.router, APIRouter)):
            app.include_router(module.router)
            loaded_plugins[plugin_name] = module
            logger.info(f"Plugin {plugin_name} loaded successfully.")
        elif isinstance(module.app, FastAPI):
            loaded_plugins[plugin_name] = module
            logger.info(f"Plugin {plugin_name} loaded successfully.")
        else:
            logger.warning(
                f"Plugin {plugin_name} does not have a valid 'router' attribute.")
    except Exception as e:
        logger.error(f"Failed to load plugin {plugin_name}: {e}")


def unload_plugin(plugin_name: str, app):
    """
    Unloads a plugin (Python module) and removes its routes from the FastAPI app.
    """
    global loaded_plugins
    if plugin_name not in loaded_plugins:
        logger.warning(f"Plugin {plugin_name} is not loaded.")
        return

    try:
        module = loaded_plugins[plugin_name]
        if (hasattr(module, "router") and isinstance(module.router, APIRouter)):
            app.router.routes = [
                route for route in app.router.routes if route not in module.router.routes
            ]
            logger.info(f"Plugin {plugin_name} unloaded successfully.")
        del loaded_plugins[plugin_name]
    except Exception as e:
        logger.error(f"Failed to unload plugin {plugin_name}: {e}")


def reload_plugin(plugin_name: str, directory: str, app):
    """
    Reloads a plugin by unloading and loading it again.
    """
    unload_plugin(plugin_name, app)
    load_plugin(directory, plugin_name, app)


def list_plugins() -> List[str]:
    """
    Lists all currently loaded plugins.

    Returns:
        List[str]: A list of names of currently loaded plugins.
    """
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
    return {"plugin_name": plugin_name, "status": status}


def start_plugin_watcher(directory: str, app):
    """
    Starts a watchdog observer to watch for plugin file changes.
    """
    event_handler = PluginEventHandler(directory, app)
    observer = Observer()
    observer.schedule(event_handler, path=directory, recursive=False)
    observer.start()
    logger.info(f"Started watching for plugins in directory: {directory}")
    return observer


def stop_plugin_watcher(observer: Observer):
    """
    Stops the plugin watcher.
    """
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
        raise HTTPException(
            status_code=404, detail=f"Plugin {plugin_name} not found")

    plugin = loaded_plugins[plugin_name]
    info = {
        "name": plugin_name,
        "routes": [route.path for route in plugin.router.routes] if hasattr(plugin, "router") else []
    }
    return info


def update_plugin(plugin_name: str, app):
    """
    Forces an update or reloading of the plugin by re-importing the module.
    """
    if plugin_name not in loaded_plugins:
        raise HTTPException(
            status_code=404, detail=f"Plugin {plugin_name} not found")

    # Unload the plugin first
    unload_plugin(plugin_name, app)

    # Reload the plugin
    directory = os.path.dirname(os.path.abspath(
        loaded_plugins[plugin_name].__file__))
    load_plugin(directory, plugin_name, app)
    logger.info(f"Plugin {plugin_name} updated and reloaded.")


def install_plugin(plugin_name: str, repository: str, app):
    """
    Installs a plugin from a repository or external source.
    This could involve downloading the plugin file and placing it in the plugins directory.

    Args:
        plugin_name (str): The name of the plugin to install.
        repository (str): The URL or path to the repository from where to download the plugin.
    """
    # Example implementation: downloading a file from a repository
    import requests

    plugin_url = f"{repository}/{plugin_name}.py"
    try:
        response = requests.get(plugin_url)
        response.raise_for_status()

        # Save the downloaded plugin file to the plugins directory
        plugin_path = os.path.join("plugins", f"{plugin_name}.py")
        with open(plugin_path, "wb") as f:
            f.write(response.content)

        logger.info(
            f"Plugin {plugin_name} installed successfully from {repository}")
        load_plugin("plugins", plugin_name, app)
    except Exception as e:
        logger.error(
            f"Failed to install plugin {plugin_name} from {repository}: {e}")
        raise HTTPException(status_code=500, detail="Failed to install plugin")


def check_plugin_dependencies(plugin_name: str) -> List[str]:
    """
    Checks for any dependencies required by the plugin and verifies if they are met.

    Args:
        plugin_name (str): The name of the plugin to check dependencies for.

    Returns:
        List[str]: A list of missing dependencies, if any.
    """
    if plugin_name not in loaded_plugins:
        raise HTTPException(
            status_code=404, detail=f"Plugin {plugin_name} not found")

    module = loaded_plugins[plugin_name]
    if hasattr(module, "dependencies"):
        missing_dependencies = [
            dep for dep in module.dependencies if not _is_dependency_installed(dep)]
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

    def __init__(self, directory: str, app):
        self.directory = directory
        self.app = app

    def on_created(self, event):
        """
        Triggered when a new file is created.
        """
        if event.src_path.endswith(".py"):
            plugin_name = os.path.basename(event.src_path)[:-3]
            load_plugin(self.directory, plugin_name, self.app)

    def on_deleted(self, event):
        """
        Triggered when a file is deleted.
        """
        if event.src_path.endswith(".py"):
            plugin_name = os.path.basename(event.src_path)[:-3]
            unload_plugin(plugin_name, self.app)

    def on_modified(self, event):
        """
        Triggered when a file is modified.
        """
        if event.src_path.endswith(".py"):
            plugin_name = os.path.basename(event.src_path)[:-3]
            reload_plugin(plugin_name, self.directory, self.app)
