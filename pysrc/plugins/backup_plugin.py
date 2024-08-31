import os
import shutil
import zipfile
from datetime import datetime, timedelta
from fastapi import APIRouter, HTTPException
from loguru import logger

# 配置loguru日志
logger.add("backup_manager.log", level="DEBUG",
           format="{time} {level} {message}", rotation="10 MB")

router = APIRouter()

BACKUP_DIR = "./backups/"


@router.post("/backup/create/")
async def create_backup(file_path: str, compress: bool = False):
    """
    Creates a backup of a specified file, with an option to compress it.
    """
    logger.debug(
        f"Received request to create backup for file: {file_path}, compress={compress}")

    if not os.path.exists(file_path):
        logger.error(f"File not found: {file_path}")
        raise HTTPException(status_code=404, detail="File not found")

    if not os.path.exists(BACKUP_DIR):
        os.makedirs(BACKUP_DIR)
        logger.info(f"Backup directory created at {BACKUP_DIR}")

    timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
    backup_file_name = f"{os.path.basename(file_path)}_{timestamp}"
    backup_path = os.path.join(BACKUP_DIR, backup_file_name)

    try:
        if compress:
            backup_path += ".zip"
            with zipfile.ZipFile(backup_path, 'w') as zipf:
                zipf.write(file_path, os.path.basename(file_path))
            logger.info(
                f"File {file_path} backed up and compressed to {backup_path}")
        else:
            shutil.copy(file_path, backup_path)
            logger.info(f"File {file_path} backed up to {backup_path}")

        backup_size = os.path.getsize(backup_path)
        logger.debug(
            f"Backup created successfully with size: {backup_size} bytes")

        return {"status": "success", "backup_path": backup_path, "size": backup_size}

    except Exception as e:
        logger.error(f"Failed to create backup: {e}")
        raise HTTPException(status_code=500, detail="Failed to create backup")


@router.post("/backup/restore/")
async def restore_backup(file_name: str, decompress: bool = False, restore_dir: str = "./"):
    """
    Restores a file from its backup, with an option to decompress it if it's a zip file.
    """
    logger.debug(
        f"Received request to restore backup: {file_name}, decompress={decompress}, restore_dir={restore_dir}")

    backup_path = os.path.join(BACKUP_DIR, file_name)
    if not os.path.exists(backup_path):
        logger.error(f"Backup not found: {backup_path}")
        raise HTTPException(status_code=404, detail="Backup not found")

    if not os.path.exists(restore_dir):
        os.makedirs(restore_dir)
        logger.info(f"Restore directory created at {restore_dir}")

    original_path = os.path.join(restore_dir, file_name.split('_')[0])

    try:
        if decompress and backup_path.endswith(".zip"):
            with zipfile.ZipFile(backup_path, 'r') as zipf:
                zipf.extractall(restore_dir)
                restored_files = zipf.namelist()
            logger.info(
                f"Backup {backup_path} decompressed and restored to {restore_dir}")
            return {"status": "success", "restored_files": restored_files}
        else:
            shutil.copy(backup_path, original_path)
            logger.info(f"Backup {backup_path} restored to {original_path}")
            return {"status": "success", "restored_path": original_path}

    except Exception as e:
        logger.error(f"Failed to restore backup: {e}")
        raise HTTPException(status_code=500, detail="Failed to restore backup")


@router.get("/backup/list/")
async def list_backups():
    """
    Lists all backup files.
    """
    logger.debug("Received request to list all backups")

    if not os.path.exists(BACKUP_DIR):
        logger.info("No backups found, backup directory does not exist.")
        return {"backups": []}

    backups = os.listdir(BACKUP_DIR)
    logger.info(f"Found {len(backups)} backups")
    return {"backups": backups}


@router.delete("/backup/delete/")
async def delete_backup(file_name: str):
    """
    Deletes a specific backup file.
    """
    logger.debug(f"Received request to delete backup: {file_name}")

    backup_path = os.path.join(BACKUP_DIR, file_name)
    if not os.path.exists(backup_path):
        logger.error(f"Backup not found: {backup_path}")
        raise HTTPException(status_code=404, detail="Backup not found")

    try:
        os.remove(backup_path)
        logger.info(f"Backup {file_name} deleted successfully")
        return {"status": "success", "message": f"Backup '{file_name}' deleted"}

    except Exception as e:
        logger.error(f"Failed to delete backup: {e}")
        raise HTTPException(status_code=500, detail="Failed to delete backup")


@router.post("/backup/cleanup/")
async def cleanup_backups(max_backups: int = 5, days: int = 30):
    """
    Cleans up old backups, keeping only the latest `max_backups` or backups within `days` days.
    """
    logger.debug(
        f"Received request to clean up backups, max_backups={max_backups}, days={days}")

    if not os.path.exists(BACKUP_DIR):
        logger.info("No backups to clean up, backup directory does not exist.")
        return {"status": "success", "message": "No backups to clean up"}

    backups = os.listdir(BACKUP_DIR)
    backups.sort(key=lambda x: os.path.getctime(os.path.join(BACKUP_DIR, x)))

    cutoff_time = datetime.now() - timedelta(days=days)
    old_backups = [b for b in backups if datetime.fromtimestamp(
        os.path.getctime(os.path.join(BACKUP_DIR, b))) < cutoff_time]

    backups_to_delete = old_backups[:-
                                    max_backups] if len(backups) > max_backups else []

    for backup in backups_to_delete:
        try:
            os.remove(os.path.join(BACKUP_DIR, backup))
            logger.info(f"Deleted old backup: {backup}")
        except Exception as e:
            logger.error(f"Failed to delete old backup {backup}: {e}")

    logger.info(f"Cleanup complete, deleted {len(backups_to_delete)} backups")
    return {"status": "success", "deleted_backups": backups_to_delete}
