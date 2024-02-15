# coding=utf-8

"""

Copyright(c) 2022-2023 Max Qian  <lightapt.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License version 3 as published by the Free Software Foundation.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.
You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

"""

# #################################################################
#
# This file is part of LightAPT server , which contains the functionality
# of the communication between the server and the lightguider , via asynchronous
# socket communication
#
# #################################################################

import asyncio
import json
import os
import platform
import selectors
import socket
import threading

from logger import lightguider_logger as logger

class switch(object):
    """switch function NOTE : must call break after all"""
    def __init__(self, value):
        self.value = value
        self.fall = False
 
    def __iter__(self):
        """Return the match method once, then stop"""
        yield self.match
        raise StopIteration
    
    def match(self, *args):
        """Indicate whether or not to enter a case suite"""
        if self.fall  or not args:
            return True
        elif self.value in args:
            self.fall = True
            return True
        else:
            return False


def check_process_exist(process_name: str) -> bool:
    """
    Check if process exists
    Args:
        process_name : str
    Returns: bool
    """
    #for proc in psutil.process_iter():
    #    if proc.name() == process_name:
    #        return True
    return True


class TcpSocket(object):
    """
    TCP socket client interface
    """

    def __init__(self):
        self.lines = []
        self.buf = b""
        self.sock = None
        self.sel = None
        self.terminate = False

    def __del__(self):
        self.disconnect()

    def connect(self, hostname: str, port: int) -> bool:
        self.sock = socket.socket()
        try:
            self.sock.connect((hostname, port))
            self.sock.setblocking(False)  # non-blocking
            self.sel = selectors.DefaultSelector()
            self.sel.register(self.sock, selectors.EVENT_READ)
        except Exception:
            self.sel = None
            self.sock = None
            raise OSError

    def disconnect(self) -> None:
        if self.sel is not None:
            self.sel.unregister(self.sock)
            self.sel = None
        if self.sock is not None:
            self.sock.close()
            self.sock = None

    def is_connected(self) -> bool:
        return self.sock is not None

    def terminate(self) -> None:
        self.terminate = True

    def read(self):
        while not self.lines:
            while True:
                if self.terminate:
                    return ""
                events = self.sel.select(0.5)
                if events:
                    break
            s = self.sock.recv(4096)
            i0 = 0
            i = i0
            while i < len(s):
                if s[i] == b"\r"[0] or s[i] == b"\n"[0]:
                    self.buf += s[i0:i]
                    if self.buf:
                        self.lines.append(self.buf)
                        self.buf = b""
                    i += 1
                    i0 = i
                else:
                    i += 1
            self.buf += s[i0:i]
        return self.lines.pop(0)

    def send(self, s: str) -> bool:
        b = s.encode()
        totsent = 0
        while totsent < len(b):
            sent = self.sock.send(b[totsent:])
            if sent == 0:
                return False
            totsent += sent
        return True


class lightguiderClientWorker(object):
    """
    lightguider client but alse worker , all of the functions should be asynchronous.
    And do not exist any blocking operations , such as 'while True'
    """

    def __init__(self) -> None:
        """
        Initialize the lightguider client instance and prepare the connection
        Args : None
        Returns : None
        """
        self.client = TcpSocket()
        self.conn = TcpSocket()
        self._background_task = None
        self.response = None
        self.lock = threading.Lock()
        self.cond = threading.Condition()

        self._host = None
        self._lightguiderversion = None
        self._subversion = None
        self._msgversion = None

        self._is_server_connected = False
        self._is_device_connected = False
        self._terminated = False

        self._profiles = None
        self._current_profile = {}

        self._mount = None

        self._is_calibrating = False
        self._is_calibrated = False
        self._is_calibration_flipped = False
        self._calibrated_data = {}
        self._calibrated_status = {}
        self._calibrated_error = ""

        self._is_star_found = False
        self._is_star_locked = False
        self._is_star_selected = False
        self._star_position = [0, 0]

        self._is_guiding = False
        self._guiding_error = ""
        self._guiding_status = {}

        self._is_settling = False
        self._is_selected = False
        self._settle_status = {}
        self._settle_error = ""

        self._starlost_status = {}
        self._starlost_error = ""

        self._is_looping = False

        self._dither_dx = None
        self._dither_dy = None

        self._exposure = None

        self._is_cooling = False
        self._target_temperature = None
        self._current_temperature = None
        self._coolig_power = None

        self._last_error = ""

        self.lightguider_instance = None

    def __del__(self) -> None:
        """
        Destructor of the lightguider client instance
        """

    def __str__(self) -> str:
        """
        Returns a string representation of the client instance
        """
        return "LightAPT lightguider Client and Network Worker"

    # #################################################################
    #
    # Http Request Handler Functions
    #
    # #################################################################

    # #################################################################
    # Start or stop the lightguider server
    # #################################################################

    async def start_lightguider(self, path=None) -> dict:
        """
        Start the lightguider server with the specified path
        Args :
            path : str # full path to the lightguider server executable
        Returns : {
            "message": str # None if the operation was successful
        }
        """
        res = {"message": None}
        # Check if the instance had already been created
        if self.lightguider_instance is not None:
            logger.warning("lightguider instance had already been created")
            res["message"] = "lightguider instance had already been created"
            return res
        lightguider_path = None
        # judge the system type
        if platform.platform() == "Windows":
            if path is None:
                lightguider_path = "C:\Program Files (x86)\LGuiderGuiding2\lightguider.exe"
            else:
                lightguider_path = path
        elif platform.platform() == "Linux":
            if path is None:
                lightguider_path = "/usr/bin/lightguider"
            else:
                lightguider_path = path
        logger.debug("lightguider executable path : {}".format(lightguider_path))

        # Check whether the executable is existing
        if not os.path.exists(lightguider_path):
            logger.error("lightguider executable path does not exist: {}".format(lightguider_path))
        self.lightguider_instance = asyncio.subprocess.create_subprocess_exec(
            program=lightguider_path,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )

        logger.info("lightguider server started successfully")
        res["message"] = "lightguider server started successfully"
        return res

    async def stop_lightguider(self) -> dict:
        """
        Stop the lightguider server
        Args : None
        Returns : {
            "message" : str
        }
        """
        res = {"message": None}
        if self.lightguider_instance is None:
            logger.error("No lightguider instance running on this machine")
            res["message"] = "No lightguider instance running on this machine"
            return res
        try:
            self.lightguider_instance.close()
        except Exception as e:
            logger.error("Failed to close lightguider instance : {}".format(e))
            res["message"] = "Failed to close lightguider instance"
            return res
        logger.info("lightguider server stopped successfully")
        res["message"] = "lightguider server stopped successfully"
        return res

    async def check_started(self) -> dict:
        """
        Check if the lightguider server had already started
        Args : None
        Returns : {
            "status" : bool
        }
        """
        return {"status": check_process_exist("lightguider")}

    async def scan_server(self, start_port=4400, end_port=4405) -> dict:
        """
        Scan the lightguider server available in the specified port
        Args:
            start_port : int
            end_port : int
        Returns:{
            "list" : [] # a list of the ports which servers are listening on
        }
        """
        res = {"list": []}
        if (
            start_port > end_port
            or not isinstance(start_port, int)
            or not isinstance(end_port, int)
        ):
            logger.error("Invalid port was specified")
            return res
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        for port in range(start_port, end_port + 1):
            try:
                sock.bind(("localhost", port))
                sock.shutdown(2)
                res["list"].append(port)
            except socket.error:
                pass
        logger.debug("Found {} servers".format(len(res["list"])))
        return res

    async def connect_server(self, host="localhost", port=4400) -> dict:
        """
        Connect to the lightguider server on the specified port
        Args :
            host : str # hostname of the lightguider server , default is "localhost"
            port : int # port number of the server , default is 4400
        Returns : {
            "message": str # None if the operation is successful
        }
        """
        resp = {"message": None}

        if self.conn.is_connected():
            resp["message"] = "Server had already connected"
            return resp
        if not isinstance(host, str) or not isinstance(port, int):
            resp["message"] = "Invalid host or port were specified"
            return resp
        try:
            self._terminated = False
            self.conn.connect(host, port)
            self._is_server_connected = True
            # Start a standalone thread to listen to the lightguider server
            self._background_task = threading.Thread(target=self.background_task)
            self._background_task.daemon = True
            self._background_task.start()
        except OSError:
            resp["message"] = "Failed to connect to the lightguider server"
            return resp
        resp["message"] = "Connected to lightguider server successfully"
        return resp

    async def disconnect_server(self) -> dict:
        """
        Disconnects from the lightguider server
        Args : None
        Returns : {
            "message": str # None if the operation is successful
        }
        """
        res = {"message": None}
        if not self._is_server_connected:
            res[
                "message"
            ] = "Server is not connected , please do not execute this operation"
            return res
        try:
            self.conn.disconnect()
            self._terminated = True
        except Exception:
            res["message"] = "Failed to disconnect from the lightguider server"
            return res
        self._is_server_connected = False
        res["message"] = "Disconnected from the lightguider server successfully"
        return res

    async def reconnect_server(self) -> dict:
        """
        Reconnects to the lightguider server
        Args : None
        Returns : {
            "message": str # None if the operation is successful
        }
        """
        res = {"message": None}
        if not self._is_server_connected and not self.conn.is_connected():
            res[
                "message"
            ] = "Server is not connected , please connect before reconnecting"
            return res
        # Terminate the background thread first
        self._terminated = True
        # Close the socket connection
        self.conn.disconnect()
        # Then reconnect to the server
        self.conn.connect()
        # Restart the background thread
        self._terminated = False
        self._background_task = threading.Thread(target=self.background_task)
        self._background_task.start()

    # #################################################################
    # lightguider Listener
    # #################################################################

    def background_task(self) -> None:
        """
        Background task listen server message | 获取lightguider信息
        Args : None
        Returns : None
        """
        while not self._terminated:
            line = self.conn.read()
            if not line and not self.terminate:
                break
            try:
                j = json.loads(line)
            except json.JSONDecodeError:
                continue
            if "jsonrpc" in j:
                with self.cond:
                    self.response = j
                    self.cond.notify()
            else:
                asyncio.run(self.parser_json(j))

    async def generate_command(self, command: str, params: dict) -> dict:
        """
        Generate command to send to the lightguider server
        Args:
            command : str
            params : dict
        Returns : dict
        """
        res = {"method": command, "id": 1}
        if params is not None:
            if isinstance(params, (list, dict)):
                res["params"] = params
            else:
                res["params"] = [params]
        return res

    async def send_command(self, command: dict) -> dict:
        """
        Send command to the lightguider server
        Args:
            command : dict
        Returns : bool
        """
        r = json.dumps(command, separators=(",", ":"))
        self.conn.send(r + "\r\n")
        # wait for response
        with self.cond:
            while not self.response:
                self.cond.wait(timeout=30)
            response = self.response
            self.response = None
        if "error" in response:
            logger.error(
                "Guiding Error : {})".format(response.get("error").get("message"))
            )
        return response

    async def parser_json(self, message) -> None:
        """
        Parser the JSON message received from the server
        Args : message : JSON message
        Returns : None
        """
        if message is None:
            return
        event = message.get("Event")

        for case in switch(event):
            if case("Version"):
                await self.__version(message)
                break
            if case("LockPositionSet"):
                await self.__lock_position_set(message)
                break
            if case("Calibrating"):
                await self.__calibrating(message)
                break
            if case("CalibrationComplete"):
                await self.__calibration_completed(message)
                break
            if case("StarSelected"):
                await self.__star_selected(message)
                break
            if case("StartGuiding"):
                await self.__start_guiding()
                break
            if case("Paused"):
                await self.__paused()
                break
            if case("StartCalibration"):
                await self.__start_calibration(message)
                break
            if case("AppState"):
                await self.__app_state(message)
                break
            if case("CalibrationFailed"):
                await self.__calibration_failed(message)
                break
            if case("CalibrationDataFlipped"):
                await self.__calibration_data_flipped(message)
                break
            if case("LockPositionShiftLimitReached"):
                await self.__lock_position_shift_limit_reached()
                break
            if case("LoopingExposures"):
                await self.__looping_exposures(message)
                break
            if case("LoopingExposuresStopped"):
                await self.__looping_exposures_stopped()
                break
            if case("SettleBegin"):
                await self.__settle_begin()
                break
            if case("Settling"):
                await self.__settling(message)
                break
            if case("SettleDone"):
                await self.__settle_done(message)
                break
            if case("StarLost"):
                await self.__star_lost(message)
                break
            if case("GuidingStopped"):
                await self.__guiding_stopped()
                break
            if case("Resumed"):
                await self.__resumed()
                break
            if case("GuideStep"):
                await self.__guide_step(message)
                break
            if case("GuidingDithered"):
                await self.__guiding_dithered(message)
                break
            if case("LockPositionLost"):
                await self.__lock_position_lost()
                break
            if case("Alert"):
                await self.__alert(message)
                break
            if case("GuideParamChange"):
                await self.__guide_param_change(message)
                break
            if case("ConfigurationChange"):
                await self.__configuration_change()
                break
            logger.error(f"Unknown event : {event}")
            break

    # #################################################################
    # Profiles Manager
    # #################################################################

    async def get_profiles(self) -> dict:
        """
        Get all profiles available on the lightguider server
        Args : None
        Returns : {
            "list" : list # a list of profiles
            "message" : str # None if succeeded
        }
        NOTE : If no profiles are available , just return an empty list
        """
        resp = {"list": None, "message": None}
        if not self._is_server_connected:
            resp["message"] = "Server is not connected"
            return resp
        command = await self.generate_command("get_profiles", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get profiles"
            resp["error"] = res.get("error")
            return resp
        self._profiles = res.get("result")
        logger.debug("All of the profile : {}".format(self._profiles))
        resp["list"] = self._profiles
        return resp

    async def get_current_profile(self) -> dict:
        """
        Get the current profile of the lightguider server
        Args : None
        Returns : {
            "profile" : dict
        }
        """
        resp = {"message": None, "profile": None}
        command = await self.generate_command("get_profile", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["error"] = res.get("error")
            resp["message"] = "Failed to get current profile"
            return resp
        self._current_profile["id"] = res.get("result").get("id")
        logger.debug("Current profile id : {}".format(self._current_profile["id"]))
        # Check if the profile list is empty
        if not self._profiles:
            await self.get_profiles()
        for itme in self._profiles:
            if itme["id"] == self._current_profile["id"]:
                self._current_profile["name"] = itme["name"]
        # Get the devices settings in the profile
        command = await self.generate_command("get_current_equipment", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["error"] = res.get("error")
            resp["message"] = "Failed to get devices settings in the current profile"
            return resp
        try:
            self._current_profile["camera"] = (
                res.get("result").get("camera").get("name")
            )
            self._current_profile["mount"] = res.get("result").get("mount").get("name")
        except KeyError:
            pass
        resp["profile"] = self._current_profile
        return resp

    async def set_profile(self, profile_id: int) -> dict:
        """
        Set the profile of the lightguider server
        Args :
            profile_id : int
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {
            "message": None,
            "error": None,
        }
        if not isinstance(profile_id, int):
            resp["message"] = "Invalid profile_id was specified"
            return resp
        command = await self.generate_command("set_profile", [profile_id])
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to set the profile"
            resp["error"] = res.get("error")
            return resp
        self._current_profile["id"] = profile_id
        return resp

    async def generate_profile(self, profile={}) -> dict:
        """
        Generates a profile for the given parameters
        Args :
            profile : {
                "name" : str # Name of the profile
                "id" : int # Id of the profile
                "camera" : str # Camera type
                "mount" : str # Mount type
            }
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {
            "message": None,
            "error": None,
        }

        _name = profile.get("name")
        _id = profile.get("id")
        _camera = profile.get("camera")
        _mount = profile.get("mount")

        if not _name or not _id or not _camera or not _mount:
            resp[
                "message"
            ] = "Please provide all of the required parameters for a profile"
            return resp
        if (
            not isinstance(_name, str)
            or not isinstance(_id, int)
            or not isinstance(_camera, str)
            or not isinstance(_mount, str)
        ):
            resp["message"] = "Invalid profile parameters were specified"
            return resp
        """"
            LGuider Profile 1
            /auto_exp/exposure_max	1	5000
            /auto_exp/exposure_min	1	1000
            /auto_exp/target_snr	1	6
            /CalStepCalc/GuideSpeed	1	0.5
            /camera/AutoLoadDarks	1	1
            /camera/AutoLoadDefectMap	1	1
            /camera/binning	1	1
            /camera/gain	1	95
            /camera/LastMenuchoice	1	Simulator
            /camera/pixelsize	1	9.8
            /camera/SaturationADU	1	0
            /camera/SaturationByADU	1	1
            /camera/TimeoutMs	1	15000
            /camera/UseSubframes	1	0
            /frame/var_delay/enabled	1	0
            /frame/var_delay/long_delay	1	10000
            /frame/var_delay/short_delay	1	1000
            /frame/focalLength	1	100
            /frame/timeLapse	1	0
            /guider/multistar/enabled	1	1
            /guider/onestar/MassChangeThreshold	1	0.5
            /guider/onestar/MassChangeThresholdEnabled	1	0
            /guider/onestar/SearchRegion	1	15
            /guider/onestar/TolerateJumpsEnabled	1	0
            /guider/onestar/TolerateJumpsThreshold	1	4
            /guider/AutoSelDownsample	1	0
            /guider/FastRecenter	1	1
            /guider/ScaleImage	1	1
            /guider/StarMinHFD	1	1.5
            /guider/StarMinSNR	1	6
            /ImageLogger/ErrorThreshPx	1	4
            /ImageLogger/ErrorThreshRel	1	4
            /ImageLogger/LogAutoSelectFrames	1	0
            /ImageLogger/LogFramesDropped	1	0
            /ImageLogger/LogFramesOverThreshPx	1	0
            /ImageLogger/LogFramesOverThreshRel	1	0
            /ImageLogger/LoggingEnabled	1	0
            /indi/INDIcam	1	CCD Simulator
            /indi/INDIcam_ccd	1	1
            /indi/INDIcam_forceexposure	1	0
            /indi/INDIcam_forcevideo	1	0
            /indi/INDIhost	1	localhost
            /indi/INDIport	1	7624
            /indi/VerboseLogging	1	1
            /overlay/slit/angle	1	0
            /overlay/slit/center.x	1	376
            /overlay/slit/center.y	1	290
            /overlay/slit/height	1	100
            /overlay/slit/width	1	8
            /rotator/LastMenuChoice	1	
            /scope/GuideAlgorithm/X/Hysteresis/aggression	1	0.7
            /scope/GuideAlgorithm/X/Hysteresis/hysteresis	1	0.1
            /scope/GuideAlgorithm/X/Hysteresis/minMove	1	0.16
            /scope/GuideAlgorithm/Y/ResistSwitch/aggression	1	1
            /scope/GuideAlgorithm/Y/ResistSwitch/fastSwitch	1	1
            /scope/GuideAlgorithm/Y/ResistSwitch/minMove	1	0.16
            /scope/AssumeOrthogonal	1	0
            /scope/BacklashCompEnabled	1	0
            /scope/CalFlipRequiresDecFlip	1	0
            /scope/CalibrationDistance	1	49
            /scope/CalibrationDuration	1	10000
            /scope/DecBacklashCeiling	1	20
            /scope/DecBacklashFloor	1	20
            /scope/DecBacklashPulse	1	20
            /scope/DecGuideMode	1	1
            /scope/HiResEncoders	1	0
            /scope/LastAuxMenuChoice	1	
            /scope/LastMenuChoice	1	On-camera
            /scope/MaxDecDuration	1	2500
            /scope/MaxRaDuration	1	2500
            /scope/StopGuidingWhenSlewing	1	0
            /scope/UseDecComp	1	1
            /scope/XGuideAlgorithm	1	1
            /scope/YGuideAlgorithm	1	4
            /stepguider/LastMenuChoice	1	
            /AutoLoadCalibration	1	0
            /DitherMode	1	0
            /DitherRaOnly	1	0
            /DitherScaleFactor	1	1
            /ExposureDurationMs	1	1000
            /name	1	aaa
            /NoiseReductionMethod	1	0
        """

    async def export_profile(self) -> dict:
        """
        Export the profile
        Args : None
        Returns : {
            "message" : str
        }
        """
        resp = {"message": None}

        if not self._is_server_connected:
            resp["message"] = "Server is not connected"
            return resp
        command = await self.generate_command("export_config_settings", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to export the profile"
            resp["error"] = res.get("error")
            return resp
        return resp

    async def delete_profile(self, name: str) -> dict:
        """
        Delete the profile by name
        Args :
            name : str # the name of the profile
        Returns : {
            "message" : str # None if succeeded
        }
        """

    async def delete_all_profiles(self) -> dict:
        """
        Delete all of the profiles on lightguider server
        Args : None
        Returns : {
            "message" : str
        }
        """

    async def copy_profile(self, name: str, dest: str) -> dict:
        """
        Copy a profile just like a backport
        Args :
            name : str # the name of the profile to copy
            dest : str # the name of the copied file
        Returns : {
            "message" : str
        }
        """

    async def rename_profile(self, name: str, newname: str) -> dict:
        """
        Rename a profile
        Args :
            name : str # the old name of the profile
            newname : str # the new name of the profile
        Returns : {
            "message" : str
        }
        """

    # #################################################################
    # Device Connection
    # #################################################################

    async def connect_device(self) -> dict:
        """
        Connect to the devices in the specified profile
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if self._current_profile is None:
            resp["message"] = "no profile available"
            return resp
        if not self._is_server_connected:
            resp["message"] = "Server is not connected"
            return resp
        command = await self.generate_command("set_connected", True)
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to connect to all of the devices"
            resp["error"] = res.get("error")
            return resp
        self._is_device_connected = True
        return resp

    async def disconnect_device(self) -> dict:
        """
        Disconnect the connected devices
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("set_connected", False)
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to connect to all of the devices"
            resp["error"] = res.get("error")
            return resp
        self._is_device_connected = False
        return resp

    async def reconnect_device(self) -> dict:
        """
        Reconnect the connected devices , just an encapsulation of the connect and disconnect
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        resp = await self.disconnect_device()
        await asyncio.sleep(1)
        resp = await self.connect_device()
        return resp

    async def check_connected(self) -> dict:
        """
        Check if all of the devices connected
        Args : None
        Returns : {
            "message" : str # None if succeeded
            "status" : bool # True if connected
        }
        """
        resp = {"message": None}

        command = await self.generate_command("get_connected", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to check the status of the connection"
            resp["error"] = res.get("error")
            return resp
        self._is_device_connected = res.get("result")
        resp["status"] = res.get("result")

        return resp

    # #################################################################
    #
    # Websocket Handler Functions
    #
    # #################################################################

    # #################################################################
    # Calibration
    # #################################################################

    async def start_calibration(self) -> dict:
        """"""

    async def stop_calibration(self) -> dict:
        """"""

    async def check_calibration(self) -> dict:
        """
        Check if the calibration had been completed successfully
        Args : None
        Returns : {
            "status" : bool # True if the calibrated
        }
        """
        resp = {"message": None, "status": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("get_calibrated", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get calibration data"
            resp["error"] = res.get("error")
            return resp
        self._is_calibrated = res.get("result")
        resp["status"] = self._is_calibrated
        return resp

    async def get_calibration_data(self, device="Mount") -> dict:
        """
        Get the calibration data and return it as a dictionary
        Args :
            device : str # device type , default is "Mount"
        Returns : {
            "data" : {
                'xAngle': self.xAngle,
                'xRate': self.xRate,
                'xParity': self.xParity,
                'yAngle': self.yAngle,
                'yRate': self.yRate,
                'yParity': self.yParity,
            }
        }
        """
        resp = {"message": None, "data": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("get_calibration_data", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get calibration data"
            resp["error"] = res.get("error")
            return resp
        self._calibrated_data = res.get("result")
        resp["data"] = self._calibrated_data
        return resp

    async def clear_calibration_data(self) -> dict:
        """
        Clear calibration data and need a calibration before restart guiding
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("clear_calibration", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to clear calibration data"
            resp["error"] = res.get("error")
            return resp
        self._calibrated_data = {}
        self._is_calibrated = False
        return resp

    async def flip_calibration_data(self) -> dict:
        """
        Flip the calibration data usually for transmit flipper
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("clear_calibration", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to flip calibration data"
            resp["error"] = res.get("error")
            return resp
        self._is_calibration_flipped = True
        return resp

    # #################################################################
    # Guiding
    # #################################################################

    async def find_star(self, roi=[]) -> dict:
        """
        Automatically find a star on the screen
        Args :
            roi : [] # camera frame list
        Returns : {
            "message" : str # None if succeeded
            "x" : int # x position of the found star
            "y" : int # y position of the found star
        }
        """
        resp = {"message": None, "x": None, "y": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("find_star", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to find a star for guiding"
            resp["error"] = res.get("error")
            return resp
        self._is_star_found = True
        self._star_position = res.get("result")
        resp["x"] = self._star_position[0]
        resp["y"] = self._star_position[1]
        return resp

    async def start_guiding(
        self, pixel: float, time: int, timeout: int, recalibration: bool
    ) -> dict:
        """
        Start guiding with the given parameters
        Args :
            pixel : float # maximum guide distance for guiding to be considered stable or "in-range"
            time : int # minimum time to be in-range before considering guiding to be stable
            timeout : int # time limit before settling is considered to have failed
            recalibration : bool # whether to restart the calibration before guiding
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        # Check if the parameters are valid
        if (
            not isinstance(pixel, float)
            or not isinstance(time, int)
            or not isinstance(timeout, int)
            or not isinstance(recalibration, bool)
        ):
            resp["message"] = "Invalid guiding parameters was specified"
            return resp
        settle = {"pixels": pixel, "time": time, "timeout": timeout}

        command = await self.generate_command(
            "guide", {"settle": settle, "recalibrate": recalibration}
        )
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to start guiding"
            resp["error"] = res.get("error")
            return resp
        return resp

    async def stop_guiding(self) -> dict:
        """
        Stop guiding operation
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("set_paused", [True])
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to start guiding"
            resp["error"] = res.get("error")
            return resp
        return resp

    async def pause_guiding(self) -> dict:
        """
        Pause guiding operation but not stop looping
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """

    async def resume_guiding(self) -> dict:
        """
        Resume the guiding operation just be symmetrical with the above function
        Args : None
        Returns : {
            "message" : str # None if succeeded
        }
        """

    async def get_guiding_algorithm(self, axis: str, name=None) -> dict:
        """
        Get the algorithm of the specified axis , if the name is not None
        Args :
            axis : str # name of the axis , "ra","x","dec", or "y"
            name : str # if not None , return a accurate value
        Returns : {
            "list" : list # a list of guide algorithm param names (strings)
            "value" : float # the value of the named parameter
        }
        """

    async def set_guiding_algorithm(self, axis: str, name: str, value: float) -> dict:
        """
        Set the algorithm of the specified axis
        URL : chat.forchange.cn
        Args :
            axis : str # "ra","x","dec", or "y"
            name : str # the name of the parameter
            value : float
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {
            "message": None,
        }

        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        if not isinstance(axis, str) or not axis in ["ra", "x", "dec", "y"]:
            resp["message"] = "Invalid axis specified"
            return resp
        if not isinstance(name, str) or not name in ["ra", "x", "dec", "y"]:
            resp["message"] = "Invalid axis specified"
            return resp
        command = await self.generate_command("set_dec_guide_mode", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to set the guiding mode of the DEC axis"
            resp["error"] = res.get("error")
            return resp
        return resp

    async def get_dec_guiding_mode(self) -> dict:
        """
        Get the guiding mode of the DEC axis
        Args : None
        Returns : {
            "mode" : str # "Off"/"Auto"/"North"/"South"
        }
        """
        resp = {"message": None, "mode": None}

        if not self._is_device_connected:
            resp["message"] = "get_dec_guide_mode"
            return resp
        command = await self.generate_command("get_dec_guide_mode", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get the guiding mode of the DEC axis"
            resp["error"] = res.get("error")
            return resp
        self._dec_guiding_mode = res.get("result")
        resp["mode"] = self._dec_guiding_mode
        return resp

    async def set_dec_guiding_mode(self, mode: str) -> dict:
        """
        Set the guiding mode of the DEC axis
        Args :
            mode : str # "Off"/"Auto"/"North"/"South"
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {
            "message": None,
        }

        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        if not isinstance(mode, str) or not mode in ["Off", "Auto", "North", "South"]:
            resp["message"] = "Invalid mode specified"
            return resp
        command = await self.generate_command("set_dec_guide_mode", {"mode": mode})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to set the guiding mode of the DEC axis"
            resp["error"] = res.get("error")
            return resp
        return resp

    # #################################################################
    # Dither
    # #################################################################

    async def dither(
        self, pixel: float, time: int, timeout: int, raonly: bool, amount: int
    ) -> dict:
        """
        Dither
        Args :
            pixel : float
            time : int
            timeout : int
            raonly : bool
            amount : int
        Returns : {
            "message" : str # None if succeeded
        }
        NOTE : After the jitter starts, it takes a certain time to reach stability, so the detection result needs to be returned to the client
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        # Check if the parameters are valid
        if (
            not isinstance(pixel, float)
            or not isinstance(time, int)
            or not isinstance(timeout, int)
            or not isinstance(raonly, bool)
            or not isinstance(amount, int)
        ):
            resp["message"] = "Invalid dithering parameters was specified"
            return resp
        settle = {"pixels": pixel, "time": time, "timeout": timeout}

        command = await self.generate_command(
            "dither", {"settle": settle, "raOnly": raonly, "amount": amount}
        )
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to start dithering"
            resp["error"] = res.get("error")
            return resp
        return resp

    # #################################################################
    # Camera
    # #################################################################

    async def get_camera_frame_size(self) -> dict:
        """
        Get the frame size of the camera
        Args : None
        Returns : {
            "height" : int # height of the camera frame
            "width" : int # width of the camera frame
        }
        """
        resp = {"message": None, "height": None, "width": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("get_camera_frame_size", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get frame size of the camemra"
            resp["error"] = res.get("error")
            return resp
        self._frame_width, self._frame_height = res.get("result")
        resp["height"] = self._frame_height
        resp["width"] = self._frame_width
        return resp

    async def get_cooler_status(self) -> dict:
        """
        Get the cooler status of the camera
        Args : None
        Returns : {
            "temperature": sensor temperature in degrees C (number),
            "coolerOn": boolean,
            "setpoint": cooler set-point temperature (number, degrees C),
            "power": cooler power (number, percent)
        }
        NOTE : This function needs camera supported
        """
        resp = {
            "message": None,
            "temperature": None,
            "cooler_on": None,
            "setpoint": None,
            "power": None,
        }
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("get_cooler_status", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get cooling status of the camemra"
            resp["error"] = res.get("error")
            return resp
        self._current_temperature = res.get("result").get("temperature")
        self._is_cooling = res.get("result").get("coolerOn")
        self._target_temperature = res.get("result").get("setpoint")
        self._coolig_power = res.get("result").get("power")
        resp["temperature"] = self._current_temperature
        resp["cooler_on"] = self._is_cooling
        resp["setpoint"] = self._target_temperature
        resp["power"] = self._coolig_power
        return resp

    async def get_camera_temperature(self) -> dict:
        """
        Get the current temperature of the camera
        Args : None
        Returns : {
            "temperature" : float # sensor temperature in degrees C
        }
        """
        resp = {"message": None, "temperature": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("get_ccd_temperature", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get temperature of the camemra"
            resp["error"] = res.get("error")
            return resp
        temperature = res.get("result").get("temperature")
        self._current_temperature = temperature
        resp["temperature"] = temperature
        return resp

    async def get_exposure(self) -> dict:
        """
        Get the exposure of the camera
        Args : None
        Returns : {
            "exposure" : float # in seconds
        }
        """
        resp = {"message": None, "exposure": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("get_exposure", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get exposure of the camemra"
            resp["error"] = res.get("error")
            return resp
        exposure = res.get("result") / 1000
        self._exposure = exposure
        resp["exposure"] = exposure
        return resp

    async def set_exposure(self, exposure: float) -> dict:
        """
        Set the exposure value of the camera
        Args :
            exposure : float # in seconds
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        if not isinstance(exposure, float) or not 0 < exposure < 30:
            logger.error("Invalid exposure value specified")
            resp["message"] = "Invalid exposure value specified"
            return resp
        command = await self.generate_command(
            "set_exposure", {"exposure": int(exposure * 1000)}
        )
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to set exposure of the camemra"
            resp["error"] = res.get("error")
            return resp
        self._exposure = exposure
        return resp

    # #################################################################
    # Image
    # #################################################################

    async def get_image(self) -> dict:
        """
        Get the current image of the guiding
        Args : None
        Returns : {
            "frame" : int # frame number
            "width" : int # width in pixels
            "height" : int # height in pixels
            "image" : str # a base64 encoded image
            "star_position" : list # The position of the star is locked , [x,y]
        }
        """
        resp = {
            "message": None,
            "image": None,
            "width": None,
            "height": None,
            "star_position": None,
        }

        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command("get_star_image", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get the current image"
            resp["error"] = res.get("error")
            return resp
        resp["height"] = res.get("height")
        resp["width"] = res.get("width")
        resp["star_position"] = res.get("star_position")
        return resp

    async def save_image(self) -> dict:
        """
        Save the image to the specified folder
        Args : None
        Returns : {
            "path" : str # the full path to the image
        }
        """

    # #################################################################
    # Modified
    # #################################################################

    async def check_modified(self) -> dict:
        """
        Check if the lightguider server been modified by LightAPT
        Args : None
        Returns : {
            "status" : bool # true if the server was modified
        }
        """
        resp = {"message": None, "status": None}
        if not self._is_server_connected:
            resp["message"] = "Server is not connected"
            return resp
        command = await self.generate_command("lightapt_modify_response", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to check if the server is modified"
            resp["error"] = res.get("error")
            return resp
        self._is_server_modified = res.get("result").get("modified")
        resp["status"] = self._is_server_modified
        return resp

    async def get_darklib_path(self) -> dict:
        """
        Get the full path to the darklib directory
        Args : None
        Returns : {
            "path" : str # the full path to the darklib directory
        }
        """
        resp = {"message": None, "path": None}
        if not self._is_server_connected:
            resp["message"] = "Server is not connected"
            return resp
        command = await self.generate_command("get_darklib_path", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get the dark library path"
            resp["error"] = res.get("error")
            return resp
        self._darklib_path = res.get("result").get("path")
        resp["path"] = self._darklib_path
        return resp

    async def get_darklib_name(self, profile_id: int) -> dict:
        """
        Get the name of the darklib profile for the given profile_id
        Args :
            profile_id : int
        Returns : {
            "name" : str # the name of the darklib
        }
        """

    async def check_darklib_loaded(self) -> dict:
        """
        Check if the dark library is loaded
        Args : None
        Returns : {
            "status" : bool # True if the dark library is loaded
        }
        """
        resp = {"message": None, "status": None}

        if not self._is_server_connected:
            resp["message"] = "Server is not connected"
            return resp
        command = await self.generate_command("is_darklib_loaded", {})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get the dark library path"
            resp["error"] = res.get("error")
            return resp
        self._is_darklib_loaded = res.get("result").get("loaded")
        resp["status"] = self._is_darklib_loaded
        return resp

    async def create_darklib(
        self,
        name: str,
        max_exposure: float,
        min_exposure: float,
        count: int,
        rebuild: bool,
        continued: bool,
    ) -> dict:
        """
        Create the dark frame library
        Args :
            name : str # the name of the darklib to save
            max_exposure : float # the max value of the exposure time
            min_exposure : float # the min value of the exposure time
            count : int # the number of the images
            rebuild : bool # whether to rebuild the dark lib
            continued : bool # whether to continue to improve the old lib
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None}
        if not self._is_device_connected:
            resp["message"] = "Device is not connected"
            return resp
        command = await self.generate_command(
            "create_darklib",
            {
                "name": name,
                "max_exposure": max_exposure * 1000,
                "min_exposure": min_exposure * 1000,
                "count": count,
                "rebuild": int(rebuild),
                "continue": int(continued),
            },
        )
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        
        print(res)
        
        if "error" in res:
            resp["message"] = "Failed to get the dark library path"
            resp["error"] = res.get("error")
            return resp
        self._is_darklib_loaded = res.get("result").get("loaded")
        resp["status"] = self._is_darklib_loaded
        return resp

    async def create_profile(self, name: str) -> dict:
        """
        Create a new profile
        Args :
            path : str # the full path to the profile
        Returns : {
            "message" : str # None if succeeded
        }
        """
        resp = {"message": None, "status": None}

        if not self._is_server_connected:
            resp["message"] = "Server is not connected"
            return resp
        command = await self.generate_command("create_profile", {"name": name})
        try:
            res = await self.send_command(command)
        except socket.error as e:
            resp["message"] = "Send command failed"
            resp["error"] = e
            return resp
        if "error" in res:
            resp["message"] = "Failed to get the dark library path"
            resp["error"] = res.get("error")
            return resp
        resp["status"] = res.get("result").get("status")

        return resp

    # #################################################################
    # Server Listener Functions
    # #################################################################

    async def __version(self, message: dict) -> None:
        """
        Get lightguider version
        Args : None
        Returns : None
        """
        self._host = message.get("Host")
        self._lightguiderversion = message.get("LGuiderVersion")
        self._subversion = message.get("LGuiderSubver")
        self._msgversion = message.get("MsgVersion")

    async def __lock_position_set(self, message: dict) -> None:
        """
        Get lock position set
        Args:
            message : dict
        Returns : None
        """
        self._star_position[0] = message.get("X")
        self._star_position[1] = message.get("Y")
        self._is_star_locked = True

    async def __calibrating(self, message: dict) -> None:
        """
        Get calibrating state
        Args:
            message : dict
        Returns : None
        """
        self._calibrated_status["direction"] = message.get("dir")
        self._calibrated_status["distance"] = message.get("dist")
        self._calibrated_status["dx"] = message.get("dx")
        self._calibrated_status["dy"] = message.get("dy")
        self._calibrated_status["position"] = message.get("pos")
        self._calibrated_status["stop"] = message.get("step")
        self._calibrated_status["state"] = message.get("State")

    async def __calibration_completed(self, message: dict) -> None:
        """
        Get calibration completed state
        Args:
            message : dict
        Returns : None
        """
        self._mount = message.get("Mount")

    async def __star_selected(self, message: dict) -> None:
        """
        Get star selected state
        Args:
            message : dict
        Returns : None
        """
        self._star_position[0] = message.get("X")
        self._star_position[1] = message.get("Y")
        self._is_star_selected = True

    async def __start_guiding(self) -> None:
        """
        Get start guiding state
        Args:
            message : dict
        Returns : None
        """
        self._is_guiding = True

    async def __paused(self) -> None:
        """
        Get paused state
        Args : None
        Returns : None
        """
        self._is_guiding = False
        self._is_calibrating = False

    async def __start_calibration(self, message: dict) -> None:
        """
        Get start calibration state
        Args:
            message : dict
        Returns : None
        """
        self._mount = message.get("Mount")
        self._is_calibrating = True
        self._is_guiding = False

    async def __app_state(self, message: dict) -> None:
        """
        Get app state
        Args:
            message : dict
        Returns : None
        """
        state = message.get("State")
        for case in switch(state):
            if case("Stopped"):
                self._is_calibrating = False
                self._is_looping = False
                self._is_guiding = False
                self._is_settling = False
                break
            if case("Selected"):
                self._is_selected = True
                self._is_looping = False
                self._is_guiding = False
                self._is_settling = False
                self._is_calibrating = False
                break
            if case("Calibrating"):
                self._is_calibrating = True
                self._is_guiding = False
                break
            if case("Guiding"):
                self._is_guiding = True
                self._is_calibrating = False
                break
            if case("LostLock"):
                self._is_guiding = True
                self._is_star_locked = False
                break
            if case("Paused"):
                self._is_guiding = False
                self._is_calibrating = False
                break
            if case("Looping"):
                self._is_looping = True
                break

    async def __calibration_failed(self, message: dict) -> None:
        """
        Get calibration failed state
        Args:
            message : dict
        Returns : None
        """
        self._calibrated_error = message.get("Reason")
        self._is_calibrating = False
        self._is_calibrated = False

    async def __calibration_data_flipped(self, message: dict) -> None:
        """
        Get calibration data flipping state
        Args:
            message : dict
        Returns : None
        """
        self._is_calibration_flipped = True

    async def __lock_position_shift_limit_reached(self) -> None:
        """
        Get lock position shift limit reached state
        Args : None
        Returns : None
        """
        logger.warning("Star locked position reached the edge of the camera frame")

    async def __looping_exposures(self, message: dict) -> None:
        """
        Get looping exposures state
        Args:
            message : dict
        Returns : None
        """
        self._is_looping = True

    async def __looping_exposures_stopped(self) -> None:
        """
        Get looping exposures stopped state
        Args : None
        Returns : None
        """
        self._is_looping = False

    async def __settle_begin(self) -> None:
        """
        Get settle begin state
        Args : None
        Returns : None
        """
        self._is_settling = True

    async def __settling(self, message: dict) -> None:
        """
        Get settling state
        Args:
            message : dict
        Returns : None
        """
        self._settle_status["distance"] = message.get("Distance")
        self._settle_status["time"] = message.get("SettleTime")
        self._settle_status["locked"] = message.get("StarLocked")
        self._is_settling = True

    async def __settle_done(self, message: dict) -> None:
        """
        Get settle done state
        Args:
            message : dict
        Returns : None
        """
        status = message.get("Status")
        if status == 0:
            logger.info("Settle succeeded")
            self._is_settled = True
        else:
            self._settle_error = message.get("Error")
            logger.info(f"Settle failed , error : {message.get('Error')}")
            self._is_settled = False
        self._is_settling = False

    async def __star_lost(self, message: dict) -> None:
        """
        Get star lost state
        Args:
            message : dict
        Returns : None
        """
        self._starlost_status["snr"] = message.get("SNR")
        self._starlost_status["star_mass"] = message.get("StarMass")
        self._starlost_status["avg_dist"] = message.get("AvgDist")
        self._starlost_error = message.get("Status")
        logger.error(
            f"Star Lost , SNR : {self._starlost_status['snr']} , StarMass : {self._starlost_status['star_mass']} , AvgDist : {self._starlost_status['avg_dist']}"
        )
        self._is_guiding = False
        self._is_calibrating = False

    async def __guiding_stopped(self) -> None:
        """
        Get guiding stopped state
        Args : None
        Returns : None
        """
        self._is_guiding = False
        logger.info("Guiding Stopped")

    async def __resumed(self) -> None:
        """
        Get guiding resumed state
        Args : None
        Returns : None
        """
        logger.info("Guiding Resumed")
        self._is_guiding = True

    async def __guide_step(self, message: dict) -> None:
        """
        Get guide step state
        Args:
            message : dict
        Returns : None
        """
        self._mount = message.get("Mount")
        logger.debug("Guide step mount : {}".format(self._mount))
        self._guiding_error = message.get("ErrorCode")
        logger.debug("Guide step error : {}".format(self._guiding_error))

        self._guiding_status["avg_dist"] = message.get("AvgDist")
        logger.debug(
            "Guide step average distance : {}".format(self._guiding_status["avg_dist"])
        )

        self._guiding_status["dx"] = message.get("dx")
        logger.debug("Guide step dx : {}".format(self._guiding_status["dx"]))
        self._guiding_status["dy"] = message.get("dy")
        logger.debug("Guide step dy : {}".format(self._guiding_status["dy"]))

        self._guiding_status["ra_raw_distance"] = message.get("RADistanceRaw")
        logger.debug(
            "Guide step RADistanceRaw : {}".format(
                self._guiding_status["ra_raw_distance"]
            )
        )
        self._guiding_status["dec_raw_distance"] = message.get("DECDistanceRaw")
        logger.debug(
            "Guide step DECDistanceRaw : {}".format(
                self._guiding_status["dec_raw_distance"]
            )
        )

        self._guiding_status["ra_distance"] = message.get("RADistanceGuide")
        logger.debug(
            "Guide step RADistanceGuide : {}".format(
                self._guiding_status["ra_distance"]
            )
        )
        self._guiding_status["dec_distance"] = message.get("DECDistanceGuide")
        logger.debug(
            "Guide step DECDistanceGuide : {}".format(
                self._guiding_status["dec_distance"]
            )
        )

        self._guiding_status["ra_duration"] = message.get("RADuration")
        logger.debug(
            "Guide step RADuration : {}".format(self._guiding_status["ra_duration"])
        )
        self._guiding_status["dec_duration"] = message.get("DECDuration")
        logger.debug(
            "Guide step DECDuration : {}".format(self._guiding_status["dec_duration"])
        )

        self._guiding_status["ra_direction"] = message.get("RADirection")
        logger.debug(
            "Guide step RADirection : {}".format(self._guiding_status["ra_direction"])
        )
        self._guiding_status["dec_direction"] = message.get("DECDirection")
        logger.debug(
            "Guide step DECDirection : {}".format(self._guiding_status["dec_direction"])
        )

        self._guiding_status["snr"] = message.get("SNR")
        logger.debug("Guide step SNR : {}".format(self._guiding_status["snr"]))
        self._guiding_status["starmass"] = message.get("StarMass")
        logger.debug(
            "Guide step StarMass : {}".format(self._guiding_status["starmass"])
        )
        self._guiding_status["hfd"] = message.get("HFD")
        logger.debug("Guide step HFD : {}".format(self._guiding_status["hfd"]))

    async def __guiding_dithered(self, message: dict) -> None:
        """
        Get guiding dithered state
        Args:
            message : dict
        Returns : None
        """
        self._dither_dx = message.get("dx")
        self._dither_dy = message.get("dy")

    async def __lock_position_lost(self) -> None:
        """
        Get lock position lost state
        Args : None
        Returns : None
        """
        self._is_star_locked = True
        logger.error("Star Lock Position Lost")

    async def __alert(self, message: dict) -> None:
        """
        Get alert state
        Args:
            message : dict
        Returns : None
        """
        self._last_error = message.get("Msg")
        logger.error("Alert : {}".format(self._last_error))

    async def __guide_param_change(self, message: dict) -> None:
        """
        Get guide param change state
        Args:
            message : dict
        Returns : None
        """

    async def __configuration_change(self) -> None:
        """
        Get configuration change state
        Args : None
        Returns : None
        """
