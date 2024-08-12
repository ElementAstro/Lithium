from fastapi import FastAPI

app = FastAPI()

from . import routes  # 导入路由
