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

import datetime
import ephem
import numpy as np
from typing import List, Dict


def parser(observer: ephem.Observer, body) -> List[str]:
    """
    Parse rise, transit, and set times for a given celestial body.

    Args:
        observer (ephem.Observer): The observer's location and time.
        body: The celestial body to observe.

    Returns:
        List[str]: The rise, transit, and set times.
    """
    positions = []

    try:
        # Check for body rise, transit, and set times.
        if (
            ephem.localtime(observer.previous_rising(body)).date(
            ) == ephem.localtime(observer.date).date()
            and observer.previous_rising(body) < observer.previous_transit(body) < observer.previous_setting(body)
            < observer.date
        ):
            positions.extend([
                observer.previous_rising(body),
                observer.previous_transit(body),
                observer.previous_setting(body)
            ])
        elif (
            ephem.localtime(observer.previous_rising(body)).date(
            ) == ephem.localtime(observer.date).date()
            and observer.previous_rising(body) < observer.previous_transit(body) < observer.date
            < observer.next_setting(body)
        ):
            positions.extend([
                observer.previous_rising(body),
                observer.previous_transit(body),
                observer.next_setting(body)
            ])
        elif (
            ephem.localtime(observer.previous_rising(body)).date(
            ) == ephem.localtime(observer.date).date()
            and observer.previous_rising(body) < observer.date < observer.next_transit(body)
            < observer.next_setting(body)
        ):
            positions.extend([
                observer.previous_rising(body),
                observer.next_transit(body),
                observer.next_setting(body)
            ])
        elif (
            ephem.localtime(observer.previous_rising(body)).date(
            ) == ephem.localtime(observer.date).date()
            and observer.date < observer.next_rising(body) < observer.next_transit(body)
            < observer.next_setting(body)
        ):
            positions.extend([
                observer.next_rising(body),
                observer.next_transit(body),
                observer.next_setting(body)
            ])
        else:
            positions.extend([
                observer.next_rising(body),
                observer.next_transit(body),
                observer.next_setting(body)
            ])
    except (ephem.NeverUpError, ephem.AlwaysUpError):
        try:
            if (
                ephem.localtime(observer.previous_transit(body)).date(
                ) == ephem.localtime(observer.date).date()
                and observer.previous_transit(body) < observer.date
            ):
                positions.extend(["-", observer.previous_transit(body), "-"])
            elif (
                ephem.localtime(observer.previous_transit(body)).date(
                ) == ephem.localtime(observer.date).date()
                and observer.next_transit(body) > observer.date
            ):
                positions.extend(["-", observer.next_transit(body), "-"])
            else:
                positions.extend(["-", "-", "-"])
        except (ephem.NeverUpError, ephem.AlwaysUpError):
            positions.extend(["-", "-", "-"])

    positions = [ephem.localtime(pos).strftime(
        "%H:%M:%S") if pos != "-" else "-" for pos in positions]

    return positions


class Moon:
    """
    Class to represent and fetch information about the Moon.
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        self.home.date = datetime.datetime.utcnow()

    @property
    def get_next_full(self) -> str:
        """
        Get the time of the next full moon.

        Returns:
            str: The date of the next full moon.
        """
        return ephem.localtime(ephem.next_full_moon(self.home.date)).date()

    @property
    def get_next_new(self) -> str:
        """
        Get the time of the next new moon.

        Returns:
            str: The date of the next new moon.
        """
        return ephem.localtime(ephem.next_new_moon(self.home.date)).date()

    @property
    def get_previous_full(self) -> str:
        """
        Get the time of the last full moon.

        Returns:
            str: The date of the last full moon.
        """
        return ephem.localtime(ephem.previous_full_moon(self.home.date)).date()

    @property
    def get_previous_new(self) -> str:
        """
        Get the time of the last new moon.

        Returns:
            str: The date of the last new moon.
        """
        return ephem.localtime(ephem.previous_new_moon(self.home.date)).date()

    @property
    def get_next_last_quarter(self) -> str:
        """
        Get the time of the next last quarter moon.

        Returns:
            str: The date of the next last quarter moon.
        """
        return ephem.localtime(ephem.next_last_quarter_moon(self.home.date)).date()

    @property
    def get_next_first_quarter(self) -> str:
        """
        Get the time of the next first quarter moon.

        Returns:
            str: The date of the next first quarter moon.
        """
        return ephem.localtime(ephem.next_first_quarter_moon(self.home.date)).date()

    @property
    def get_previous_last_quarter(self) -> str:
        """
        Get the time of the last quarter moon.

        Returns:
            str: The date of the last quarter moon.
        """
        return ephem.localtime(ephem.previous_last_quarter_moon(self.home.date)).date()

    @property
    def get_previous_first_quarter(self) -> str:
        """
        Get the time of the previous first quarter moon.

        Returns:
            str: The date of the previous first quarter moon.
        """
        return ephem.localtime(ephem.previous_first_quarter_moon(self.home.date)).date()

    def get_moon_phase(self) -> str:
        """
        Get the current moon phase.

        Returns:
            str: The current moon phase.
        """
        target_date_local = ephem.localtime(self.home.date).date()

        if target_date_local in (self.get_next_full, self.get_previous_full):
            return "Full"
        elif target_date_local in (self.get_next_new, self.get_previous_new):
            return "New"
        elif target_date_local in (self.get_next_first_quarter, self.get_previous_first_quarter):
            return "First Quarter"
        elif target_date_local in (self.get_next_last_quarter, self.get_previous_last_quarter):
            return "Last Quarter"
        elif (
            self.get_previous_new < self.get_next_first_quarter < self.get_next_full
            < self.get_next_last_quarter < self.get_next_new
        ):
            return "Waxing Crescent"
        elif (
            self.get_previous_first_quarter < self.get_next_full < self.get_next_last_quarter
            < self.get_next_new < self.get_next_first_quarter
        ):
            return "Waxing Gibbous"
        elif (
            self.get_previous_full < self.get_next_last_quarter < self.get_next_new
            < self.get_next_first_quarter < self.get_next_full
        ):
            return "Waning Gibbous"
        elif (
            self.get_previous_last_quarter < self.get_next_new < self.get_next_first_quarter
            < self.get_next_full < self.get_next_last_quarter
        ):
            return "Waning Crescent"

    def get_moon_ra(self) -> str:
        """
        Get the current RA of the moon.

        Returns:
            str: The current RA of the moon.
        """
        return ephem.Moon(self.home).ra

    def get_moon_dec(self) -> str:
        """
        Get the current DEC of the moon.

        Returns:
            str: The current DEC of the moon.
        """
        return ephem.Moon(self.home).dec

    def get_moon_az(self) -> str:
        """
        Get the current AZ of the moon.

        Returns:
            str: The current AZ of the moon.
        """
        return f"{np.degrees(ephem.Moon(self.home).az):.2f}°"

    def get_moon_alt(self) -> str:
        """
        Get the current ALT of the moon.

        Returns:
            str: The current ALT of the moon.
        """
        return f"{np.degrees(ephem.Moon(self.home).alt):.2f}°"

    def get_moon_rise(self) -> str:
        """
        Get the rise time of the moon.

        Returns:
            str: The rise time of the moon.
        """
        return parser(self.        home, ephem.Moon(self.home))[0]

    def get_moon_set(self) -> str:
        """
        Get the set time of the moon.

        Returns:
            str: The set time of the moon.
        """
        return parser(self.home, ephem.Moon(self.home))[2]

    def get_moon_transit(self) -> str:
        """
        Get the transit time of the moon.

        Returns:
            str: The transit time of the moon.
        """
        return parser(self.home, ephem.Moon(self.home))[1]


class Sun:
    """
    Class to represent and fetch information about the Sun.
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        self.home.date = datetime.datetime.utcnow()

    @property
    def get_astro_twilight_start(self) -> str:
        """
        Get the start time of astronomical twilight.

        Returns:
            str: The start time of astronomical twilight.
        """
        return self.get_sun_twilights()[2][0]

    @property
    def get_astro_twilight_end(self) -> str:
        """
        Get the end time of astronomical twilight.

        Returns:
            str: The end time of astronomical twilight.
        """
        return self.get_sun_twilights()[2][1]

    @property
    def get_civil_twilight_start(self) -> str:
        """
        Get the start time of civil twilight.

        Returns:
            str: The start time of civil twilight.
        """
        return self.get_sun_twilights()[0][0]

    @property
    def get_civil_twilight_end(self) -> str:
        """
        Get the end time of civil twilight.

        Returns:
            str: The end time of civil twilight.
        """
        return self.get_sun_twilights()[0][1]

    def get_sun_twilights(self) -> List[List[str]]:
        """
        Get the sun twilights for the given observer.

        Returns:
            List[List[str]]: The twilight times.
        """
        results = []
        observer_horizon = self.home.horizon
        twilights = [("-6", True), ("-12", True), ("-18", True)]

        for horizon, _ in twilights:
            self.home.horizon = horizon
            try:
                rising_setting = parser(self.home, ephem.Sun(self.home))
                results.append([rising_setting[0], rising_setting[2]])
            except ephem.AlwaysUpError:
                results.append(["n/a", "n/a"])

        self.home.horizon = observer_horizon
        return results

    def get_sun_ra(self) -> str:
        """
        Get the current RA of the sun.

        Returns:
            str: The current RA of the sun.
        """
        return ephem.Sun(self.home).ra

    def get_sun_dec(self) -> str:
        """
        Get the current DEC of the sun.

        Returns:
            str: The current DEC of the sun.
        """
        return ephem.Sun(self.home).dec

    def get_sun_az(self) -> str:
        """
        Get the current AZ of the sun.

        Returns:
            str: The current AZ of the sun.
        """
        return f"{np.degrees(ephem.Sun(self.home).az):.2f}°"

    def get_sun_alt(self) -> str:
        """
        Get the current ALT of the sun.

        Returns:
            str: The current ALT of the sun.
        """
        return f"{np.degrees(ephem.Sun(self.home).alt):.2f}°"

    def get_sun_rise(self) -> str:
        """
        Get the rise time of the sun.

        Returns:
            str: The rise time of the sun.
        """
        return parser(self.home, ephem.Sun(self.home))[0]

    def get_sun_set(self) -> str:
        """
        Get the set time of the sun.

        Returns:
            str: The set time of the sun.
        """
        return parser(self.home, ephem.Sun(self.home))[2]

    def get_sun_transit(self) -> str:
        """
        Get the transit time of the sun.

        Returns:
            str: The transit time of the sun.
        """
        return parser(self.home, ephem.Sun(self.home))[1]


class OtherPlanet:
    """
    Class to represent and fetch information about other planets in our solar system.
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        self.home.date = datetime.datetime.utcnow()

    def mercury(self) -> Dict[str, str]:
        """
        Get real-time information of Mercury.

        Returns:
            Dict[str, str]: Mercury's observational data.
        """
        mercury = ephem.Mercury(self.home)
        return {
            "mercury_rise": parser(self.home, mercury)[0],
            "mercury_transit": parser(self.home, mercury)[1],
            "mercury_set": parser(self.home, mercury)[2],
            "mercury_az": f"{np.degrees(mercury.az):.2f}°",
            "mercury_alt": f"{np.degrees(mercury.alt):.2f}°",
            "mercury_ra": mercury.ra,
            "mercury_dec": mercury.dec,
        }

    def venus(self) -> Dict[str, str]:
        """
        Get real-time information of Venus.

        Returns:
            Dict[str, str]: Venus' observational data.
        """
        venus = ephem.Venus(self.home)
        return {
            "venus_rise": parser(self.home, venus)[0],
            "venus_transit": parser(self.home, venus)[1],
            "venus_set": parser(self.home, venus)[2],
            "venus_az": f"{np.degrees(venus.az):.2f}°",
            "venus_alt": f"{np.degrees(venus.alt):.2f}°",
            "venus_ra": venus.ra,
            "venus_dec": venus.dec,
        }

    def mars(self) -> Dict[str, str]:
        """
        Get real-time information of Mars.

        Returns:
            Dict[str, str]: Mars' observational data.
        """
        mars = ephem.Mars(self.home)
        return {
            "mars_rise": parser(self.home, mars)[0],
            "mars_transit": parser(self.home, mars)[1],
            "mars_set": parser(self.home, mars)[2],
            "mars_az": f"{np.degrees(mars.az):.2f}°",
            "mars_alt": f"{np.degrees(mars.alt):.2f}°",
            "mars_ra": mars.ra,
            "mars_dec": mars.dec,
        }

    def jupiter(self) -> Dict[str, str]:
        """
        Get real-time information of Jupiter.

        Returns:
            Dict[str, str]: Jupiter's observational data.
        """
        jupiter = ephem.Jupiter(self.home)
        return {
            "jupiter_rise": parser(self.home, jupiter)[0],
            "jupiter_transit": parser(self.home, jupiter)[1],
            "jupiter_set": parser(self.home, jupiter)[2],
            "jupiter_az": f"{np.degrees(jupiter.az):.2f}°",
            "jupiter_alt": f"{np.degrees(jupiter.alt)::.2f}°",
            "jupiter_ra": jupiter.ra,
            "jupiter_dec": jupiter.dec,
        }

    def saturn(self) -> Dict[str, str]:
        """
        Get real-time information of Saturn.

        Returns:
            Dict[str, str]: Saturn's observational data.
        """
        saturn = ephem.Saturn(self.home)
        return {
            "saturn_rise": parser(self.home, saturn)[0],
            "saturn_transit": parser(self.home, saturn)[1],
            "saturn_set": parser(self.home, saturn)[2],
            "saturn_az": f"{np.degrees(saturn.az):.2f}°",
            "saturn_alt": f"{np.degrees(saturn.alt):.2f}°",
            "saturn_ra": saturn.ra,
            "saturn_dec": saturn.dec,
        }

    def uranus(self) -> Dict[str, str]:
        """
        Get real-time information of Uranus.

        Returns:
            Dict[str, str]: Uranus' observational data.
        """
        uranus = ephem.Uranus(self.home)
        return {
            "uranus_rise": parser(self.home, uranus)[0],
            "uranus_transit": parser(self.home, uranus)[1],
            "uranus_set": parser(self.home, uranus)[2],
            "uranus_az": f"{np.degrees(uranus.az)::.2f}°",
            "uranus_alt": f"{np.degrees(uranus.alt):.2f}°",
            "uranus_ra": uranus.ra,
            "uranus_dec": uranus.dec,
        }

    def neptune(self) -> Dict[str, str]:
        """
        Get real-time information of Neptune.

        Returns:
            Dict[str, str]: Neptune's observational data.
        """
        neptune = ephem.Neptune(self.home)
        return {
            "neptune_rise": parser(self.home, neptune)[0],
            "neptune_transit": parser(self.home, neptune)[1],
            "neptune_set": parser(self.home, neptune)[2],
            "neptune_az": f"{np.degrees(neptune.az):.2f}°",
            "neptune_alt": f"{np.degrees(neptune.alt):.2f}°",
            "neptune_ra": neptune.ra,
            "neptune_dec": neptune.dec,
        }


class NorthPolar:
    """
    Class to represent and fetch information about Polaris.
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        self.home.date = datetime.datetime.utcnow()

    def get_polaris_data(self) -> List:
        """
        Get the polar data for a given observer.

        Returns:
            List: Polaris hour angle, next transit time, and altitude.
        """
        polaris_data = []

        j2000 = ephem.Date("2000/01/01 12:00:00")
        d = self.home.date - j2000

        lon = np.rad2deg(float(self.home.lon))

        utstr = self.home.date.datetime().strftime("%H:%M:%S")
        ut = sum(float(x) / 60 ** i for i, x in enumerate(utstr.split(":")))

        lst = 100.46 + 0.985647 * d + lon + 15 * ut
        lst %= 360

        polaris = ephem.readdb(
            "Polaris,f|M|F7,2:31:48.704,89:15:50.72,2.02,2000")
        polaris.compute(self.home)
        polaris_ra_deg = np.rad2deg(float(polaris.ra))

        # Polaris Hour Angle = LST - RA Polaris
        pha = lst - polaris_ra_deg
        pha = pha % 360

        polaris_data.append(pha)

        try:
            polaris_data.append(
                ephem.localtime(self.home.next_transit(
                    polaris)).strftime("%H:%M:%S")
            )
        except (ephem.NeverUpError, ephem.AlwaysUpError):
            polaris_data.append("-")

        polaris_data.append(f"{np.degrees(polaris.alt):.2f}°")

        return polaris_data

    def get_polar_info(self) -> Dict[str, str]:
        """
        Get Polaris information.

        Returns:
            Dict[str, str]: Information about Polaris.
        """
        polaris_data = self.get_polaris_data()
        return {
            "polaris_hour_angle": f"{polaris_data[0]:.2f}°",
            "polaris_next_transit": polaris_data[1],
            "polaris_alt": polaris_data[2],
        }

if __name__ == "__main__":
    import datetime

    # 示例：获取月亮信息
    moon = Moon(lat="37.7749", lon="-122.4194", elevation=10)  # 例如，旧金山的纬度和经度
    print("下次满月时间:", moon.get_next_full)
    print("下次新月时间:", moon.get_next_new)
    print("上次满月时间:", moon.get_previous_full)
    print("当前月相:", moon.get_moon_phase())
    print("月亮赤经:", moon.get_moon_ra())
    print("月亮赤纬:", moon.get_moon_dec())
    print("月亮方位角:", moon.get_moon_az())
    print("月亮高度角:", moon.get_moon_alt())
    print("月亮升起时间:", moon.get_moon_rise())
    print("月亮中天时间:", moon.get_moon_transit())
    print("月亮落下时间:", moon.get_moon_set())

    # 示例：获取太阳信息
    sun = Sun(lat="37.7749", lon="-122.4194", elevation=10)
    print("\n天文黎明开始时间:", sun.get_astro_twilight_start)
    print("天文黎明结束时间:", sun.get_astro_twilight_end)
    print("民用黎明开始时间:", sun.get_civil_twilight_start)
    print("民用黎明结束时间:", sun.get_civil_twilight_end)
    print("太阳赤经:", sun.get_sun_ra())
    print("太阳赤纬:", sun.get_sun_dec())
    print("太阳方位角:", sun.get_sun_az())
    print("太阳高度角:", sun.get_sun_alt())
    print("太阳升起时间:", sun.get_sun_rise())
    print("太阳中天时间:", sun.get_sun_transit())
    print("太阳落下时间:", sun.get_sun_set())

    # 示例：获取其他行星信息（以金星为例）
    planets = OtherPlanet(lat="37.7749", lon="-122.4194", elevation=10)
    venus_info = planets.venus()
    print("\n金星升起时间:", venus_info["venus_rise"])
    print("金星中天时间:", venus_info["venus_transit"])
    print("金星落下时间:", venus_info["venus_set"])
    print("金星方位角:", venus_info["venus_az"])
    print("金星高度角:", venus_info["venus_alt"])
    print("金星赤经:", venus_info["venus_ra"])
    print("金星赤纬:", venus_info["venus_dec"])

    # 示例：获取北极星信息
    north_polar = NorthPolar(lat="37.7749", lon="-122.4194", elevation=10)
    polaris_info = north_polar.get_polar_info()
    print("\n北极星时角:", polaris_info["polaris_hour_angle"])
    print("北极星下次中天时间:", polaris_info["polaris_next_transit"])
    print("北极星高度角:", polaris_info["polaris_alt"])
