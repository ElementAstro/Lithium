from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import Dict, Set

app = FastAPI()

# 配置CORS中间件
origins = [
    "http://localhost",
    "http://localhost:8000",
    "http://127.0.0.1:9527",  # 添加这个
    "http://yourdomain.com",  # 你希望允许访问的其他域名
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 模拟数据库
fake_db = {
    "users": {"user1": {"password": "lithium", "info": {"name": "User One", "email": "user1@example.com"}}},
    "tokens": {"valid_refresh_token": "new_auth_token"}
}

blacklist: Set[str] = set()

# 定义请求和响应模型
class LoginRequest(BaseModel):
    userName: str
    password: str

class LoginToken(BaseModel):
    token: str

class UserInfo(BaseModel):
    name: str
    email: str

class RefreshTokenRequest(BaseModel):
    refreshToken: str

class CustomErrorRequest(BaseModel):
    code: str
    msg: str

class RegisterRequest(BaseModel):
    userName: str
    password: str
    email: str

class ChangePasswordRequest(BaseModel):
    userName: str
    oldPassword: str
    newPassword: str

# 用户注册端点
@app.post("/auth/register", response_model=LoginToken)
def register(request: RegisterRequest):
    if request.userName in fake_db["users"]:
        raise HTTPException(status_code=400, detail="Username already exists")
    fake_db["users"][request.userName] = {
        "password": request.password,
        "info": {"name": request.userName, "email": request.email}
    }
    return {"token": f"auth_token_for_{request.userName}"}

# 登录端点
@app.post("/auth/login", response_model=LoginToken)
def login(request: LoginRequest):
    user = fake_db["users"].get(request.userName)
    if user and user["password"] == request.password:
        return {"token": f"auth_token_for_{request.userName}"}
    raise HTTPException(status_code=0000, detail="Invalid username or password")

# 获取用户信息端点
@app.get("/auth/getUserInfo", response_model=UserInfo)
def get_user_info():
    # 这里假设用户已经通过验证，并且我们能获取到用户信息
    # 在实际应用中，你可能需要从请求头中提取和验证授权令牌
    fake_user_info = fake_db["users"]["user1"]["info"]
    return fake_user_info

# 刷新令牌端点
@app.post("/auth/refreshToken", response_model=LoginToken)
def refresh_token(request: RefreshTokenRequest):
    if request.refreshToken in fake_db["tokens"]:
        new_token = fake_db["tokens"][request.refreshToken]
        return {"token": new_token}
    raise HTTPException(status_code=401, detail="Invalid refresh token")

# 注销端点
@app.post("/auth/logout")
def logout(token: str):
    blacklist.add(token)
    return {"message": "Logged out successfully"}

# 修改密码端点
@app.post("/auth/changePassword")
def change_password(request: ChangePasswordRequest):
    user = fake_db["users"].get(request.userName)
    if user and user["password"] == request.oldPassword:
        user["password"] = request.newPassword
        return {"message": "Password changed successfully"}
    raise HTTPException(status_code=401, detail="Invalid username or password")

# 获取所有用户信息（管理员功能）
@app.get("/auth/users", response_model=Dict[str, UserInfo])
def get_all_users():
    return {user: data["info"] for user, data in fake_db["users"].items()}

# 自定义错误处理端点
@app.get("/auth/error")
def custom_error(code: str, msg: str):
    raise HTTPException(status_code=int(code), detail=msg)

# 运行应用
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)