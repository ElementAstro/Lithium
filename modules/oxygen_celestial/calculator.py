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

from astropy import units as u
from astropy.coordinates import AltAz, EarthLocation, SkyCoord
from astropy.time import Time
from astropy.coordinates import FK5

location = None

def convert_j2000_to_jnow(star_object : SkyCoord) -> SkyCoord:
    """
        Convert coordinates in J2000 format to JNow format
    """
    tmp = star_object.fk5
    return tmp.transform_to(FK5(equinox=f'J{datetime.datetime.today().year}'))

def convert_radec_to_azalt(star_object : SkyCoord) -> SkyCoord:
    """
        Convert coordinates in RA/DEC format to AZ/ALT format
    """
    return star_object.transform_to(AltAz(
        obstime=Time(datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'))-8*u.hour,
        location=location)
        )

def set_location(lat : str | float , lon : str | float , ele : float) -> EarthLocation:
    """
        Set the location
    """
    global location
    location = EarthLocation(lat=float(lat)*u.deg, lon=float(lon)*u.deg, height=ele*u.m)