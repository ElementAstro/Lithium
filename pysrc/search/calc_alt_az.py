import math
from ephem import *

def utc2jd(t):
    # 将UTC时间转换为儒略日
    try:
        s = (Date(t)).tuple()
        y = s[0]
        m = s[1]
        d = s[2]
        h = s[3]
        dh = h + s[4] / 60.0 + s[5] / 3600.0
        if m <= 2:
            y = y - 1
            m = m + 12
        day_num = int(365.25 * y) + int(30.6001 * (m + 1)) + d + int(dh / 24.0 + 1720981.5)
        hour_num = (h + 12) % 24
        jd = day_num + hour_num / 24 + s[4] / 1440 + s[5] / 86400
        return jd
    except Exception as e:
        print("Error: ", e)

def sidereal_t(t):
    # 计算格林尼治恒星时
    try:
        jd = utc2jd(t)
        mjd = jd - 2451545.0
        Tu = mjd / 36525
        s0 = 6.697374558 + 2400.05134 * Tu + 0.000025862333 * Tu * Tu - 0.000000001722222 * Tu * Tu * Tu  
        s = (Date(t)).tuple()
        M = s[3] + s[4] / 60.0 + s[5] / 3600.0 
        s = s0 + M * 366.2422 / 365.2422  
        s = s % 24
        return s
    except Exception as e:
        print("Error: ", e)

def local_sidereal_t(obs, t):
    # 计算本地恒星时
    try:
        s = sidereal_t(t) 
        ls = s + obs.lon * 180 / math.pi / 15 
        ls = ls % 24
        return ls
    except Exception as e:
        print("Error: ", e)

def calculate_alt_az(local_time, local_longitude, local_latitude, RA, DEC):
    # 计算天体方位角高度角
    try:
        observer = Observer()
        observer.lat = local_latitude
        observer.lon = local_longitude
        observer.elevation = 40 
        observer.pressure = 0 
        observer.date=local_time
        target = FixedBody()
        target._ra = RA
        target._dec = DEC
        target._epoch = observer.date 
        target.compute(observer)
        t = observer.date
        ls = local_sidereal_t(observer, t)
        ha = ls - target._ra * 180 / math.pi / 15  # 天体的时角（单位角时），赤经加上时角等于恒星时
        ha = ha * 15 * math.pi / 180  # 角时转为弧度
        sina = -math.cos(target._dec) * math.sin(ha)
        cosa = math.cos(observer.lat) * math.sin(target._dec) - math.cos(target._dec) * math.sin(observer.lat) * math.cos(ha)
        A = math.atan2(sina, cosa)
        if A < 0:
            A = A + 2 * math.pi
        h = math.asin(math.sin(observer.lat) * math.sin(target._dec) + math.cos(observer.lat) * math.cos(target._dec) * math.cos(ha))
        return degrees(A), degrees(h)
    except Exception as e:
        print("Error: ", e)
