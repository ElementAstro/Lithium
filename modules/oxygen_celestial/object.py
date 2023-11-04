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
import numpy


def parser(observer: ephem.Observer, body) -> list:
    positions = []

    # test for always below horizon or always above horizon
    try:
        if (
            ephem.localtime(observer.previous_rising(body)).date()
            == ephem.localtime(observer.date).date()
            and observer.previous_rising(body)
            < observer.previous_transit(body)
            < observer.previous_setting(body)
            < observer.date
        ):
            positions.append(observer.previous_rising(body))
            positions.append(observer.previous_transit(body))
            positions.append(observer.previous_setting(body))
        elif ephem.localtime(observer.previous_rising(body)).date() == ephem.localtime(
            observer.date
        ).date() and observer.previous_rising(body) < observer.previous_transit(
            body
        ) < observer.date < observer.next_setting(
            body
        ):
            positions.append(observer.previous_rising(body))
            positions.append(observer.previous_transit(body))
            positions.append(observer.next_setting(body))
        elif ephem.localtime(observer.previous_rising(body)).date() == ephem.localtime(
            observer.date
        ).date() and observer.previous_rising(
            body
        ) < observer.date < observer.next_transit(
            body
        ) < observer.next_setting(
            body
        ):
            positions.append(observer.previous_rising(body))
            positions.append(observer.next_transit(body))
            positions.append(observer.next_setting(body))
        elif ephem.localtime(observer.previous_rising(body)).date() == ephem.localtime(
            observer.date
        ).date() and observer.date < observer.next_rising(body) < observer.next_transit(
            body
        ) < observer.next_setting(
            body
        ):
            positions.append(observer.next_rising(body))
            positions.append(observer.next_transit(body))
            positions.append(observer.next_setting(body))
        else:
            positions.append(observer.next_rising(body))
            positions.append(observer.next_transit(body))
            positions.append(observer.next_setting(body))
    except (ephem.NeverUpError, ephem.AlwaysUpError):
        try:
            if (
                ephem.localtime(observer.previous_transit(body)).date()
                == ephem.localtime(observer.date).date()
                and observer.previous_transit(body) < observer.date
            ):
                positions.append("-")
                positions.append(observer.previous_transit(body))
                positions.append("-")
            elif (
                ephem.localtime(observer.previous_transit(body)).date()
                == ephem.localtime(observer.date).date()
                and observer.next_transit(body) > observer.date
            ):
                positions.append("-")
                positions.append(observer.next_transit(body))
                positions.append("-")
            else:
                positions.append("-")
                positions.append("-")
                positions.append("-")
        except (ephem.NeverUpError, ephem.AlwaysUpError):
            positions.append("-")
            positions.append("-")
            positions.append("-")
    if positions[0] != "-":
        positions[0] = ephem.localtime(positions[0]).strftime("%H:%M:%S")
    if positions[1] != "-":
        positions[1] = ephem.localtime(positions[1]).strftime("%H:%M:%S")
    if positions[2] != "-":
        positions[2] = ephem.localtime(positions[2]).strftime("%H:%M:%S")
    return positions


class Moon(object):
    """
    Show some infomation about moon
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        # update time
        self.home.date = datetime.datetime.utcnow()

    @property
    def get_next_full(self) -> str:
        """
        Get the time of the next full moon
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.next_full_moon(self.home.date)).date()

    @property
    def get_net_new(self) -> str:
        """
        Get the time of the next new moon
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.next_new_moon(self.home.date)).date()

    @property
    def get_previous_full(self) -> str:
        """
        Get the time of the last full moon
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.previous_full_moon(self.home.date)).date()

    @property
    def get_previous_new(self) -> str:
        """
        Get the time of the last new moon
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.previous_new_moon(self.home.date)).date()

    @property
    def get_next_last_quarter(self) -> str:
        """
        Get the time of the last quarter month
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.next_last_quarter_moon(self.home.date)).date()

    @property
    def get_next_first_quarter(self) -> str:
        """
        Get the time of the first quarter month
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.next_first_quarter_moon(self.home.date)).date()

    @property
    def get_previous_last_quarter(self) -> str:
        """
        Get the time of the previous last quarter month
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.previous_last_quarter_moon(self.home.date)).date()

    @property
    def get_previous_first_quarter(self) -> str:
        """
        Get the time of the previous first quarter month
        Args : None
        Returns : str
        """
        return ephem.localtime(ephem.previous_first_quarter_moon(self.home.date)).date()

    def get_moon_phase(self) -> str:
        """
        Get the current moon phase
        Args : None
        Returns : str
        """

        target_date_local = ephem.localtime(self.home.date).date()

        next_full = self.get_next_full
        next_new = self.get_net_new
        next_last_quarter = self.get_next_last_quarter
        next_first_quarter = self.get_next_first_quarter
        previous_full = self.get_previous_full
        previous_new = self.get_previous_new
        previous_last_quarter = self.get_previous_last_quarter
        previous_first_quarter = self.get_previous_first_quarter

        if target_date_local in (next_full, previous_full):
            return "Full"
        elif target_date_local in (next_new, previous_new):
            return "New"
        elif target_date_local in (next_first_quarter, previous_first_quarter):
            return "First Quarter"
        elif target_date_local in (next_last_quarter, previous_last_quarter):
            return "Last Quarter"
        elif (
            previous_new < next_first_quarter < next_full < next_last_quarter < next_new
        ):
            return "Waxing Crescent"
        elif (
            previous_first_quarter
            < next_full
            < next_last_quarter
            < next_new
            < next_first_quarter
        ):
            return "Waxing Gibbous"
        elif (
            previous_full
            < next_last_quarter
            < next_new
            < next_first_quarter
            < next_full
        ):
            return "Waning Gibbous"
        elif (
            previous_last_quarter
            < next_new
            < next_first_quarter
            < next_full
            < next_last_quarter
        ):
            return "Waning Crescent"

    def get_moon_ra(self) -> str:
        """
        Get the current RA of the moon
        Args : None
        Returns : str
        """
        return ephem.Moon(self.home).ra

    def get_moon_dec(self) -> str:
        """
        Get the current DEC of the moon
        Args : None
        Returns : str
        """
        return ephem.Moon(self.home).dec

    def get_moon_az(self) -> str:
        """
        Get the current AZ of the moon
        Args : None
        Returns : str
        """
        return numpy.degrees(ephem.Moon(self.home).az)

    def get_moon_dec(self) -> str:
        """
        Get the current ALT of the moon
        Args : None
        Returns : str
        """
        return numpy.degrees(ephem.Moon(self.home).alt)

    def get_moon_rise(self) -> str:
        """
        Get the rise time of the moon
        Args : None
        Returns : str
        """
        return parser(self.home, ephem.Moon(self.home))[0]

    def get_moon_set(self) -> str:
        """
        Get the set time of the moon
        Args : None
        Returns : str
        """
        return parser(self.home, ephem.Moon(self.home))[2]

    def get_moon_transmit(self) -> str:
        """
        Get the transmit time of the moon
        Args : None
        Returns : str
        """
        return parser(self.home, ephem.Moon(self.home))[1]


class Sun(object):
    """
    Get some infomation about sun
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        # update time
        self.home.date = datetime.datetime.utcnow()

    @property
    def get_astro_twilight_start(self) -> str:
        """
        Get the start time of astronomical twilight
        Args : None
        Returns : str
        """
        return self.get_sun_twilights(self.home)[2][0]

    @property
    def get_astro_twilight_stop(self) -> str:
        """
        Get the stop time of astronomical twilight
        Args : None
        Returns : str
        """
        return self.get_sun_twilights(self.home)[2][1]

    @property
    def get_civil_twilight_start(self) -> str:
        """
        Get the start time of civil twilight
        Args : None
        Returns : str
        """
        return self.get_sun_twilights(self.home)[0][0]

    @property
    def get_civil_twilight_stop(self) -> str:
        """
        Get the stop time of civil twilight
        Args : None
        Returns : str
        """
        return self.get_sun_twilights(self.home)[0][1]

    def get_sun_twilights(self) -> list:
        """
        Get the sun-twilights for the given observer
        Args :
            observer : ephem.Observer
        Returns : list
        """
        results = []
        # remember entry observer horizon
        observer_horizon = self.home.horizon
        # Twilights, their horizons and whether to use the centre of the Sun or not
        twilights = [("-6", True), ("-12", True), ("-18", True)]
        for twi in twilights:
            self.home.horizon = twi[0]
            try:
                rising_setting = parser(self.home, ephem.Sun(self.home))
                results.append((rising_setting[0], rising_setting[2]))
            except ephem.AlwaysUpError:
                results.append(("n/a", "n/a"))
        # reset observer horizon to entry
        self.home.horizon = observer_horizon
        return results

    def get_sun_ra(self) -> str:
        """
        Get the current RA of the sun
        Args : None
        Returns : str
        """
        return ephem.Sun(self.home).ra

    def get_sun_dec(self) -> str:
        """
        Get the current DEC of the sun
        Args : None
        Returns : str
        """
        return ephem.Sun(self.home).dec

    def get_sun_az(self) -> str:
        """
        Get the current AZ of the sun
        Args : None
        Returns : str
        """
        return numpy.degrees(ephem.Sun(self.home).az)

    def get_sun_dec(self) -> str:
        """
        Get the current ALT of the sun
        Args : None
        Returns : str
        """
        return numpy.degrees(ephem.Sun(self.home).alt)

    def get_sun_rise(self) -> str:
        """
        Get the rise time of the sun
        Args : None
        Returns : str
        """
        return parser(self.home, ephem.Sun(self.home))[0]

    def get_sun_set(self) -> str:
        """
        Get the set time of the sun
        Args : None
        Returns : str
        """
        return parser(self.home, ephem.Sun(self.home))[2]

    def get_sun_transmit(self) -> str:
        """
        Get the transmit time of the sun
        Args : None
        Returns : str
        """
        return parser(self.home, ephem.Sun(self.home))[1]


class OtherPlanet(object):
    """
    Show infomation about other planets in our solar system
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        # update time
        self.home.date = datetime.datetime.utcnow()

    def mercury(self) -> dict:
        """
        Get real-time information of Mercury
        Args : None
        Returns : dict
        """
        return {
            "mercury_rise": "%s" % parser(self.home, ephem.Mercury(self.home))[0],
            "mercury_transit": "%s" % parser(self.home, ephem.Mercury(self.home))[1],
            "mercury_set": "%s" % parser(self.home, ephem.Mercury(self.home))[2],
            "mercury_az": "%.2f°" % numpy.degrees(ephem.Mercury(self.home).az),
            "mercury_alt": "%.2f°" % numpy.degrees(ephem.Mercury(self.home).alt),
            "mercury_ra": "%s" % ephem.Mercury(self.home).ra,
            "mercury_dec": "%s" % ephem.Mercury(self.home).dec,
        }

    def venus(self) -> dict:
        """
        Get real-time information of Venus
        Args : None
        Returns : dict
        """
        return {
            "venus_rise": "%s" % parser(self.home, ephem.Venus(self.home))[0],
            "venus_transit": "%s" % parser(self.home, ephem.Venus(self.home))[1],
            "venus_set": "%s" % parser(self.home, ephem.Venus(self.home))[2],
            "venus_az": "%.2f°" % numpy.degrees(ephem.Venus(self.home).az),
            "venus_alt": "%.2f°" % numpy.degrees(ephem.Venus(self.home).alt),
            "venus_ra": "%s" % ephem.Venus(self.home).ra,
            "venus_dec": "%s" % ephem.Venus(self.home).dec,
        }

    def Mars(self) -> dict:
        """"""
        return {
            "mars_rise": "%s" % parser(self.home, ephem.Mars(self.home))[0],
            "mars_transit": "%s" % parser(self.home, ephem.Mars(self.home))[1],
            "mars_set": "%s" % parser(self.home, ephem.Mars(self.home))[2],
            "mars_az": "%.2f°" % numpy.degrees(ephem.Mars(self.home).az),
            "mars_alt": "%.2f°" % numpy.degrees(ephem.Mars(self.home).alt),
        }

    def Jupiter(self) -> dict:
        """"""
        return {
            "jupiter_rise": "%s" % parser(self.home, ephem.Jupiter(self.home))[0],
            "jupiter_transit": "%s" % parser(self.home, ephem.Jupiter(self.home))[1],
            "jupiter_set": "%s" % parser(self.home, ephem.Jupiter(self.home))[2],
            "jupiter_az": "%.2f°" % numpy.degrees(ephem.Jupiter(self.home).az),
            "jupiter_alt": "%.2f°" % numpy.degrees(ephem.Jupiter(self.home).alt),
        }

    def Saturn(self) -> dict:
        """"""
        return {
            "saturn_rise": "%s" % parser(self.home, ephem.Saturn(self.home))[0],
            "saturn_transit": "%s" % parser(self.home, ephem.Saturn(self.home))[1],
            "saturn_set": "%s" % parser(self.home, ephem.Saturn(self.home))[2],
            "saturn_az": "%.2f°" % numpy.degrees(ephem.Saturn(self.home).az),
            "saturn_alt": "%.2f°" % numpy.degrees(ephem.Saturn(self.home).alt),
        }

    def Uranus(self) -> dict:
        """"""
        return {
            "uranus_rise": "%s" % parser(self.home, ephem.Uranus(self.home))[0],
            "uranus_transit": "%s" % parser(self.home, ephem.Uranus(self.home))[1],
            "uranus_set": "%s" % parser(self.home, ephem.Uranus(self.home))[2],
            "uranus_az": "%.2f°" % numpy.degrees(ephem.Uranus(self.home).az),
            "uranus_alt": "%.2f°" % numpy.degrees(ephem.Uranus(self.home).alt),
        }

    def Neptune(self) -> dict:
        """"""
        return {
            "neptune_rise": "%s" % parser(self.home, ephem.Neptune(self.home))[0],
            "neptune_transit": "%s" % parser(self.home, ephem.Neptune(self.home))[1],
            "neptune_set": "%s" % parser(self.home, ephem.Neptune(self.home))[2],
            "neptune_az": "%.2f°" % numpy.degrees(ephem.Neptune(self.home).az),
            "neptune_alt": "%.2f°" % numpy.degrees(ephem.Neptune(self.home).alt),
        }


class NorthPolar(object):
    """
    Show infomation about other planets in our solar system
    """

    def __init__(self, lat: str, lon: str, elevation: float) -> None:
        self.home = ephem.Observer()
        self.home.lat = lat
        self.home.lon = lon
        self.home.elevation = elevation
        # update time
        self.home.date = datetime.datetime.utcnow()

    def get_polaris_data(self) -> list:
        """
        Get the polar data for a given observer
        Args : None
        Returns : list
        """
        polaris_data = []

        """
        lst = 100.46 + 0.985647 * d + lon + 15 * ut [based on http://www.stargazing.net/kepler/altaz.html]
        d - the days from J2000 (1200 hrs UT on Jan 1st 2000 AD), including the fraction of a day
        lon - your longitude in decimal degrees, East positive
        ut - the universal time in decimal hours
        """

        j2000 = ephem.Date("2000/01/01 12:00:00")
        d = self.home.date - j2000

        lon = numpy.rad2deg(float(repr(self.home.lon)))

        utstr = self.home.date.datetime().strftime("%H:%M:%S")
        ut = (
            float(utstr.split(":")[0])
            + float(utstr.split(":")[1]) / 60
            + float(utstr.split(":")[2]) / 3600
        )

        lst = 100.46 + 0.985647 * d + lon + 15 * ut
        lst = lst - int(lst / 360) * 360

        polaris = ephem.readdb("Polaris,f|M|F7,2:31:48.704,89:15:50.72,2.02,2000")
        polaris.compute()
        polaris_ra_deg = numpy.rad2deg(float(repr(polaris.ra)))

        # Polaris Hour Angle = LST - RA Polaris [expressed in degrees or 15*(h+m/60+s/3600)]
        pha = lst - polaris_ra_deg

        # normalize
        if pha < 0:
            pha += 360
        elif pha > 360:
            pha -= 360
        # append polaris hour angle
        polaris_data.append(pha)

        # append polaris next transit
        try:
            polaris_data.append(
                ephem.localtime(self.home.next_transit(polaris)).strftime("%H:%M:%S")
            )
        except (ephem.NeverUpError, ephem.AlwaysUpError):
            polaris_data.append("-")
        # append polaris alt
        polaris_data.append(polaris.alt)

        return polaris_data

    def get_polar_info(self) -> dict:
        """"""
        polaris_data = self.get_polaris_data(self.home)
        return {
            "polaris_hour_angle": polaris_data[0],
            "polaris_next_transit": "%s" % polaris_data[1],
            "polaris_alt": "%.2f°" % numpy.degrees(polaris_data[2]),
        }
