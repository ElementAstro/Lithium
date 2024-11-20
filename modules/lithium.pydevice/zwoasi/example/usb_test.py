import ctypes
from usb import USB2ST4Open, USB2ST4Close, USB2ST4GetNum, USB2ST4_ERROR_CODE, USB2ST4_SUCCESS

# 获取连接的设备数量
num_devices = USB2ST4GetNum()
print(f"Number of connected devices: {num_devices}")

# 打开第一个设备
device_index = 0
result = USB2ST4Open(device_index)
if result == USB2ST4_SUCCESS:
    print(f"Device {device_index} opened successfully.")
else:
    print(f"Failed to open device {device_index}. Error code: {result}")

# 关闭设备
result = USB2ST4Close(device_index)
if result == USB2ST4_SUCCESS:
    print(f"Device {device_index} closed successfully.")
else:
    print(f"Failed to close device {device_index}. Error code: {result}")