# config.py
from pydantic import BaseModel

class Config(BaseModel):
    """
    Configuration model for managing dynamic settings.
    """
    broadcast_interval: int = 3
    max_connections: int = 100
    auth_username: str = "admin"
    auth_password: str = "password"
    plugin_directory: str = "plugins"  # Directory to load dynamic routes from

# Instantiate configuration
config = Config()
