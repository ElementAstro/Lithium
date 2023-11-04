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

from functools import cached_property
from typing import Generator, List, Tuple, Optional, Union
import json
import numpy as np
import re
import sqlite3

from typing import Optional

from astropy.coordinates import SkyCoord

class InvalidCoordinates(Exception):
    """
    Raised when coordinates are not valid.

    Maybe you're passing an object without registered coordinates (typically an `Unknown` object)
    to some function; or you input coordinates as text in a wrong format: to be recognized
    the input text must be in the format `HH:MM:SS.ss +/-DD:MM:SS.s`.
    """
    def __init__(self, text: Optional[str] = None):
        if text is not None:
            super().__init__(text)
        else:  # pragma: no cover
            super().__init__('Coordinates not recognized.')


class ObjectNotFound(Exception):
    """
    Raised when a valid object identifier isn't found in the database.

    The identifier is recognized to be part of one of the supported catalogs,
    but the object isn't in the database (or doesn't exist at all).

    For example, `pyongc.Object('NGC7000A')` is valid, but it doesn't exist.
    """
    def __init__(self, name: Optional[str] = None):
        if name is not None:
            super().__init__(f'Object named {name} not found in the database.')
        else:  # pragma: no cover
            super().__init__('Object not found in the database.')


class UnknownIdentifier(Exception):
    """
    Raised when input text can't be recognized as a valid object identifier.

    You're asking for an identifier using the wrong format, or using an identifier
    which refers to a catalog not supported by PyOngc.
    """
    def __init__(self, text: Optional[str] = None):
        if text is not None:
            super().__init__(f'The name "{text}" is not recognized.')
        else:  # pragma: no cover
            super().__init__('Unrecognized object name.')

DBPATH = "data.db"

PATTERNS = {'NGC|IC': r'^((?:NGC|IC)\s?)(\d{1,4})\s?((NED)(\d{1,2})|[A-Z]{1,2})?$',
            'Messier': r'^(M\s?)(\d{1,3})$',
            'Barnard': r'^(B\s?)(\d{1,3})$',
            'Caldwell': r'^(C\s?)(\d{1,3})$',
            'Collinder': r'^(CL\s?)(\d{1,3})$',
            'ESO': r'^(ESO\s?)(\d{1,3})-(\d{1,3})$',
            'Harvard': r'^(H\s?)(\d{1,2})$',
            'Hickson': r'^(HCG\s?)(\d{1,3})$',
            'LBN': r'^(LBN\s?)(\d{1,3})$',
            'Melotte': r'^(MEL\s?)(\d{1,3})$',
            'MWSC': r'^(MWSC\s?)(\d{1,4})$',
            'PGC': r'^((?:PGC|LEDA)\s?)(\d{1,6})$',
            'UGC': r'^(UGC\s?)(\d{1,5})$',
            }


class Object(object):
    """Describes a Deep Sky Object from ONGC database.

    Each object of this class has the following read only properties:

    * constellation: The constellation where the object is located.
    * coords: Object coordinates in HMS and DMS as numpy array or None.
    * dec: Object Declination in a easy to read format as string.
    * name: The main identifier of the object.
    * ra: Object Right Ascension in a easy to read format as string.
    * type: Object type.

    The class also provides the following methods:

    * __init__: Object constructor.
    * __str__: Returns a basic description of the object.
    * xephemFormat: Returns object data in Xephem format.

    """

    def __init__(self, name: str, returndup: bool = False):
        """Object constructor.

        Args:
            name: Object identifier (ex.: 'NGC1', 'M15').
            returndup: If set to True, don't resolve Dup objects. Default is False.

        Raises:
            TypeError: If the object identifier is not a string.
            pyongc.ObjectNotFound: If the object identifier is not found in the database.
        """
        # Make sure user passed a string as parameter
        if not isinstance(name, str):
            raise TypeError('Wrong type as parameter. A string type was expected.')

        catalog, objectname = _recognize_name(name.upper())

        cols = ('objects.name, objects.type, objTypes.typedesc, ra, dec, const')
        tables = ('objects JOIN objTypes ON objects.type = objTypes.type '
                  'JOIN objIdentifiers ON objects.name = objIdentifiers.name')
        if catalog == 'Messier':
            params = f'messier="{objectname}"'
        else:
            params = f'objIdentifiers.identifier="{objectname}"'
        objectData = _queryFetchOne(cols, tables, params)

        if objectData is None:
            raise ObjectNotFound(objectname)

        # If object is a duplicate then return the main object
        if objectData[2] == "Dup" and not returndup:
            if objectData[26] != "":
                objectname = f'NGC{objectData[26]}'
            else:
                objectname = f'IC{objectData[27]}'
            params = f'objIdentifiers.identifier="{objectname}"'
            objectData = _queryFetchOne(cols, tables, params)

        # Assign object properties
        self._name = objectData[0]
        self._type = objectData[1]
        self._ra = objectData[3]
        self._dec = objectData[4]
        self._const = objectData[5]
       
    def __str__(self) -> str:
        """
            Returns a basic description of the object.
            >>> s = Object("ngc1")
            >>> print(s)
            NGC0001, Galaxy in Peg
        """
        return f'{self._name}, {self._type} in {self._const}'

    @cached_property
    def constellation(self) -> str:
        """
            The constellation where the object is located.
            >>> s = Object("ngc1")
            >>> s.constellation
            'Peg'
            Returns:
                Name of the constellation in IAU 3-letter form.
        """
        return self._const

    @cached_property
    def coords(self) -> Optional[np.ndarray]:
        """
            Returns object coordinates in HMS and DMS as numpy array or None.
            Returns:
                A numpy array of shape (2, 3) with R.A. and Declination
                values expressed in HMS and DMS.
        """
        if self._ra is None or self._dec is None:
            return None

        ra = np.empty(3)
        ra[0] = np.trunc(np.rad2deg(self._ra) / 15)
        ms = ((np.rad2deg(self._ra) / 15) - ra[0]) * 60
        ra[1] = np.trunc(ms)
        ra[2] = (ms - ra[1]) * 60

        dec = np.empty(3)
        dec[0] = np.trunc(np.rad2deg(np.abs(self._dec)))
        ms = (np.rad2deg(np.abs(self._dec)) - dec[0]) * 60
        dec[1] = np.trunc(ms)
        dec[2] = (ms - dec[1]) * 60
        dec[0] = dec[0] * -1 if np.signbit(self._dec) else dec[0]
        return np.array([ra, dec, ])

    @cached_property
    def dec(self) -> str:
        """
            Object Declination in a easy to read format as string.
            If you need the raw data to use in calculations use `coords` or `rad_coords` properties.
            Returns:
                string: `'+/-DD:MM:SS.s'` or `'N/A'` if the object has no coordinates.

        """
        if self.coords is not None:
            return '{:+03.0f}:{:02.0f}:{:04.1f}'.format(*self.coords[1])
        else:
            return 'N/A'
        
    @cached_property
    def name(self) -> str:
        """
            The main identifier of the object.
            Returns:
                The main identifier of the object, as listed in ONGC database
                or its addendum.
        """
        return self._name

    @cached_property
    def ra(self) -> str:
        """
            Object Right Ascension in a easy to read format as string.
            If you need the raw data to use in calculations use `coords` or `rad_coords` properties.
            Returns:
                string: `'HH:MM:SS.ss'` or `'N/A'` if the object has no coordinates.
        """
        if self.coords is not None:
            return '{:02.0f}:{:02.0f}:{:05.2f}'.format(*self.coords[0])
        else:
            return 'N/A'

    @cached_property
    def rad_coords(self) -> Optional[np.ndarray]:
        """
            Returns object coordinates in radians as numpy array or None.
            Returns:
                A numpy array of shape (2,) with R.A. and Declination
                values expressed in radians.
        """
        if self._ra is None or self._dec is None:
            return None

        return np.array([self._ra, self._dec, ])

    @cached_property
    def type(self) -> str:
        """
            Object type.
            Returns:
                string: Object type
        """
        return self._type

    @cached_property
    def astropy(self) -> SkyCoord:
        """
            Convert to a SkyCoord Instance
            SkyCoord('00h42.5m', '+41d12m')
        """
        return SkyCoord(f"{int(self.coords[0][0])}h{int(self.coords[0][1])}m{int(self.coords[0][2])}s",
                        f"{int(self.coords[1][0])}d{int(self.coords[1][1])}m{int(self.coords[1][2])}s"
                        )

    def to_json(self) -> str:
        """Returns object data in JSON format."""
        return json.dumps(self, cls=ObjectEncoder)

class ObjectEncoder(json.JSONEncoder):
    """
        A custom json.dumps serializer for Object class.
    """

    def default(self, obj: Object) -> dict:
        """
            A custom json.dumps serializer for Object class.
            Args:
                obj: the Object object to encode.
        """
        if isinstance(obj, Object):
            obj_description = {'name': obj.name,
                               'type': obj.type,
                               'coordinates': {'RA': obj.ra,
                                               'DEC': obj.dec,
                                               },
                               'constellation': obj.constellation
                               }
            return obj_description
        else:
            return super().default(obj)


def _distance(coords1: np.ndarray, coords2: np.ndarray) -> Tuple[float, float, float]:
    """Calculate distance between two points in the sky.

    With p1 = '01:00:00 +15:30:00' and p2 = '01:30:00 +10:30:00':

            >>> import numpy as np
            >>> p1 = np.array([0.26179939, 0.27052603])
            >>> p2 = np.array([0.39269908, 0.18325957])
            >>> _distance(p1, p2)
            (8.852139937970884, 7.499999776570824, -4.999999851047216)

    Args:
        coords1: R.A. and Dec expressed in radians of the first point as
            numpy array with shape(2,)
        coords2: R.A. and Dec expressed in radians of the second point as
            numpy array with shape(2,)

    Returns:
        `(angular separation, difference in A.R, difference in Dec)`

        This function will return three float values, which are the apparent total
        angular separation between the two objects, the difference in Right Ascension and the
        difference in Declination.

        All values are expressed in degrees.

    """
    a1 = coords1[0]
    a2 = coords2[0]
    d1 = coords1[1]
    d2 = coords2[1]

    # separation = np.arccos(np.sin(d1)*np.sin(d2) + np.cos(d1)*np.cos(d2)*np.cos(a1-a2))
    # Better precision formula
    # see http://aa.quae.nl/en/reken/afstanden.html
    separation = 2*np.arcsin(np.sqrt(np.sin((d2-d1)/2)**2 +
                                     np.cos(d1)*np.cos(d2)*np.sin((a2-a1)/2)**2))

    return np.degrees(separation), np.degrees(a2-a1), np.degrees(d2-d1)


def _limiting_coords(coords: np.ndarray, radius: int) -> str:
    """Write query filters for limiting search to specific area of the sky.

    This is a quick method to exclude objects farther than a specified distance
    from the starting point, but it's not meant to be precise.

            >>> from pyongc.ongc import Object, _limiting_coords
            >>> start = Object('ngc1').coords
            >>> _limiting_coords(start, 2)
            ' AND (ra <= 0.06660176425610362 OR ra >= 6.279973901355917) AND \
(dec BETWEEN 0.44869069854374555 AND 0.5185038686235187)'

    Args:
        coords: R.A. and Dec of the starting point in the sky.

            It can be expressed as a numpy array of H:M:S/D:M:S

            `array([[HH., MM., SS.ss],[DD., MM., SS.ss]])`

            or as numpy array of radians

            `array([RA, Dec])`
        radius: radius of the search in degrees

    Returns:
        Parameters to be added to query

    """
    if coords.shape == (2, 3):
        rad_coords = np.empty(2)
        rad_coords[0] = np.radians(np.sum(coords[0] * [15, 1/4, 1/240]))
        if np.signbit(coords[1][0]):
            rad_coords[1] = np.radians(np.sum(coords[1] * [1, -1/60, -1/3600]))
        else:
            rad_coords[1] = np.radians(np.sum(coords[1] * [1, 1/60, 1/3600]))
    else:
        rad_coords = coords

    radius_rad = np.radians(radius)
    ra_lower_limit = rad_coords[0] - radius_rad
    ra_upper_limit = rad_coords[0] + radius_rad
    if ra_lower_limit < 0:
        ra_lower_limit += 2 * np.pi
        params = f' AND (ra <= {ra_upper_limit} OR ra >= {ra_lower_limit})'
    elif ra_upper_limit > 2 * np.pi:
        ra_upper_limit -= 2 * np.pi
        params = f' AND (ra <= {ra_upper_limit} OR ra >= {ra_lower_limit})'
    else:
        params = f' AND (ra BETWEEN {ra_lower_limit} AND {ra_upper_limit})'

    dec_lower_limit = rad_coords[1] - radius_rad
    if dec_lower_limit < -1/2 * np.pi:
        dec_lower_limit = -1/2 * np.pi
    dec_upper_limit = rad_coords[1] + radius_rad
    if dec_upper_limit > 1/2 * np.pi:
        dec_upper_limit = 1/2 * np.pi

    params += f' AND (dec BETWEEN {dec_lower_limit} AND {dec_upper_limit})'
    return params


def _queryFetchOne(cols: str, tables: str, params: str) -> tuple:
    """Search one row in database.

    Be sure to use a WHERE clause which is very specific, otherwise the query
    will return the first row that matches.

            >>> from pyongc.ongc import _queryFetchOne
            >>> cols = 'type'
            >>> tables = 'objects'
            >>> params = 'name="NGC0001"'
            >>> _queryFetchOne(cols, tables, params)
            ('G',)

    Args:
        cols: the `SELECT` field of the query
        tables: the `FROM` field of the query
        params: the `WHERE` field of the query

    Returns:
        Selected row data from database

    """
    try:
        db = sqlite3.connect(f'file:{DBPATH}?mode=ro', uri=True)
    except sqlite3.Error:
        raise OSError(f'There was a problem accessing database file at {DBPATH}')

    try:
        cursor = db.cursor()
        cursor.execute(f'SELECT {cols} '
                       f'FROM {tables} '
                       f'WHERE {params}'
                       )
        objectData = cursor.fetchone()
    except Exception as err:  # pragma: no cover
        raise err
    finally:
        db.close()

    return objectData


def _queryFetchMany(cols: str, tables: str, params: str,
                    order: str = '') -> Generator[tuple, None, None]:
    """Search many rows in database.

            >>> from pyongc.ongc import _queryFetchMany
            >>> cols = 'name'
            >>> tables = 'objects'
            >>> params = 'type="G"'
            >>> _queryFetchMany(cols, tables, params) #doctest: +ELLIPSIS
            <generator object _queryFetchMany at 0x...>

    Args:
        cols: the `SELECT` field of the query
        tables: the `FROM` field of the query
        params: the `WHERE` field of the query
        order: the `ORDER` clause of the query

    Yields:
        Selected row data from database

    """
    try:
        db = sqlite3.connect(f'file:{DBPATH}?mode=ro', uri=True)
    except sqlite3.Error:
        raise OSError(f'There was a problem accessing database file at {DBPATH}')

    try:
        cursor = db.cursor()

        cursor.execute(f'SELECT {cols} '
                       f'FROM {tables} '
                       f'WHERE {params}'
                       f'{" ORDER BY " + order if order != "" else ""}'
                       )
        while True:
            objectList = cursor.fetchmany()
            if objectList == []:
                break
            yield objectList[0]
    except Exception as err:  # pragma: no cover
        raise err
    finally:
        db.close()


def _recognize_name(text: str) -> Tuple[str, str]:
    """Recognize catalog and object id.

            >>> from pyongc.ongc import _recognize_name
            >>> _recognize_name('NGC1')
            ('NGC|IC', 'NGC0001')

    Args:
        text: the object name in input. Must be uppercase.

    Returns:
        `('catalog name', 'object name')`

    Raises:
        UnknownIdentifier: If the text cannot be recognized as a valid object name.

    """
    for cat, pat in PATTERNS.items():
        name_parts = re.match(pat, text)
        if name_parts is not None:
            if cat == 'NGC|IC' and name_parts.group(3) is not None:
                # User searches for a NGC/IC sub-object
                if name_parts.group(4) is not None:
                    # User searches for a NED suffixed component
                    objectname = f'{name_parts.group(1).strip()}' \
                                 f'{name_parts.group(2):0>4}' \
                                 f' {name_parts.group(4)}' \
                                 f'{name_parts.group(5):0>2}'
                else:
                    # User searches for a letter suffixed component
                    objectname = f'{name_parts.group(1).strip()}' \
                                 f'{name_parts.group(2):0>4}' \
                                 f'{name_parts.group(3).strip()}'
            elif cat in ('NGC|IC', 'MWSC'):
                objectname = f'{name_parts.group(1).strip()}{name_parts.group(2):0>4}'
            elif cat == 'ESO':
                objectname = f'{name_parts.group(1).strip()}{name_parts.group(2):0>3}-' \
                             f'{name_parts.group(3):0>3}'
            elif cat == 'Harvard':
                objectname = f'{name_parts.group(1).strip()}{name_parts.group(2):0>2}'
            elif cat == 'Messier':
                # We need to return only the numeric part of the name
                objectname = ('101' if name_parts.group(2) == '102'
                              else f'{name_parts.group(2):0>3}'
                              )
            elif cat == 'UGC':
                objectname = f'{name_parts.group(1).strip()}{name_parts.group(2):0>5}'
            elif cat == 'PGC':
                # Fixed catalog name to recognize also LEDA prefix
                objectname = f'{cat}{name_parts.group(2):0>6}'
            else:
                objectname = f'{name_parts.group(1).strip()}{name_parts.group(2):0>3}'
            return cat, objectname
    raise UnknownIdentifier(text)


def _str_to_coords(text: str) -> np.ndarray:
    """Recognize coordinates as string and return them as radians.

    Args:
        text (string): a string expressing coordinates in the form `HH:MM:SS.ss +/-DD:MM:SS.s`

    Returns:
        `array([RA, Dec])`

        A numpy array of shape (2,) with coordinates expressed in radians.

    Raises:
        InvalidCoordinates: If the text cannot be recognized as valid coordinates.

    """
    pattern = re.compile(r'^(?:(\d{1,2}):(\d{1,2}):(\d{1,2}(?:\.\d{1,2})?))\s'
                         r'(?:([+-]\d{1,2}):(\d{1,2}):(\d{1,2}(?:\.\d{1,2})?))$')
    result = pattern.match(text)

    if result:
        hms = np.array([float(x) for x in result.groups()[0:3]])
        ra = np.radians(np.sum(hms * [15, 1/4, 1/240]))
        dms = np.array([float(x) for x in result.groups()[3:6]])
        if np.signbit(dms[0]):
            dec = np.radians(np.sum(dms * [1, -1/60, -1/3600]))
        else:
            dec = np.radians(np.sum(dms * [1, 1/60, 1/3600]))

        return np.array([ra, dec])
    else:
        raise InvalidCoordinates(f'This text cannot be recognized as coordinates: {text}')


def get(name: str) -> Optional[Object]:
    """Search and return an object from the database.

    If an object name isn't recognized, it will return None.

    Args:
        name: the name of the object

    Returns:
        Object or None.

    """
    try:
        obj = Object(name)
    except (ObjectNotFound, UnknownIdentifier):
        return None
    return obj


def getNeighbors(obj: Union[Object, str], separation: Union[int, float],
                 catalog: str = "all") -> List[Tuple[Object, float]]:
    """
        Find all neighbors of an object within a user selected range.

        It requires an object as the starting point of the search (either a string containing
        the name or a Object type) and a search radius expressed in arcmins.

        The maximum allowed search radius is 600 arcmin (10 degrees).

        It returns a list of of tuples with the Object objects found in range and its distance,
        or an empty list if no object is found:

                >>> from pyongc.ongc import Object, getNeighbors
                >>> s1 = Object("ngc521")
                >>> getNeighbors(s1, 15) #doctest: +ELLIPSIS
                [(<pyongc.ongc.Object object at 0x...>, 0.13726168561780452), \
    (<pyongc.ongc.Object object at 0x...>, 0.24140243942744602)]

                >>> from pyongc.ongc import getNeighbors
                >>> getNeighbors("ngc521", 1)
                []

        The optional "catalog" parameter can be used to filter the search to only NGC or IC objects:

                >>> from pyongc.ongc import getNeighbors
                >>> getNeighbors("ngc521", 15, catalog="NGC") #doctest: +ELLIPSIS
                [(<pyongc.ongc.Object object at 0x...>, 0.24140243942744602)]

        Args:
            object: a Object object or a string which identifies the object
            separation: maximum distance from the object expressed in arcmin
            catalog: filter for "NGC" or "IC" objects - default is all

        Returns:
            A list of tuples with each element composed by the Object object found and
            its distance from the starting point, ordered by distance.

        Raises:
            ValueError: If the search radius exceeds 10 degrees.
            InvalidCoordinates: If the starting object hasn't got registered cordinates.

    """
    if not isinstance(obj, Object):
        obj = Object(obj)
    if separation > 600:
        raise ValueError('The maximum search radius allowed is 10 degrees.')
    if obj.rad_coords is None:
        raise InvalidCoordinates('Starting object hasn\'t got registered coordinates.')

    cols = 'objects.name'
    tables = 'objects'
    params = f'type != "Dup" AND name !="{obj.name}"'
    if catalog.upper() in ["NGC", "IC"]:
        params += f' AND name LIKE "{catalog.upper()}%"'

    params += _limiting_coords(obj.rad_coords, np.ceil(separation / 60))

    neighbors = []
    for item in _queryFetchMany(cols, tables, params):
        possibleNeighbor = Object(item[0])
        distance = getSeparation(obj, possibleNeighbor)[0]
        if distance <= (separation / 60):
            neighbors.append((possibleNeighbor, distance))

    return sorted(neighbors, key=lambda neighbor: neighbor[1])


def getSeparation(obj1: Union[Object, str], obj2: Union[Object, str],
                  style: str = "raw") -> Union[Tuple[float, float, float], str]:
    """Finds the apparent angular separation between two objects.

    This function will compute the apparent angular separation between two objects,
    either identified with their names as strings or directly as Object type.

    By default it returns a tuple containing the angular separation and the differences in A.R.
    and Declination expressed in degrees:

            >>> from pyongc.ongc import Object, getSeparation
            >>> s1 = Object("ngc1")
            >>> s2 = Object("ngc2")
            >>> getSeparation(s1, s2)
            (0.03008927371519897, 0.005291666666666788, -0.02972222222221896)

            >>> from pyongc.ongc import getSeparation
            >>> getSeparation("ngc1", "ngc2")
            (0.03008927371519897, 0.005291666666666788, -0.02972222222221896)

    With the optional parameter `style` set to `text`, it returns a formatted string:

            >>> from pyongc.ongc import getSeparation
            >>> getSeparation("ngc1", "ngc2", style="text")
            '0° 1m 48.32s'

    If one of the objects is not found in the database it returns an ObjectNotFound exception:

            >>> from pyongc.ongc import getSeparation
            >>> getSeparation("ngc1a", "ngc2")
            Traceback (most recent call last):
            ...
            pyongc.exceptions.ObjectNotFound: Object named NGC0001A not found in the database.

    Args:
        obj1: first Object object or string identifier
        obj2: second Object object or string identifier
        style: use "text" to return a string with degrees, minutes and seconds

    Returns:
        By default the return value is a tuple with values expressed in degrees

        (angular separation, difference in A.R, difference in Dec)

        With the `style` parameter set to `text` we get a more readable output in the form
        `'DD° MMm SS.SSs'`

    """
    if not isinstance(obj1, Object):
        obj1 = Object(obj1)
    if not isinstance(obj2, Object):
        obj2 = Object(obj2)
    if obj1.rad_coords is None or obj2.rad_coords is None:
        raise InvalidCoordinates('One object hasn\'t got registered coordinates.')

    separation = _distance(obj1.rad_coords, obj2.rad_coords)

    if style == "text":
        d = int(separation[0])
        md = abs(separation[0] - d) * 60
        m = int(md)
        s = (md - m) * 60
        return f'{d:d}° {m:d}m {s:.2f}s'
    else:
        return separation


def listObjects(**kwargs) -> List[Object]:
    """
        Query the database for DSObjects with specific parameters.

        This function returns a list of all DSObjects that match user defined parameters.
        If no argument is passed to the function, it returns all the objects from the database:

                >>> from pyongc.ongc import listObjects
                >>> objectList = listObjects()
                >>> len(objectList)
                13992

        Filters are combined with "AND" in the query; only one value for filter is allowed:

                >>> from pyongc.ongc import listObjects
                >>> objectList = listObjects(catalog="NGC", constellation=["Boo", ])
                >>> len(objectList)
                281

        Duplicated objects are not resolved to main objects:

                >>> from pyongc.ongc import listObjects
                >>> objectList = listObjects(type=["Dup", ])
                >>> print(objectList[0])
                IC0011, Duplicated record in Cas

        The maxSize filter will include objects with no size recorded in database:

                >>> from pyongc.ongc import listObjects
                >>> objectList = listObjects(maxsize=0)
                >>> len(objectList)
                1967

        Args:
            catalog (string, optional): filter for catalog. [NGC|IC|M]
            type (list, optional): filter for object type. See OpenNGC types list.
            constellation (list, optional): filter for constellation
                (three letter latin form - e.g. "And")
            minsize (float, optional): filter for objects with MajAx >= minSize(arcmin)
            maxsize (float, optional): filter for objects with MajAx < maxSize(arcmin)
                OR MajAx not available
            uptobmag (float, optional): filter for objects with B-Mag brighter than value
            uptovmag (float, optional): filter for objects with V-Mag brighter than value
            minra (float, optional): filter for objects with RA degrees greater than value
            maxra (float, optional): filter for objects with RA degrees lower than value
            mindec (float, optional): filter for objects above specified Dec degrees
            maxdec (float, optional): filter for objects below specified Dec degrees
            cname (string, optional): filter for objects with common name like input value
            withname (bool, optional): filter for objects with common names

        Returns:
            A list of ongc.Object objects.

        Raises:
            ValueError: If a filter name other than those expected is inserted.
            ValueError: If an unrecognized catalog name is entered. Only [NGC|IC|M] are permitted.

    """
    available_filters = ['catalog',
                         'type',
                         'constellation',
                         'minsize',
                         'maxsize',
                         'uptobmag',
                         'uptovmag',
                         'minra',
                         'maxra',
                         'mindec',
                         'maxdec',
                         'cname',
                         'withname']
    cols = 'objects.name'
    tables = 'objects'

    if kwargs == {}:
        params = '1'
        return [Object(str(item[0]), True) for item in _queryFetchMany(cols, tables, params)]
    for element in kwargs:
        if element not in available_filters:
            raise ValueError("Wrong filter name.")

    paramslist = []
    order = ''
    if "catalog" in kwargs:
        if kwargs["catalog"].upper() == "NGC" or kwargs["catalog"].upper() == "IC":
            paramslist.append(f'name LIKE "{kwargs["catalog"].upper()}%"')
        elif kwargs["catalog"].upper() == "M":
            paramslist.append('messier != ""')
            order = 'messier ASC'
        else:
            raise ValueError('Wrong value for catalog filter. [NGC|IC|M]')
    if "type" in kwargs:
        types = [f'"{t}"' for t in kwargs["type"]]
        paramslist.append(f'type IN ({",".join(types)})')

    if "constellation" in kwargs:
        constellations = [f'"{c.capitalize()}"' for c in kwargs["constellation"]]
        paramslist.append(f'const IN ({",".join(constellations)})')

    if "minra" in kwargs and "maxra" in kwargs:
        if kwargs["maxra"] > kwargs["minra"]:
            paramslist.append(f'ra BETWEEN '
                              f'{np.radians(kwargs["minra"])} '
                              f'AND {np.radians(kwargs["maxra"])}'
                              )
        else:
            paramslist.append(f'ra >= {np.radians(kwargs["minra"])} '
                              f'OR ra <= {np.radians(kwargs["maxra"])}'
                              )
    elif "minra" in kwargs:
        paramslist.append(f'ra >= {np.radians(kwargs["minra"])}')
    elif "maxra" in kwargs:
        paramslist.append(f'ra <= {np.radians(kwargs["maxra"])}')

    if "mindec" in kwargs and "maxdec" in kwargs:
        if kwargs["maxdec"] > kwargs["mindec"]:
            paramslist.append(f'dec BETWEEN '
                              f'{np.radians(kwargs["mindec"])} '
                              f'AND {np.radians(kwargs["maxdec"])}'
                              )
    elif "mindec" in kwargs:
        paramslist.append(f'dec >= {np.radians(kwargs["mindec"])}')
    elif "maxdec" in kwargs:
        paramslist.append(f'dec <= {np.radians(kwargs["maxdec"])}')

    params = " AND ".join(paramslist)
    return [Object(item[0], True) for item in _queryFetchMany(cols, tables, params, order)]


def nearby(coords_string: str, separation: float = 60,
           catalog: str = "all") -> List[Tuple[Object, float]]:
    """
        Search for objects around given coordinates.

        Returns all objects around a point expressed by the coords parameter and within a search
        radius expressed by the separation parameter.
        Coordinates must be Right Ascension and Declination expressed as a string in the
        form "HH:MM:SS.ss +/-DD:MM:SS.s".

        The maximum allowed search radius is 600 arcmin (10 degrees) and default value is 60.

        It returns a list of of tuples with the Object objects found in range and its distance,
        or an empty list if no object is found:

                >>> from pyongc.ongc import nearby
                >>> nearby('11:08:44 -00:09:01.3') #doctest: +ELLIPSIS +FLOAT_CMP
                [(<pyongc.ongc.Object object at 0x...>, 0.1799936868460791), \
    (<pyongc.ongc.Object object at 0x...>, 0.7398295985600021), \
    (<pyongc.ongc.Object object at 0x...>, 0.9810037613087355)]

        The optional "catalog" parameter can be used to filter the search to only NGC or IC objects:

                >>> from pyongc.ongc import nearby
                >>> nearby('11:08:44 -00:09:01.3', separation=60, catalog='NGC') #doctest: +ELLIPSIS \
    +FLOAT_CMP
                [(<pyongc.ongc.Object object at 0x...>, 0.7398295985600021)]

        Args:
            coords: R.A. and Dec of search center
            separation: search radius expressed in arcmin - default 60
            catalog: filter for "NGC" or "IC" objects - default is all

        Returns:
            `[(Object, separation),]`

            A list of tuples with the Object object found and its distance from the starting point,
            ordered by distance.

        Raises:
            ValueError: If the search radius exceeds 10 degrees.

    """
    if separation > 600:
        raise ValueError('The maximum search radius allowed is 10 degrees.')

    coords = _str_to_coords(coords_string)

    cols = 'objects.name'
    tables = 'objects'
    params = 'type != "Dup"'
    if catalog.upper() in ["NGC", "IC"]:
        params += f' AND name LIKE "{catalog.upper()}%"'

    params += _limiting_coords(coords, np.ceil(separation / 60))

    neighbors = []
    for item in _queryFetchMany(cols, tables, params):
        possibleNeighbor = Object(item[0])
        distance = _distance(coords, possibleNeighbor.rad_coords)[0]
        if distance <= (separation / 60):
            neighbors.append((possibleNeighbor, distance))

    return sorted(neighbors, key=lambda neighbor: neighbor[1])
