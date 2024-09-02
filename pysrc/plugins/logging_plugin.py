from fastapi import APIRouter, Query, HTTPException
from typing import Optional, List
from loguru import logger
from datetime import datetime, timedelta

router = APIRouter()

# 配置 loguru 日志系统
logger.add("server.log", level="INFO",
           format="{time} {level} {message}", rotation="10 MB")


@router.get("/logs/search")
async def search_logs(
    level: Optional[str] = Query(
        None, description="Log level to filter by (e.g., INFO, ERROR)"),
    keyword: Optional[str] = Query(
        None, description="Keyword to search for in logs"),
    start_date: Optional[str] = Query(
        None, description="Start date (YYYY-MM-DD) for filtering logs"),
    end_date: Optional[str] = Query(
        None, description="End date (YYYY-MM-DD) for filtering logs")
):
    """
    Searches logs for a specific level, keyword, and/or date range.
    """
    logger.debug(
        f"Searching logs with level={level}, keyword={keyword}, start_date={start_date}, end_date={end_date}")

    try:
        with open("server.log", "r") as f:
            lines = f.readlines()

        filtered_logs = []
        for line in lines:
            log_time_str = line.split(" ")[0]
            log_time = datetime.fromisoformat(log_time_str)

            if start_date:
                start_datetime = datetime.fromisoformat(start_date)
                if log_time < start_datetime:
                    continue

            if end_date:
                end_datetime = datetime.fromisoformat(
                    end_date) + timedelta(days=1)
                if log_time >= end_datetime:
                    continue

            if (level is None or level.upper() in line) and (keyword is None or keyword in line):
                filtered_logs.append(line)

        logger.info(f"Found {len(filtered_logs)} matching log entries")
        return {"logs": filtered_logs}

    except Exception as e:
        logger.error(f"Error while searching logs: {e}")
        raise HTTPException(status_code=500, detail="Failed to search logs")


@router.get("/logs/list")
async def list_log_files():
    """
    Lists all log files in the current directory.
    """
    from os import listdir
    from os.path import isfile, join

    logger.debug("Listing all log files")
    log_files = [f for f in listdir('.') if isfile(
        join('.', f)) and f.endswith('.log')]

    logger.info(f"Found {len(log_files)} log files")
    return {"log_files": log_files}


@router.delete("/logs/delete")
async def delete_log_file(file_name: str):
    """
    Deletes a specific log file.
    """
    from os import remove
    from os.path import exists

    logger.debug(f"Request to delete log file: {file_name}")

    if not exists(file_name):
        logger.error(f"Log file not found: {file_name}")
        raise HTTPException(status_code=404, detail="Log file not found")

    try:
        remove(file_name)
        logger.info(f"Log file deleted: {file_name}")
        return {"status": "success", "message": f"Log file '{file_name}' deleted"}

    except Exception as e:
        logger.error(f"Failed to delete log file: {e}")
        raise HTTPException(
            status_code=500, detail="Failed to delete log file")
