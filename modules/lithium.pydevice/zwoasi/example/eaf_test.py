#!/usr/bin/env python3
import argparse
import configparser
import ctypes
from pathlib import Path
from typing import Optional, Union

from loguru import logger
from rich.console import Console
from rich.syntax import Syntax

from ..libs.eaf import *

# Initialize Rich Console
console = Console()

# Configure Loguru Logger
logger.add("eaf.log", format="{time} {level} {message}",
           level="INFO", rotation="10 MB")


class ASIEAFController:
    def __init__(self):
        self.device_ids = []
        self.num_devices = 0

    def get_device_count(self) -> bool:
        try:
            self.num_devices = EAFGetNum()
            if self.num_devices <= 0:
                logger.error("No devices found")
                console.print("[red]No devices found.[/red]")
                return False
            logger.info(f"Found {self.num_devices} device(s)")
            console.print(f"[green]Found {self.num_devices} device(s)[/green]")
            return True
        except Exception as e:
            logger.exception("Exception occurred while getting device count")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def get_device_ids(self) -> bool:
        if self.num_devices <= 0:
            console.print("[red]No devices to retrieve IDs from.[/red]")
            return False
        try:
            self.device_ids = (ctypes.c_int * self.num_devices)()
            result = EAFGetProductIDs(self.device_ids)
            if result != EAF_SUCCESS:
                logger.error("Failed to get device IDs")
                console.print("[red]Failed to get device IDs.[/red]")
                return False
            logger.info("Successfully retrieved device IDs")
            console.print("[green]Successfully retrieved device IDs.[/green]")
            return True
        except Exception as e:
            logger.exception("Exception occurred while getting device IDs")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def open_device(self, device_id: int) -> bool:
        try:
            result = EAFOpen(device_id)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to open device {device_id}")
                console.print(f"[red]Failed to open device {device_id}.[/red]")
                return False
            logger.info(f"Device {device_id} opened")
            console.print(f"[green]Device {device_id} opened.[/green]")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while opening device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def get_device_property(self, device_id: int) -> Optional[EAF_INFO]:
        try:
            device_info = EAF_INFO()
            result = EAFGetProperty(device_id, ctypes.byref(device_info))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get properties of device {device_id}")
                console.print(
                    f"[red]Failed to get properties of device {device_id}.[/red]")
                return None
            logger.info(f"Successfully got properties of device {device_id}")
            console.print(
                f"[green]Successfully got properties of device {device_id}.[/green]")
            return device_info
        except Exception as e:
            logger.exception(
                f"Exception occurred while getting properties of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return None

    def move_device(self, device_id: int, position: int) -> bool:
        try:
            result = EAFMove(device_id, position)
            if result != EAF_SUCCESS:
                logger.error(
                    f"Failed to move device {device_id} to position {position}")
                console.print(
                    f"[red]Failed to move device {device_id} to position {position}.[/red]")
                return False
            logger.info(f"Device {device_id} moved to position {position}")
            console.print(
                f"[green]Device {device_id} moved to position {position}.[/green]")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while moving device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def stop_device(self, device_id: int) -> bool:
        try:
            result = EAFStop(device_id)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to stop device {device_id}")
                console.print(f"[red]Failed to stop device {device_id}.[/red]")
                return False
            logger.info(f"Device {device_id} stopped")
            console.print(f"[green]Device {device_id} stopped.[/green]")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while stopping device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def get_position(self, device_id: int) -> Optional[int]:
        try:
            position = ctypes.c_int()
            result = EAFGetPosition(device_id, ctypes.byref(position))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get position of device {device_id}")
                console.print(
                    f"[red]Failed to get position of device {device_id}.[/red]")
                return None
            logger.info(
                f"Current position of device {device_id}: {position.value}")
            console.print(
                f"[green]Current position of device {device_id}: {position.value}[/green]")
            return position.value
        except Exception as e:
            logger.exception(
                f"Exception occurred while getting position of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return None

    def reset_position(self, device_id: int, position: int) -> bool:
        try:
            result = EAFResetPostion(device_id, position)
            if result != EAF_SUCCESS:
                logger.error(
                    f"Failed to reset position of device {device_id} to {position}")
                console.print(
                    f"[red]Failed to reset position of device {device_id} to {position}.[/red]")
                return False
            logger.info(f"Position of device {device_id} reset to {position}")
            console.print(
                f"[green]Position of device {device_id} reset to {position}.[/green]")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while resetting position of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def get_temperature(self, device_id: int) -> Optional[float]:
        try:
            temperature = ctypes.c_float()
            result = EAFGetTemp(device_id, ctypes.byref(temperature))
            if result != EAF_SUCCESS:
                logger.error(
                    f"Failed to get temperature of device {device_id}")
                console.print(
                    f"[red]Failed to get temperature of device {device_id}.[/red]")
                return None
            logger.info(
                f"Current temperature of device {device_id}: {temperature.value}°C")
            console.print(
                f"[green]Current temperature of device {device_id}: {temperature.value}°C[/green]")
            return temperature.value
        except Exception as e:
            logger.exception(
                f"Exception occurred while getting temperature of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return None

    def set_max_step(self, device_id: int, max_step: int) -> bool:
        try:
            result = EAFSetMaxStep(device_id, max_step)
            if result != EAF_SUCCESS:
                logger.error(
                    f"Failed to set max step of device {device_id} to {max_step}")
                console.print(
                    f"[red]Failed to set max step of device {device_id} to {max_step}.[/red]")
                return False
            logger.info(f"Max step of device {device_id} set to {max_step}")
            console.print(
                f"[green]Max step of device {device_id} set to {max_step}.[/green]")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while setting max step of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def get_max_step(self, device_id: int) -> Optional[int]:
        try:
            max_step = ctypes.c_int()
            result = EAFGetMaxStep(device_id, ctypes.byref(max_step))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get max step of device {device_id}")
                console.print(
                    f"[red]Failed to get max step of device {device_id}.[/red]")
                return None
            logger.info(f"Max step of device {device_id}: {max_step.value}")
            console.print(
                f"[green]Max step of device {device_id}: {max_step.value}[/green]")
            return max_step.value
        except Exception as e:
            logger.exception(
                f"Exception occurred while getting max step of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return None

    def set_backlash(self, device_id: int, backlash: int) -> bool:
        try:
            result = EAFSetBacklash(device_id, backlash)
            if result != EAF_SUCCESS:
                logger.error(
                    f"Failed to set backlash of device {device_id} to {backlash}")
                console.print(
                    f"[red]Failed to set backlash of device {device_id} to {backlash}.[/red]")
                return False
            logger.info(f"Backlash of device {device_id} set to {backlash}")
            console.print(
                f"[green]Backlash of device {device_id} set to {backlash}.[/green]")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while setting backlash of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return False

    def get_backlash(self, device_id: int) -> Optional[int]:
        try:
            backlash = ctypes.c_int()
            result = EAFGetBacklash(device_id, ctypes.byref(backlash))
            if result != EAF_SUCCESS:
                logger.error(f"Failed to get backlash of device {device_id}")
                console.print(
                    f"[red]Failed to get backlash of device {device_id}.[/red]")
                return None
            logger.info(f"Backlash of device {device_id}: {backlash.value}")
            console.print(
                f"[green]Backlash of device {device_id}: {backlash.value}[/green]")
            return backlash.value
        except Exception as e:
            logger.exception(
                f"Exception occurred while getting backlash of device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return None

    def close_device(self, device_id: int) -> bool:
        try:
            result = EAFClose(device_id)
            if result != EAF_SUCCESS:
                logger.error(f"Failed to close device {device_id}")
                console.print(
                    f"[red]Failed to close device {device_id}.[/red]")
                return False
            logger.info(f"Device {device_id} closed")
            console.print(f"[green]Device {device_id} closed.[/green]")
            return True
        except Exception as e:
            logger.exception(
                f"Exception occurred while closing device {device_id}")
            console.print(f"[red]Exception: {e}[/red]")
            return False


def list_devices(controller: ASIEAFController):
    if controller.get_device_count() and controller.get_device_ids():
        for idx, device_id in enumerate(controller.device_ids, start=1):
            console.print(f"Device {idx}: ID {device_id}")
    else:
        console.print("[red]No devices to list.[/red]")


def open_device(controller: ASIEAFController, device_id: int):
    controller.open_device(device_id)


def get_property(controller: ASIEAFController, device_id: int):
    device_info = controller.get_device_property(device_id)
    if device_info:
        pretty_print_ini(str(device_info))


def move_device(controller: ASIEAFController, device_id: int, position: int):
    controller.move_device(device_id, position)


def stop_device(controller: ASIEAFController, device_id: int):
    controller.stop_device(device_id)


def get_position(controller: ASIEAFController, device_id: int):
    controller.get_position(device_id)


def reset_position(controller: ASIEAFController, device_id: int, position: int):
    controller.reset_position(device_id, position)


def get_temperature(controller: ASIEAFController, device_id: int):
    controller.get_temperature(device_id)


def set_max_step(controller: ASIEAFController, device_id: int, max_step: int):
    controller.set_max_step(device_id, max_step)


def get_max_step(controller: ASIEAFController, device_id: int):
    controller.get_max_step(device_id)


def set_backlash(controller: ASIEAFController, device_id: int, backlash: int):
    controller.set_backlash(device_id, backlash)


def get_backlash(controller: ASIEAFController, device_id: int):
    controller.get_backlash(device_id)


def close_device(controller: ASIEAFController, device_id: int):
    controller.close_device(device_id)


def main():
    parser = argparse.ArgumentParser(
        description="ASIEAFController CLI Tool",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""
Examples:
    List all connected devices:
        eaf_cli.py list-devices

    Open a device with ID 1:
        eaf_cli.py open-device 1

    Get properties of device 1:
        eaf_cli.py get-property 1

    Move device 1 to position 1000:
        eaf_cli.py move-device 1 1000

    Stop device 1:
        eaf_cli.py stop-device 1

    Get current position of device 1:
        eaf_cli.py get-position 1

    Reset position of device 1 to 500:
        eaf_cli.py reset-position 1 500

    Get temperature of device 1:
        eaf_cli.py get-temperature 1

    Set max step of device 1 to 200:
        eaf_cli.py set-max-step 1 200

    Get max step of device 1:
        eaf_cli.py get-max-step 1

    Set backlash of device 1 to 50:
        eaf_cli.py set-backlash 1 50

    Get backlash of device 1:
        eaf_cli.py get-backlash 1

    Close device 1:
        eaf_cli.py close-device 1
"""
    )

    subparsers = parser.add_subparsers(title="Commands", dest="command")
    subparsers.required = True

    # list-devices
    subparsers.add_parser("list-devices", help="List all connected devices")

    # open-device
    parser_open = subparsers.add_parser(
        "open-device", help="Open a specific device")
    parser_open.add_argument("device_id", type=int,
                             help="ID of the device to open")

    # get-property
    parser_get_prop = subparsers.add_parser(
        "get-property", help="Get properties of a device")
    parser_get_prop.add_argument(
        "device_id", type=int, help="ID of the device")

    # move-device
    parser_move = subparsers.add_parser(
        "move-device", help="Move a device to a specific position")
    parser_move.add_argument("device_id", type=int, help="ID of the device")
    parser_move.add_argument("position", type=int,
                             help="Position to move the device to")

    # stop-device
    parser_stop = subparsers.add_parser("stop-device", help="Stop a device")
    parser_stop.add_argument("device_id", type=int, help="ID of the device")

    # get-position
    parser_get_pos = subparsers.add_parser(
        "get-position", help="Get current position of a device")
    parser_get_pos.add_argument("device_id", type=int, help="ID of the device")

    # reset-position
    parser_reset_pos = subparsers.add_parser(
        "reset-position", help="Reset position of a device")
    parser_reset_pos.add_argument(
        "device_id", type=int, help="ID of the device")
    parser_reset_pos.add_argument(
        "position", type=int, help="New position to reset to")

    # get-temperature
    parser_get_temp = subparsers.add_parser(
        "get-temperature", help="Get temperature of a device")
    parser_get_temp.add_argument(
        "device_id", type=int, help="ID of the device")

    # set-max-step
    parser_set_max = subparsers.add_parser(
        "set-max-step", help="Set max step of a device")
    parser_set_max.add_argument("device_id", type=int, help="ID of the device")
    parser_set_max.add_argument(
        "max_step", type=int, help="Maximum step value")

    # get-max-step
    parser_get_max = subparsers.add_parser(
        "get-max-step", help="Get max step of a device")
    parser_get_max.add_argument("device_id", type=int, help="ID of the device")

    # set-backlash
    parser_set_backlash = subparsers.add_parser(
        "set-backlash", help="Set backlash of a device")
    parser_set_backlash.add_argument(
        "device_id", type=int, help="ID of the device")
    parser_set_backlash.add_argument(
        "backlash", type=int, help="Backlash value to set")

    # get-backlash
    parser_get_backlash = subparsers.add_parser(
        "get-backlash", help="Get backlash of a device")
    parser_get_backlash.add_argument(
        "device_id", type=int, help="ID of the device")

    # close-device
    parser_close = subparsers.add_parser("close-device", help="Close a device")
    parser_close.add_argument("device_id", type=int, help="ID of the device")

    # Verbosity
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose logging"
    )

    args = parser.parse_args()

    # Set logging level based on verbosity
    if args.verbose:
        logger.remove()
        logger.add(
            "eaf.log", format="{time} {level} {message}", level="DEBUG", rotation="10 MB")
        logger.debug("Verbose mode enabled.")

    controller = ASIEAFController()

    if args.command == "list-devices":
        list_devices(controller)
    elif args.command == "open-device":
        open_device(controller, args.device_id)
    elif args.command == "get-property":
        get_property(controller, args.device_id)
    elif args.command == "move-device":
        move_device(controller, args.device_id, args.position)
    elif args.command == "stop-device":
        stop_device(controller, args.device_id)
    elif args.command == "get-position":
        get_position(controller, args.device_id)
    elif args.command == "reset-position":
        reset_position(controller, args.device_id, args.position)
    elif args.command == "get-temperature":
        get_temperature(controller, args.device_id)
    elif args.command == "set-max-step":
        set_max_step(controller, args.device_id, args.max_step)
    elif args.command == "get-max-step":
        get_max_step(controller, args.device_id)
    elif args.command == "set-backlash":
        set_backlash(controller, args.device_id, args.backlash)
    elif args.command == "get-backlash":
        get_backlash(controller, args.device_id)
    elif args.command == "close-device":
        close_device(controller, args.device_id)
    else:
        parser.print_help()


def pretty_print_ini(data: str) -> None:
    """
    Utility function to pretty print INI data using Rich.
    """
    try:
        config = configparser.ConfigParser()
        config.read_string(data)
        pretty_ini = ""
        for section in config.sections():
            pretty_ini += f"[{section}]\n"
            for key, value in config.items(section):
                pretty_ini += f"{key} = {value}\n"
            pretty_ini += "\n"
        syntax = Syntax(pretty_ini, "ini", theme="monokai", line_numbers=True)
        console.print(syntax)
    except configparser.Error as e:
        console.print(f"[red]INI Parse Error: {e}[/red]")
        logger.error(f"INI Parse Error: {e}")


if __name__ == "__main__":
    main()
