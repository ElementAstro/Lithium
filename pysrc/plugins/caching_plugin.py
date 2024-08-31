from fastapi import APIRouter, HTTPException
from cachetools import LRUCache, cached
from cachetools.keys import hashkey
from typing import Optional, Dict, Any
from loguru import logger

# 配置 loguru 日志系统
logger.add("caching_plugin.log", level="DEBUG",
           format="{time} {level} {message}", rotation="10 MB")

router = APIRouter()
cache = LRUCache(maxsize=100)  # LRU 缓存，最大容量为100


@router.get("/cache/get")
async def get_cache(key: str) -> Optional[str]:
    """
    Retrieves a value from the cache.
    """
    logger.debug(f"Retrieving value for key: {key}")
    value = cache.get(key)
    if value is not None:
        logger.info(f"Cache hit for key: {key}")
    else:
        logger.warning(f"Cache miss for key: {key}")
    return value


@router.post("/cache/set")
async def set_cache(key: str, value: str):
    """
    Sets a value in the cache.
    """
    logger.debug(f"Setting value for key: {key}, value: {value}")
    cache[key] = value
    logger.info(f"Value set in cache for key: {key}")
    return {"status": "success", "key": key, "value": value}


@router.delete("/cache/delete")
async def delete_cache(key: str):
    """
    Deletes a value from the cache.
    """
    logger.debug(f"Deleting value for key: {key}")
    if key in cache:
        del cache[key]
        logger.info(f"Value deleted from cache for key: {key}")
        return {"status": "success", "key": key}
    else:
        logger.warning(f"Attempted to delete non-existent key: {key}")
        return {"status": "error", "message": "Key not found"}


@router.get("/cache/stats")
async def cache_stats() -> Dict[str, Any]:
    """
    Returns cache statistics.
    """
    logger.debug("Fetching cache statistics")
    stats = {
        "maxsize": cache.maxsize,
        "currsize": cache.currsize,
        "hits": getattr(cache, 'hits', 0),
        "misses": getattr(cache, 'misses', 0)
    }
    logger.info(f"Cache statistics: {stats}")
    return stats


@router.post("/cache/clear")
async def clear_cache():
    """
    Clears all values from the cache.
    """
    logger.debug("Clearing all values from cache")
    cache.clear()
    logger.info("Cache cleared successfully")
    return {"status": "success", "message": "Cache cleared"}


def cache_hit_miss_decorator(func):
    """
    Decorator to track cache hits and misses.
    """
    cache.hits = 0
    cache.misses = 0

    @cached(cache=cache, key=lambda *args, **kwargs: hashkey(*args, **kwargs))
    async def wrapped(*args, **kwargs):
        key = hashkey(*args, **kwargs)
        if key in cache:
            cache.hits += 1
            logger.info(f"Cache hit for key: {key}")
        else:
            cache.misses += 1
            logger.info(f"Cache miss for key: {key}")
        return await func(*args, **kwargs)

    return wrapped

# Example decorated function


@cache_hit_miss_decorator
async def example_cached_function(key: str) -> str:
    """
    Example function that uses the cache with hit/miss tracking.
    """
    return f"Data for {key}"


@router.get("/cache/example")
async def get_example(key: str):
    """
    Retrieves data using the example cached function.
    """
    logger.debug(
        f"Retrieving data using example_cached_function for key: {key}")
    result = await example_cached_function(key)
    return {"key": key, "data": result}
