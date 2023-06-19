from star_search_model import search_star_coordinates
from angle_compute_try import calculate_alt_az
from astropy.time import Time
import astropy.units as u
from astropy.coordinates import EarthLocation, AltAz
local_time = Time("2023-06-11 20:10:40")
local_longitude = 120.15 * u.deg  # 东经120°9′
local_latitude = 30.28 * u.deg   # 北纬30°17′
star_code = input("请输入星体编号: ")
coordinates = search_star_coordinates(star_code)
RA = coordinates[7]
DEC = coordinates[8]
# 时间信息 time = Time.now()  # 当前时间，或者指定一个特定的时间

location = EarthLocation(lon=local_longitude, lat=local_latitude)
altitude, azimuth = calculate_alt_az(location, RA, DEC, local_time)
# local_time="2023/6/10 10:44:53"#时间根据需要修改
print("当前度角:", altitude)
print("当前方位角:", azimuth)
print("赤经:", RA)
print("赤纬:",DEC)