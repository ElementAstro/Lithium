from pydantic import BaseModel
from typing import List, Dict, Any

class CommandModel(BaseModel):
    name: str
    args: List[Any] = []
    kwargs: Dict[str, Any] = {}
    permissions: List[str] = []

class ExecPythonRequest(BaseModel):
    code: str
