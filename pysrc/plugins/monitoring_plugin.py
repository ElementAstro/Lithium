import os
import psutil
import time
from fastapi import APIRouter, HTTPException, Query
from typing import List, Dict
from loguru import logger

# 配置 loguru 日志系统
logger.add("monitoring.log", level="DEBUG",
           format="{time} {level} {message}", rotation="10 MB")

router = APIRouter()


@router.get("/monitoring/status")
async def get_server_status():
    """
    Returns the current server status including CPU, memory, and disk usage.
    """
    try:
        status = {
            "cpu_usage": psutil.cpu_percent(interval=1),
            "memory_usage": psutil.virtual_memory().percent,
            "disk_usage": psutil.disk_usage('/').percent,
            "uptime": time.time() - psutil.boot_time()
        }
        logger.info(f"Retrieved server status: {status}")
        return status
    except Exception as e:
        logger.error(f"Failed to retrieve server status: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to retrieve server status")


@router.get("/monitoring/requests")
async def get_request_statistics():
    """
    Returns statistics about the number of requests handled.
    """
    # Example implementation, replace with actual statistics gathering
    try:
        stats = {
            "total_requests": 100,
            "requests_per_second": 2,
            "successful_requests": 95,
            "failed_requests": 5,
            "average_response_time_ms": 200
        }
        logger.info(f"Retrieved request statistics: {stats}")
        return stats
    except Exception as e:
        logger.error(f"Failed to retrieve request statistics: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to retrieve request statistics")


@router.get("/monitoring/network")
async def get_network_statistics():
    """
    Returns current network statistics including bytes sent/received and connection count.
    """
    try:
        net_io = psutil.net_io_counters()
        connections = psutil.net_connections()
        network_stats = {
            "bytes_sent": net_io.bytes_sent,
            "bytes_received": net_io.bytes_recv,
            "active_connections": len(connections)
        }
        logger.info(f"Retrieved network statistics: {network_stats}")
        return network_stats
    except Exception as e:
        logger.error(f"Failed to retrieve network statistics: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to retrieve network statistics")


@router.get("/monitoring/processes")
async def get_top_processes(limit: int = 5):
    """
    Returns a list of the top N processes by CPU usage.
    """
    try:
        processes = []
        for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_percent']):
            processes.append(proc.info)

        # 按CPU使用率排序，并返回前N个进程
        processes.sort(key=lambda x: x['cpu_percent'], reverse=True)
        top_processes = processes[:limit]
        logger.info(
            f"Retrieved top {limit} processes by CPU usage: {top_processes}")
        return {"top_processes": top_processes}
    except Exception as e:
        logger.error(f"Failed to retrieve top processes: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to retrieve top processes")


@router.get("/monitoring/temperature")
async def get_temperature():
    """
    Returns the current system temperatures (if available).
    """
    try:
        temperatures = psutil.sensors_temperatures()
        if not temperatures:
            logger.warning("Temperature sensors not available")
            raise HTTPException(
                status_code=404, detail="Temperature sensors not available")
        logger.info(f"Retrieved system temperatures: {temperatures}")
        return temperatures
    except Exception as e:
        logger.error(f"Failed to retrieve system temperatures: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/monitoring/logs")
async def get_recent_logs(lines: int = 10):
    """
    Returns the last N lines from the log file.
    """
    log_file_path = "/var/log/syslog"  # Replace with the actual log file path
    try:
        with open(log_file_path, 'r') as file:
            logs = file.readlines()
        recent_logs = logs[-lines:]
        logger.info(f"Retrieved last {lines} lines from log file")
        return {"recent_logs": recent_logs}
    except FileNotFoundError:
        logger.error(f"Log file not found: {log_file_path}")
        raise HTTPException(status_code=404, detail="Log file not found")
    except Exception as e:
        logger.error(f"Failed to retrieve recent logs: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/monitoring/io")
async def get_io_statistics():
    """
    Returns I/O statistics including read/write bytes.
    """
    try:
        io_stats = psutil.disk_io_counters()
        io_data = {
            "read_bytes": io_stats.read_bytes,
            "write_bytes": io_stats.write_bytes
        }
        logger.info(f"Retrieved I/O statistics: {io_data}")
        return io_data
    except Exception as e:
        logger.error(f"Failed to retrieve I/O statistics: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to retrieve I/O statistics")


@router.get("/monitoring/load")
async def get_load_avg():
    """
    Returns the system load averages over the last 1, 5, and 15 minutes.
    """
    try:
        load_avg = os.getloadavg()
        load_data = {
            "1_min": load_avg[0],
            "5_min": load_avg[1],
            "15_min": load_avg[2]
        }
        logger.info(f"Retrieved system load averages: {load_data}")
        return load_data
    except Exception as e:
        logger.error(f"Failed to retrieve system load averages: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to retrieve system load averages")


@router.get("/monitoring/threads")
async def get_thread_statistics(limit: int = 5):
    """
    Returns a list of the top N processes by thread count.
    """
    try:
        processes = []
        for proc in psutil.process_iter(['pid', 'name', 'num_threads']):
            processes.append(proc.info)

        # 按线程数排序，并返回前N个进程
        processes.sort(key=lambda x: x['num_threads'], reverse=True)
        top_threads = processes[:limit]
        logger.info(
            f"Retrieved top {limit} processes by thread count: {top_threads}")
        return {"top_processes_by_threads": top_threads}
    except Exception as e:
        logger.error(f"Failed to retrieve top processes by thread count: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to retrieve top processes by thread count")
