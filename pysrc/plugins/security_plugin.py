from fastapi import FastAPI, APIRouter, HTTPException, Request, Response
from hashlib import sha256
from time import time
from starlette.middleware.base import BaseHTTPMiddleware
from collections import defaultdict
import logging

app = FastAPI()
router = APIRouter()

SECRET_KEY = "your_secret_key"
REQUEST_LIMIT = 100  # 每分钟允许的最大请求数
SIGNATURE_EXPIRATION = 300  # 签名有效期（秒）
IP_BLACKLIST = {"192.168.1.10", "10.0.0.1"}  # 示例黑名单

# 存储IP地址的请求计数和时间戳
request_counts = defaultdict(lambda: {"count": 0, "start_time": time()})

# 设置日志记录
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

async def verify_signature(request: Request):
    """
    Verifies the request signature.
    """
    signature = request.headers.get("X-Signature")
    timestamp = request.headers.get("X-Timestamp")
    data = await request.body()

    if not signature or not timestamp:
        raise HTTPException(status_code=400, detail="Missing signature or timestamp")

    if abs(time() - int(timestamp)) > SIGNATURE_EXPIRATION:
        raise HTTPException(status_code=403, detail="Signature expired")

    expected_signature = sha256((SECRET_KEY + timestamp + data.decode()).encode()).hexdigest()
    if expected_signature != signature:
        raise HTTPException(status_code=403, detail="Invalid signature")


async def rate_limiter(request: Request):
    """
    Rate limits incoming requests based on IP address.
    """
    client_ip = request.client.host
    if client_ip in IP_BLACKLIST:
        raise HTTPException(status_code=403, detail="Forbidden IP address")

    current_time = time()
    request_info = request_counts[client_ip]

    # Reset count every minute
    if current_time - request_info["start_time"] > 60:
        request_counts[client_ip] = {"count": 1, "start_time": current_time}
    else:
        request_info["count"] += 1

    if request_info["count"] > REQUEST_LIMIT:
        raise HTTPException(status_code=429, detail="Too many requests")


class SecurityMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        """
        Middleware to enhance security for incoming requests.
        """
        try:
            await rate_limiter(request)  # Check rate limits
            await verify_signature(request)  # Verify request signature
        except HTTPException as e:
            return Response(content=str(e.detail), status_code=e.status_code)

        # Log the request details
        logger.info(f"Request from {request.client.host} to {request.url.path} at {time()}")

        response = await call_next(request)
        return response


# Add the middleware to the FastAPI app
app.add_middleware(SecurityMiddleware)
