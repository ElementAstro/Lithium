Here is a simple INDI device infomation:
```c++
Filter Simulator.CONNECTION.CONNECT=On
Filter Simulator.CONNECTION.DISCONNECT=Off
Filter Simulator.DRIVER_INFO.DRIVER_NAME=Filter Simulator
Filter Simulator.DRIVER_INFO.DRIVER_EXEC=indi_simulator_wheel
Filter Simulator.DRIVER_INFO.DRIVER_VERSION=1.0
Filter Simulator.DRIVER_INFO.DRIVER_INTERFACE=16
Filter Simulator.CONFIG_PROCESS.CONFIG_LOAD=Off
Filter Simulator.CONFIG_PROCESS.CONFIG_SAVE=Off
Filter Simulator.CONFIG_PROCESS.CONFIG_DEFAULT=Off
Filter Simulator.CONFIG_PROCESS.CONFIG_PURGE=Off
Filter Simulator.FILTER_SLOT.FILTER_SLOT_VALUE=1
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_1=Red
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_2=Green
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_3=Blue
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_4=H_Alpha
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_5=SII
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_6=OIII
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_7=LPR
Filter Simulator.FILTER_NAME.FILTER_SLOT_NAME_8=Luminance
Filter Simulator.USEJOYSTICK.ENABLE=Off
Filter Simulator.USEJOYSTICK.DISABLE=On
Filter Simulator.SNOOP_JOYSTICK.SNOOP_JOYSTICK_DEVICE=Joystick
Filter Simulator.USEJOYSTICK.ENABLE=Off
Filter Simulator.USEJOYSTICK.DISABLE=On
Filter Simulator.SNOOP_JOYSTICK.SNOOP_JOYSTICK_DEVICE=Joystick
Focuser Simulator.CONNECTION.CONNECT=Off
Focuser Simulator.CONNECTION.DISCONNECT=On
Focuser Simulator.DRIVER_INFO.DRIVER_NAME=Focuser Simulator
Focuser Simulator.DRIVER_INFO.DRIVER_EXEC=indi_simulator_focus
Focuser Simulator.DRIVER_INFO.DRIVER_VERSION=1.0
Focuser Simulator.DRIVER_INFO.DRIVER_INTERFACE=8
Focuser Simulator.DEBUG.ENABLE=Off
Focuser Simulator.DEBUG.DISABLE=On
Focuser Simulator.POLLING_PERIOD.PERIOD_MS=1000
Focuser Simulator.CONFIG_PROCESS.CONFIG_LOAD=Off
Focuser Simulator.CONFIG_PROCESS.CONFIG_SAVE=Off
Focuser Simulator.CONFIG_PROCESS.CONFIG_DEFAULT=Off
Focuser Simulator.CONFIG_PROCESS.CONFIG_PURGE=Off
Focuser Simulator.CONNECTION_MODE.CONNECTION_SERIAL=On
Focuser Simulator.CONNECTION_MODE.CONNECTION_TCP=Off
Focuser Simulator.DEVICE_PORT.PORT=/dev/ttyUSB0
Focuser Simulator.DEVICE_BAUD_RATE.9600=On
Focuser Simulator.DEVICE_BAUD_RATE.19200=Off
Focuser Simulator.DEVICE_BAUD_RATE.38400=Off
Focuser Simulator.DEVICE_BAUD_RATE.57600=Off
Focuser Simulator.DEVICE_BAUD_RATE.115200=Off
Focuser Simulator.DEVICE_BAUD_RATE.230400=Off
Focuser Simulator.DEVICE_AUTO_SEARCH.INDI_ENABLED=On
Focuser Simulator.DEVICE_AUTO_SEARCH.INDI_DISABLED=Off
Focuser Simulator.DEVICE_PORT_SCAN.Scan Ports=Off
Focuser Simulator.Mode.All=On
Focuser Simulator.Mode.Absolute=Off
Focuser Simulator.Mode.Relative=Off
Focuser Simulator.Mode.Timer=Off
Focuser Simulator.Mode.All=On
Focuser Simulator.Mode.Absolute=Off
Focuser Simulator.Mode.Relative=Off
Focuser Simulator.Mode.Timer=Off
Telescope Simulator.CONNECTION.CONNECT=Off
Telescope Simulator.CONNECTION.DISCONNECT=On
Telescope Simulator.DRIVER_INFO.DRIVER_NAME=Telescope Simulator
Telescope Simulator.DRIVER_INFO.DRIVER_EXEC=indi_simulator_telescope
Telescope Simulator.DRIVER_INFO.DRIVER_VERSION=1.0
Telescope Simulator.DRIVER_INFO.DRIVER_INTERFACE=5
Telescope Simulator.POLLING_PERIOD.PERIOD_MS=250
Telescope Simulator.DEBUG.ENABLE=Off
Telescope Simulator.DEBUG.DISABLE=On
Telescope Simulator.CONFIG_PROCESS.CONFIG_LOAD=Off
Telescope Simulator.CONFIG_PROCESS.CONFIG_SAVE=Off
Telescope Simulator.CONFIG_PROCESS.CONFIG_DEFAULT=Off
Telescope Simulator.CONFIG_PROCESS.CONFIG_PURGE=Off
Telescope Simulator.CONNECTION_MODE.CONNECTION_SERIAL=On
Telescope Simulator.CONNECTION_MODE.CONNECTION_TCP=Off
Telescope Simulator.DEVICE_PORT.PORT=/dev/ttyUSB0
Telescope Simulator.DEVICE_BAUD_RATE.9600=On
Telescope Simulator.DEVICE_BAUD_RATE.19200=Off
Telescope Simulator.DEVICE_BAUD_RATE.38400=Off
Telescope Simulator.DEVICE_BAUD_RATE.57600=Off
Telescope Simulator.DEVICE_BAUD_RATE.115200=Off
Telescope Simulator.DEVICE_BAUD_RATE.230400=Off
Telescope Simulator.DEVICE_AUTO_SEARCH.INDI_ENABLED=On
Telescope Simulator.DEVICE_AUTO_SEARCH.INDI_DISABLED=Off
Telescope Simulator.DEVICE_PORT_SCAN.Scan Ports=Off
Telescope Simulator.ACTIVE_DEVICES.ACTIVE_GPS=GPS Simulator
Telescope Simulator.ACTIVE_DEVICES.ACTIVE_DOME=Dome Simulator
Telescope Simulator.DOME_POLICY.DOME_IGNORED=On
Telescope Simulator.DOME_POLICY.DOME_LOCKS=Off
Telescope Simulator.TELESCOPE_INFO.TELESCOPE_APERTURE=120
Telescope Simulator.TELESCOPE_INFO.TELESCOPE_FOCAL_LENGTH=900
Telescope Simulator.TELESCOPE_INFO.GUIDER_APERTURE=120
Telescope Simulator.TELESCOPE_INFO.GUIDER_FOCAL_LENGTH=900
Telescope Simulator.SCOPE_CONFIG_NAME.SCOPE_CONFIG_NAME=
Telescope Simulator.MOUNT_AXES.PRIMARY=0
Telescope Simulator.MOUNT_AXES.SECONDARY=0
Telescope Simulator.ACTIVE_DEVICES.ACTIVE_GPS=GPS Simulator
Telescope Simulator.ACTIVE_DEVICES.ACTIVE_DOME=Dome Simulator
Telescope Simulator.DOME_POLICY.DOME_IGNORED=On
Telescope Simulator.DOME_POLICY.DOME_LOCKS=Off
Telescope Simulator.TELESCOPE_INFO.TELESCOPE_APERTURE=120
Telescope Simulator.TELESCOPE_INFO.TELESCOPE_FOCAL_LENGTH=900
Telescope Simulator.TELESCOPE_INFO.GUIDER_APERTURE=120
Telescope Simulator.TELESCOPE_INFO.GUIDER_FOCAL_LENGTH=900
Telescope Simulator.SCOPE_CONFIG_NAME.SCOPE_CONFIG_NAME=
CCD Simulator.CONNECTION.CONNECT=On
CCD Simulator.CONNECTION.DISCONNECT=Off
CCD Simulator.DRIVER_INFO.DRIVER_NAME=CCD Simulator
CCD Simulator.DRIVER_INFO.DRIVER_EXEC=indi_simulator_ccd
CCD Simulator.DRIVER_INFO.DRIVER_VERSION=1.0
CCD Simulator.DRIVER_INFO.DRIVER_INTERFACE=22
CCD Simulator.POLLING_PERIOD.PERIOD_MS=1000
CCD Simulator.DEBUG.ENABLE=Off
CCD Simulator.DEBUG.DISABLE=On
CCD Simulator.CONFIG_PROCESS.CONFIG_LOAD=Off
CCD Simulator.CONFIG_PROCESS.CONFIG_SAVE=Off
CCD Simulator.CONFIG_PROCESS.CONFIG_DEFAULT=Off
CCD Simulator.CONFIG_PROCESS.CONFIG_PURGE=Off
CCD Simulator.ACTIVE_DEVICES.ACTIVE_TELESCOPE=Telescope Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_ROTATOR=Rotator Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_FOCUSER=Focuser Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_FILTER=CCD Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_SKYQUALITY=SQM
CCD Simulator.SIMULATOR_SETTINGS.SIM_XRES=1280
CCD Simulator.SIMULATOR_SETTINGS.SIM_YRES=1024
CCD Simulator.SIMULATOR_SETTINGS.SIM_XSIZE=5.2000000000000001776
CCD Simulator.SIMULATOR_SETTINGS.SIM_YSIZE=5.2000000000000001776
CCD Simulator.SIMULATOR_SETTINGS.SIM_MAXVAL=65000
CCD Simulator.SIMULATOR_SETTINGS.SIM_SATURATION=1
CCD Simulator.SIMULATOR_SETTINGS.SIM_LIMITINGMAG=17
CCD Simulator.SIMULATOR_SETTINGS.SIM_NOISE=10
CCD Simulator.SIMULATOR_SETTINGS.SIM_SKYGLOW=19.5
CCD Simulator.SIMULATOR_SETTINGS.SIM_OAGOFFSET=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_POLAR=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_POLARDRIFT=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_PEPERIOD=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_PEMAX=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_TIME_FACTOR=1
CCD Simulator.SIMULATOR_SETTINGS.SIM_ROTATION=0
CCD Simulator.EQUATORIAL_PE.RA_PE=0
CCD Simulator.EQUATORIAL_PE.DEC_PE=0
CCD Simulator.SIM_FOCUSING.SIM_FOCUS_POSITION=36700
CCD Simulator.SIM_FOCUSING.SIM_FOCUS_MAX=100000
CCD Simulator.SIM_FOCUSING.SIM_SEEING=3.5
CCD Simulator.SIMULATE_BAYER.INDI_ENABLED=Off
CCD Simulator.SIMULATE_BAYER.INDI_DISABLED=On
CCD Simulator.CCD_EXPOSURE.CCD_EXPOSURE_VALUE=1
CCD Simulator.CCD_ABORT_EXPOSURE.ABORT=Off
CCD Simulator.CCD_FRAME.X=0
CCD Simulator.CCD_FRAME.Y=0
CCD Simulator.CCD_FRAME.WIDTH=1280
CCD Simulator.CCD_FRAME.HEIGHT=1024
CCD Simulator.CCD_BINNING.HOR_BIN=1
CCD Simulator.CCD_BINNING.VER_BIN=1
CCD Simulator.GUIDER_EXPOSURE.GUIDER_EXPOSURE_VALUE=1
CCD Simulator.GUIDER_ABORT_EXPOSURE.ABORT=Off
CCD Simulator.GUIDER_FRAME.X=0
CCD Simulator.GUIDER_FRAME.Y=0
CCD Simulator.GUIDER_FRAME.WIDTH=500
CCD Simulator.GUIDER_FRAME.HEIGHT=290
CCD Simulator.CCD_TEMPERATURE.CCD_TEMPERATURE_VALUE=0
CCD Simulator.CCD_TEMP_RAMP.RAMP_SLOPE=0
CCD Simulator.CCD_TEMP_RAMP.RAMP_THRESHOLD=0.2000000000000000111
CCD Simulator.CCD_CAPTURE_FORMAT.INDI_MONO=On
CCD Simulator.CCD_TRANSFER_FORMAT.FORMAT_FITS=On
CCD Simulator.CCD_TRANSFER_FORMAT.FORMAT_NATIVE=Off
CCD Simulator.CCD_TRANSFER_FORMAT.FORMAT_XISF=Off
CCD Simulator.CCD_INFO.CCD_MAX_X=1280
CCD Simulator.CCD_INFO.CCD_MAX_Y=1024
CCD Simulator.CCD_INFO.CCD_PIXEL_SIZE=5.1999998092651367188
CCD Simulator.CCD_INFO.CCD_PIXEL_SIZE_X=5.1999998092651367188
CCD Simulator.CCD_INFO.CCD_PIXEL_SIZE_Y=5.1999998092651367188
CCD Simulator.CCD_INFO.CCD_BITSPERPIXEL=16
CCD Simulator.GUIDER_INFO.CCD_MAX_X=500
CCD Simulator.GUIDER_INFO.CCD_MAX_Y=290
CCD Simulator.GUIDER_INFO.CCD_PIXEL_SIZE=9.8000001907348632812
CCD Simulator.GUIDER_INFO.CCD_PIXEL_SIZE_X=9.8000001907348632812
CCD Simulator.GUIDER_INFO.CCD_PIXEL_SIZE_Y=12.600000381469726562
CCD Simulator.GUIDER_INFO.CCD_BITSPERPIXEL=16
CCD Simulator.GUIDER_BINNING.HOR_BIN=1
CCD Simulator.GUIDER_BINNING.VER_BIN=1
CCD Simulator.CCD_COMPRESSION.INDI_ENABLED=Off
CCD Simulator.CCD_COMPRESSION.INDI_DISABLED=On
CCD Simulator.GUIDER_COMPRESSION.INDI_ENABLED=Off
CCD Simulator.GUIDER_COMPRESSION.INDI_DISABLED=On
CCD Simulator.TELESCOPE_TIMED_GUIDE_NS.TIMED_GUIDE_N=0
CCD Simulator.TELESCOPE_TIMED_GUIDE_NS.TIMED_GUIDE_S=0
CCD Simulator.TELESCOPE_TIMED_GUIDE_WE.TIMED_GUIDE_W=0
CCD Simulator.TELESCOPE_TIMED_GUIDE_WE.TIMED_GUIDE_E=0
CCD Simulator.CCD_FRAME_TYPE.FRAME_LIGHT=On
CCD Simulator.CCD_FRAME_TYPE.FRAME_BIAS=Off
CCD Simulator.CCD_FRAME_TYPE.FRAME_DARK=Off
CCD Simulator.CCD_FRAME_TYPE.FRAME_FLAT=Off
CCD Simulator.GUIDER_FRAME_TYPE.FRAME_LIGHT=On
CCD Simulator.GUIDER_FRAME_TYPE.FRAME_BIAS=Off
CCD Simulator.GUIDER_FRAME_TYPE.FRAME_DARK=Off
CCD Simulator.GUIDER_FRAME_TYPE.FRAME_FLAT=Off
CCD Simulator.SCOPE_INFO.FOCAL_LENGTH=0
CCD Simulator.SCOPE_INFO.APERTURE=0
CCD Simulator.WCS_CONTROL.WCS_ENABLE=Off
CCD Simulator.WCS_CONTROL.WCS_DISABLE=On
CCD Simulator.UPLOAD_MODE.UPLOAD_CLIENT=On
CCD Simulator.UPLOAD_MODE.UPLOAD_LOCAL=Off
CCD Simulator.UPLOAD_MODE.UPLOAD_BOTH=Off
CCD Simulator.UPLOAD_SETTINGS.UPLOAD_DIR=/home/max
CCD Simulator.UPLOAD_SETTINGS.UPLOAD_PREFIX=IMAGE_XXX
CCD Simulator.CCD_FAST_TOGGLE.INDI_ENABLED=Off
CCD Simulator.CCD_FAST_TOGGLE.INDI_DISABLED=On
CCD Simulator.CCD_FAST_COUNT.FRAMES=1
CCD Simulator.CCD_VIDEO_STREAM.STREAM_ON=Off
CCD Simulator.CCD_VIDEO_STREAM.STREAM_OFF=On
CCD Simulator.STREAM_DELAY.STREAM_DELAY_TIME=0
CCD Simulator.STREAMING_EXPOSURE.STREAMING_EXPOSURE_VALUE=0.10000000000000000555
CCD Simulator.STREAMING_EXPOSURE.STREAMING_DIVISOR_VALUE=1
CCD Simulator.FPS.EST_FPS=30
CCD Simulator.FPS.AVG_FPS=30
CCD Simulator.RECORD_STREAM.RECORD_ON=Off
CCD Simulator.RECORD_STREAM.RECORD_DURATION_ON=Off
CCD Simulator.RECORD_STREAM.RECORD_FRAME_ON=Off
CCD Simulator.RECORD_STREAM.RECORD_OFF=On
CCD Simulator.RECORD_FILE.RECORD_FILE_DIR=/home/max/indi__D_
CCD Simulator.RECORD_FILE.RECORD_FILE_NAME=indi_record__T_
CCD Simulator.RECORD_OPTIONS.RECORD_DURATION=1
CCD Simulator.RECORD_OPTIONS.RECORD_FRAME_TOTAL=30
CCD Simulator.CCD_STREAM_FRAME.X=0
CCD Simulator.CCD_STREAM_FRAME.Y=0
CCD Simulator.CCD_STREAM_FRAME.WIDTH=1280
CCD Simulator.CCD_STREAM_FRAME.HEIGHT=1024
CCD Simulator.CCD_STREAM_ENCODER.RAW=On
CCD Simulator.CCD_STREAM_ENCODER.MJPEG=Off
CCD Simulator.CCD_STREAM_RECORDER.SER=On
CCD Simulator.LIMITS.LIMITS_BUFFER_MAX=512
CCD Simulator.LIMITS.LIMITS_PREVIEW_FPS=10
CCD Simulator.DSP_ACTIVATE_CONVOLUTION.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_CONVOLUTION.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_DFT.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_DFT.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_IDFT.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_IDFT.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_SPECTRUM.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_SPECTRUM.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_HISTOGRAM.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_HISTOGRAM.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_WAVELETS.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_WAVELETS.DSP_ACTIVATE_OFF=On
CCD Simulator.CCD_GAIN.GAIN=90
CCD Simulator.CCD_OFFSET.OFFSET=0
CCD Simulator.CCD_DIRECTORY_LOCATION.LOCATION=/home/max
CCD Simulator.CCD_DIRECTORY_TOGGLE.INDI_ENABLED=Off
CCD Simulator.CCD_DIRECTORY_TOGGLE.INDI_DISABLED=On
CCD Simulator.FILTER_SLOT.FILTER_SLOT_VALUE=1
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_1=Red
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_2=Green
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_3=Blue
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_4=H_Alpha
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_5=SII
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_6=OIII
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_7=LPR
CCD Simulator.FILTER_NAME.FILTER_SLOT_NAME_8=Luminance
CCD Simulator.ACTIVE_DEVICES.ACTIVE_TELESCOPE=Telescope Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_ROTATOR=Rotator Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_FOCUSER=Focuser Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_FILTER=CCD Simulator
CCD Simulator.ACTIVE_DEVICES.ACTIVE_SKYQUALITY=SQM
CCD Simulator.CCD_VIDEO_STREAM.STREAM_ON=Off
CCD Simulator.CCD_VIDEO_STREAM.STREAM_OFF=On
CCD Simulator.STREAMING_EXPOSURE.STREAMING_EXPOSURE_VALUE=0.10000000000000000555
CCD Simulator.STREAMING_EXPOSURE.STREAMING_DIVISOR_VALUE=1
CCD Simulator.FPS.EST_FPS=30
CCD Simulator.FPS.AVG_FPS=30
CCD Simulator.RECORD_STREAM.RECORD_ON=Off
CCD Simulator.RECORD_STREAM.RECORD_DURATION_ON=Off
CCD Simulator.RECORD_STREAM.RECORD_FRAME_ON=Off
CCD Simulator.RECORD_STREAM.RECORD_OFF=On
CCD Simulator.RECORD_FILE.RECORD_FILE_DIR=/home/max/indi__D_
CCD Simulator.RECORD_FILE.RECORD_FILE_NAME=indi_record__T_
CCD Simulator.RECORD_OPTIONS.RECORD_DURATION=1
CCD Simulator.RECORD_OPTIONS.RECORD_FRAME_TOTAL=30
CCD Simulator.CCD_STREAM_FRAME.X=0
CCD Simulator.CCD_STREAM_FRAME.Y=0
CCD Simulator.CCD_STREAM_FRAME.WIDTH=1280
CCD Simulator.CCD_STREAM_FRAME.HEIGHT=1024
CCD Simulator.CCD_STREAM_ENCODER.RAW=On
CCD Simulator.CCD_STREAM_ENCODER.MJPEG=Off
CCD Simulator.CCD_STREAM_RECORDER.SER=On
CCD Simulator.LIMITS.LIMITS_BUFFER_MAX=512
CCD Simulator.LIMITS.LIMITS_PREVIEW_FPS=10
CCD Simulator.DSP_ACTIVATE_CONVOLUTION.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_CONVOLUTION.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_DFT.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_DFT.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_IDFT.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_IDFT.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_SPECTRUM.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_SPECTRUM.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_HISTOGRAM.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_HISTOGRAM.DSP_ACTIVATE_OFF=On
CCD Simulator.DSP_ACTIVATE_WAVELETS.DSP_ACTIVATE_ON=Off
CCD Simulator.DSP_ACTIVATE_WAVELETS.DSP_ACTIVATE_OFF=On
CCD Simulator.SIMULATOR_SETTINGS.SIM_XRES=1280
CCD Simulator.SIMULATOR_SETTINGS.SIM_YRES=1024
CCD Simulator.SIMULATOR_SETTINGS.SIM_XSIZE=5.2000000000000001776
CCD Simulator.SIMULATOR_SETTINGS.SIM_YSIZE=5.2000000000000001776
CCD Simulator.SIMULATOR_SETTINGS.SIM_MAXVAL=65000
CCD Simulator.SIMULATOR_SETTINGS.SIM_SATURATION=1
CCD Simulator.SIMULATOR_SETTINGS.SIM_LIMITINGMAG=17
CCD Simulator.SIMULATOR_SETTINGS.SIM_NOISE=10
CCD Simulator.SIMULATOR_SETTINGS.SIM_SKYGLOW=19.5
CCD Simulator.SIMULATOR_SETTINGS.SIM_OAGOFFSET=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_POLAR=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_POLARDRIFT=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_PEPERIOD=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_PEMAX=0
CCD Simulator.SIMULATOR_SETTINGS.SIM_TIME_FACTOR=1
CCD Simulator.SIMULATOR_SETTINGS.SIM_ROTATION=0
CCD Simulator.EQUATORIAL_PE.RA_PE=0
CCD Simulator.EQUATORIAL_PE.DEC_PE=0
CCD Simulator.SIM_FOCUSING.SIM_FOCUS_POSITION=36700
CCD Simulator.SIM_FOCUSING.SIM_FOCUS_MAX=100000
CCD Simulator.SIM_FOCUSING.SIM_SEEING=3.5
CCD Simulator.SIMULATE_BAYER.INDI_ENABLED=Off
CCD Simulator.SIMULATE_BAYER.INDI_DISABLED=On
Telescope Simulator.MOUNT_AXES.PRIMARY=0
Telescope Simulator.MOUNT_AXES.SECONDARY=0
```
