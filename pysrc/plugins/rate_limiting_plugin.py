# plugins/rate_limiting_plugin.py

from fastapi import FastAPI, HTTPException, Request
from time import time
from collections import defaultdict
from starlette.middleware.base import BaseHTTPMiddleware

# 使用简单的字典来跟踪请求时间戳
request_times = defaultdict(list)
RATE_LIMIT = 10  # 每分钟最大请求数
TIME_WINDOW = 60  # 时间窗口，单位：秒

class RateLimitingMiddleware(BaseHTTPMiddleware):
    """
    Middleware to check rate limit for incoming requests.
    """
    async def dispatch(self, request: Request, call_next):
        client_ip = request.client.host
        current_time = time()
        
        # 过滤超过时间窗口的请求
        request_times[client_ip] = [
            timestamp for timestamp in request_times[client_ip] 
            if timestamp > current_time - TIME_WINDOW
        ]
        
        if len(request_times[client_ip]) >= RATE_LIMIT:
            raise HTTPException(status_code=429, detail="Rate limit exceeded")
        
        request_times[client_ip].append(current_time)
        response = await call_next(request)
        return response

# Then, you apply this middleware to the FastAPI app instance in your main application file.

app = FastAPI()

# Apply the rate limiting middleware
app.add_middleware(RateLimitingMiddleware)