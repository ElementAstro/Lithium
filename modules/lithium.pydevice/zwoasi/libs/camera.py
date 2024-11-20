r"""Wrapper for ASICamera2.h

Do not modify this file.
"""

__docformat__ = "restructuredtext"

# Begin preamble for Python

import ctypes
import sys
from ctypes import *  # noqa: F401, F403

_int_types = (ctypes.c_int16, ctypes.c_int32)
if hasattr(ctypes, "c_int64"):
    # Some builds of ctypes apparently do not have ctypes.c_int64
    # defined; it's a pretty good bet that these builds do not
    # have 64-bit pointers.
    _int_types += (ctypes.c_int64,)
for t in _int_types:
    if ctypes.sizeof(t) == ctypes.sizeof(ctypes.c_size_t):
        c_ptrdiff_t = t
del t
del _int_types



class UserString:
    def __init__(self, seq):
        if isinstance(seq, bytes):
            self.data = seq
        elif isinstance(seq, UserString):
            self.data = seq.data[:]
        else:
            self.data = str(seq).encode()

    def __bytes__(self):
        return self.data

    def __str__(self):
        return self.data.decode()

    def __repr__(self):
        return repr(self.data)

    def __int__(self):
        return int(self.data.decode())

    def __long__(self):
        return int(self.data.decode())

    def __float__(self):
        return float(self.data.decode())

    def __complex__(self):
        return complex(self.data.decode())

    def __hash__(self):
        return hash(self.data)

    def __le__(self, string):
        if isinstance(string, UserString):
            return self.data <= string.data
        else:
            return self.data <= string

    def __lt__(self, string):
        if isinstance(string, UserString):
            return self.data < string.data
        else:
            return self.data < string

    def __ge__(self, string):
        if isinstance(string, UserString):
            return self.data >= string.data
        else:
            return self.data >= string

    def __gt__(self, string):
        if isinstance(string, UserString):
            return self.data > string.data
        else:
            return self.data > string

    def __eq__(self, string):
        if isinstance(string, UserString):
            return self.data == string.data
        else:
            return self.data == string

    def __ne__(self, string):
        if isinstance(string, UserString):
            return self.data != string.data
        else:
            return self.data != string

    def __contains__(self, char):
        return char in self.data

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.__class__(self.data[index])

    def __getslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        return self.__class__(self.data[start:end])

    def __add__(self, other):
        if isinstance(other, UserString):
            return self.__class__(self.data + other.data)
        elif isinstance(other, bytes):
            return self.__class__(self.data + other)
        else:
            return self.__class__(self.data + str(other).encode())

    def __radd__(self, other):
        if isinstance(other, bytes):
            return self.__class__(other + self.data)
        else:
            return self.__class__(str(other).encode() + self.data)

    def __mul__(self, n):
        return self.__class__(self.data * n)

    __rmul__ = __mul__

    def __mod__(self, args):
        return self.__class__(self.data % args)

    # the following methods are defined in alphabetical order:
    def capitalize(self):
        return self.__class__(self.data.capitalize())

    def center(self, width, *args):
        return self.__class__(self.data.center(width, *args))

    def count(self, sub, start=0, end=sys.maxsize):
        return self.data.count(sub, start, end)

    def decode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.decode(encoding, errors))
            else:
                return self.__class__(self.data.decode(encoding))
        else:
            return self.__class__(self.data.decode())

    def encode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.encode(encoding, errors))
            else:
                return self.__class__(self.data.encode(encoding))
        else:
            return self.__class__(self.data.encode())

    def endswith(self, suffix, start=0, end=sys.maxsize):
        return self.data.endswith(suffix, start, end)

    def expandtabs(self, tabsize=8):
        return self.__class__(self.data.expandtabs(tabsize))

    def find(self, sub, start=0, end=sys.maxsize):
        return self.data.find(sub, start, end)

    def index(self, sub, start=0, end=sys.maxsize):
        return self.data.index(sub, start, end)

    def isalpha(self):
        return self.data.isalpha()

    def isalnum(self):
        return self.data.isalnum()

    def isdecimal(self):
        return self.data.isdecimal()

    def isdigit(self):
        return self.data.isdigit()

    def islower(self):
        return self.data.islower()

    def isnumeric(self):
        return self.data.isnumeric()

    def isspace(self):
        return self.data.isspace()

    def istitle(self):
        return self.data.istitle()

    def isupper(self):
        return self.data.isupper()

    def join(self, seq):
        return self.data.join(seq)

    def ljust(self, width, *args):
        return self.__class__(self.data.ljust(width, *args))

    def lower(self):
        return self.__class__(self.data.lower())

    def lstrip(self, chars=None):
        return self.__class__(self.data.lstrip(chars))

    def partition(self, sep):
        return self.data.partition(sep)

    def replace(self, old, new, maxsplit=-1):
        return self.__class__(self.data.replace(old, new, maxsplit))

    def rfind(self, sub, start=0, end=sys.maxsize):
        return self.data.rfind(sub, start, end)

    def rindex(self, sub, start=0, end=sys.maxsize):
        return self.data.rindex(sub, start, end)

    def rjust(self, width, *args):
        return self.__class__(self.data.rjust(width, *args))

    def rpartition(self, sep):
        return self.data.rpartition(sep)

    def rstrip(self, chars=None):
        return self.__class__(self.data.rstrip(chars))

    def split(self, sep=None, maxsplit=-1):
        return self.data.split(sep, maxsplit)

    def rsplit(self, sep=None, maxsplit=-1):
        return self.data.rsplit(sep, maxsplit)

    def splitlines(self, keepends=0):
        return self.data.splitlines(keepends)

    def startswith(self, prefix, start=0, end=sys.maxsize):
        return self.data.startswith(prefix, start, end)

    def strip(self, chars=None):
        return self.__class__(self.data.strip(chars))

    def swapcase(self):
        return self.__class__(self.data.swapcase())

    def title(self):
        return self.__class__(self.data.title())

    def translate(self, *args):
        return self.__class__(self.data.translate(*args))

    def upper(self):
        return self.__class__(self.data.upper())

    def zfill(self, width):
        return self.__class__(self.data.zfill(width))


class MutableString(UserString):
    """mutable string objects

    Python strings are immutable objects.  This has the advantage, that
    strings may be used as dictionary keys.  If this property isn't needed
    and you insist on changing string values in place instead, you may cheat
    and use MutableString.

    But the purpose of this class is an educational one: to prevent
    people from inventing their own mutable string class derived
    from UserString and than forget thereby to remove (override) the
    __hash__ method inherited from UserString.  This would lead to
    errors that would be very hard to track down.

    A faster and better solution is to rewrite your program using lists."""

    def __init__(self, string=""):
        self.data = string

    def __hash__(self):
        raise TypeError("unhashable type (it is mutable)")

    def __setitem__(self, index, sub):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + sub + self.data[index + 1 :]

    def __delitem__(self, index):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + self.data[index + 1 :]

    def __setslice__(self, start, end, sub):
        start = max(start, 0)
        end = max(end, 0)
        if isinstance(sub, UserString):
            self.data = self.data[:start] + sub.data + self.data[end:]
        elif isinstance(sub, bytes):
            self.data = self.data[:start] + sub + self.data[end:]
        else:
            self.data = self.data[:start] + str(sub).encode() + self.data[end:]

    def __delslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        self.data = self.data[:start] + self.data[end:]

    def immutable(self):
        return UserString(self.data)

    def __iadd__(self, other):
        if isinstance(other, UserString):
            self.data += other.data
        elif isinstance(other, bytes):
            self.data += other
        else:
            self.data += str(other).encode()
        return self

    def __imul__(self, n):
        self.data *= n
        return self


class String(MutableString, ctypes.Union):

    _fields_ = [("raw", ctypes.POINTER(ctypes.c_char)), ("data", ctypes.c_char_p)]

    def __init__(self, obj=b""):
        if isinstance(obj, (bytes, UserString)):
            self.data = bytes(obj)
        else:
            self.raw = obj

    def __len__(self):
        return self.data and len(self.data) or 0

    def from_param(cls, obj):
        # Convert None or 0
        if obj is None or obj == 0:
            return cls(ctypes.POINTER(ctypes.c_char)())

        # Convert from String
        elif isinstance(obj, String):
            return obj

        # Convert from bytes
        elif isinstance(obj, bytes):
            return cls(obj)

        # Convert from str
        elif isinstance(obj, str):
            return cls(obj.encode())

        # Convert from c_char_p
        elif isinstance(obj, ctypes.c_char_p):
            return obj

        # Convert from POINTER(ctypes.c_char)
        elif isinstance(obj, ctypes.POINTER(ctypes.c_char)):
            return obj

        # Convert from raw pointer
        elif isinstance(obj, int):
            return cls(ctypes.cast(obj, ctypes.POINTER(ctypes.c_char)))

        # Convert from ctypes.c_char array
        elif isinstance(obj, ctypes.c_char * len(obj)):
            return obj

        # Convert from object
        else:
            return String.from_param(obj._as_parameter_)

    from_param = classmethod(from_param)


def ReturnString(obj, func=None, arguments=None):
    return String.from_param(obj)


# As of ctypes 1.0, ctypes does not support custom error-checking
# functions on callbacks, nor does it support custom datatypes on
# callbacks, so we must ensure that all callbacks return
# primitive datatypes.
#
# Non-primitive return values wrapped with UNCHECKED won't be
# typechecked, and will be converted to ctypes.c_void_p.
def UNCHECKED(type):
    if hasattr(type, "_type_") and isinstance(type._type_, str) and type._type_ != "P":
        return type
    else:
        return ctypes.c_void_p


# ctypes doesn't have direct support for variadic functions, so we have to write
# our own wrapper class
class _variadic_function(object):
    def __init__(self, func, restype, argtypes, errcheck):
        self.func = func
        self.func.restype = restype
        self.argtypes = argtypes
        if errcheck:
            self.func.errcheck = errcheck

    def _as_parameter_(self):
        # So we can pass this variadic function as a function pointer
        return self.func

    def __call__(self, *args):
        fixed_args = []
        i = 0
        for argtype in self.argtypes:
            # Typecheck what we can
            fixed_args.append(argtype.from_param(args[i]))
            i += 1
        return self.func(*fixed_args + list(args[i:]))


def ord_if_char(value):
    """
    Simple helper used for casts to simple builtin types:  if the argument is a
    string type, it will be converted to it's ordinal value.

    This function will raise an exception if the argument is string with more
    than one characters.
    """
    return ord(value) if (isinstance(value, bytes) or isinstance(value, str)) else value

# End preamble

_libs = {}
_libdirs = []

# Begin loader

"""
Load libraries - appropriately for all our supported platforms
"""
# ----------------------------------------------------------------------------
# Copyright (c) 2008 David James
# Copyright (c) 2006-2008 Alex Holkner
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of pyglet nor the names of its
#    contributors may be used to endorse or promote products
#    derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

import ctypes
import ctypes.util
import glob
import os.path
import platform
import re
import sys


def _environ_path(name):
    """Split an environment variable into a path-like list elements"""
    if name in os.environ:
        return os.environ[name].split(":")
    return []


class LibraryLoader:
    """
    A base class For loading of libraries ;-)
    Subclasses load libraries for specific platforms.
    """

    # library names formatted specifically for platforms
    name_formats = ["%s"]

    class Lookup:
        """Looking up calling conventions for a platform"""

        mode = ctypes.DEFAULT_MODE

        def __init__(self, path):
            super(LibraryLoader.Lookup, self).__init__()
            self.access = dict(cdecl=ctypes.CDLL(path, self.mode))

        def get(self, name, calling_convention="cdecl"):
            """Return the given name according to the selected calling convention"""
            if calling_convention not in self.access:
                raise LookupError(
                    "Unknown calling convention '{}' for function '{}'".format(
                        calling_convention, name
                    )
                )
            return getattr(self.access[calling_convention], name)

        def has(self, name, calling_convention="cdecl"):
            """Return True if this given calling convention finds the given 'name'"""
            if calling_convention not in self.access:
                return False
            return hasattr(self.access[calling_convention], name)

        def __getattr__(self, name):
            return getattr(self.access["cdecl"], name)

    def __init__(self):
        self.other_dirs = []

    def __call__(self, libname):
        """Given the name of a library, load it."""
        paths = self.getpaths(libname)

        for path in paths:
            # noinspection PyBroadException
            try:
                return self.Lookup(path)
            except Exception:  # pylint: disable=broad-except
                pass

        raise ImportError("Could not load %s." % libname)

    def getpaths(self, libname):
        """Return a list of paths where the library might be found."""
        if os.path.isabs(libname):
            yield libname
        else:
            # search through a prioritized series of locations for the library

            # we first search any specific directories identified by user
            for dir_i in self.other_dirs:
                for fmt in self.name_formats:
                    # dir_i should be absolute already
                    yield os.path.join(dir_i, fmt % libname)

            # check if this code is even stored in a physical file
            try:
                this_file = __file__
            except NameError:
                this_file = None

            # then we search the directory where the generated python interface is stored
            if this_file is not None:
                for fmt in self.name_formats:
                    yield os.path.abspath(os.path.join(os.path.dirname(__file__), fmt % libname))

            # now, use the ctypes tools to try to find the library
            for fmt in self.name_formats:
                path = ctypes.util.find_library(fmt % libname)
                if path:
                    yield path

            # then we search all paths identified as platform-specific lib paths
            for path in self.getplatformpaths(libname):
                yield path

            # Finally, we'll try the users current working directory
            for fmt in self.name_formats:
                yield os.path.abspath(os.path.join(os.path.curdir, fmt % libname))

    def getplatformpaths(self, _libname):  # pylint: disable=no-self-use
        """Return all the library paths available in this platform"""
        return []


# Darwin (Mac OS X)


class DarwinLibraryLoader(LibraryLoader):
    """Library loader for MacOS"""

    name_formats = [
        "lib%s.dylib",
        "lib%s.so",
        "lib%s.bundle",
        "%s.dylib",
        "%s.so",
        "%s.bundle",
        "%s",
    ]

    class Lookup(LibraryLoader.Lookup):
        """
        Looking up library files for this platform (Darwin aka MacOS)
        """

        # Darwin requires dlopen to be called with mode RTLD_GLOBAL instead
        # of the default RTLD_LOCAL.  Without this, you end up with
        # libraries not being loadable, resulting in "Symbol not found"
        # errors
        mode = ctypes.RTLD_GLOBAL

    def getplatformpaths(self, libname):
        if os.path.pathsep in libname:
            names = [libname]
        else:
            names = [fmt % libname for fmt in self.name_formats]

        for directory in self.getdirs(libname):
            for name in names:
                yield os.path.join(directory, name)

    @staticmethod
    def getdirs(libname):
        """Implements the dylib search as specified in Apple documentation:

        http://developer.apple.com/documentation/DeveloperTools/Conceptual/
            DynamicLibraries/Articles/DynamicLibraryUsageGuidelines.html

        Before commencing the standard search, the method first checks
        the bundle's ``Frameworks`` directory if the application is running
        within a bundle (OS X .app).
        """

        dyld_fallback_library_path = _environ_path("DYLD_FALLBACK_LIBRARY_PATH")
        if not dyld_fallback_library_path:
            dyld_fallback_library_path = [
                os.path.expanduser("~/lib"),
                "/usr/local/lib",
                "/usr/lib",
            ]

        dirs = []

        if "/" in libname:
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
        else:
            dirs.extend(_environ_path("LD_LIBRARY_PATH"))
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
            dirs.extend(_environ_path("LD_RUN_PATH"))

        if hasattr(sys, "frozen") and getattr(sys, "frozen") == "macosx_app":
            dirs.append(os.path.join(os.environ["RESOURCEPATH"], "..", "Frameworks"))

        dirs.extend(dyld_fallback_library_path)

        return dirs


# Posix


class PosixLibraryLoader(LibraryLoader):
    """Library loader for POSIX-like systems (including Linux)"""

    _ld_so_cache = None

    _include = re.compile(r"^\s*include\s+(?P<pattern>.*)")

    name_formats = ["lib%s.so", "%s.so", "%s"]

    class _Directories(dict):
        """Deal with directories"""

        def __init__(self):
            dict.__init__(self)
            self.order = 0

        def add(self, directory):
            """Add a directory to our current set of directories"""
            if len(directory) > 1:
                directory = directory.rstrip(os.path.sep)
            # only adds and updates order if exists and not already in set
            if not os.path.exists(directory):
                return
            order = self.setdefault(directory, self.order)
            if order == self.order:
                self.order += 1

        def extend(self, directories):
            """Add a list of directories to our set"""
            for a_dir in directories:
                self.add(a_dir)

        def ordered(self):
            """Sort the list of directories"""
            return (i[0] for i in sorted(self.items(), key=lambda d: d[1]))

    def _get_ld_so_conf_dirs(self, conf, dirs):
        """
        Recursive function to help parse all ld.so.conf files, including proper
        handling of the `include` directive.
        """

        try:
            with open(conf) as fileobj:
                for dirname in fileobj:
                    dirname = dirname.strip()
                    if not dirname:
                        continue

                    match = self._include.match(dirname)
                    if not match:
                        dirs.add(dirname)
                    else:
                        for dir2 in glob.glob(match.group("pattern")):
                            self._get_ld_so_conf_dirs(dir2, dirs)
        except IOError:
            pass

    def _create_ld_so_cache(self):
        # Recreate search path followed by ld.so.  This is going to be
        # slow to build, and incorrect (ld.so uses ld.so.cache, which may
        # not be up-to-date).  Used only as fallback for distros without
        # /sbin/ldconfig.
        #
        # We assume the DT_RPATH and DT_RUNPATH binary sections are omitted.

        directories = self._Directories()
        for name in (
            "LD_LIBRARY_PATH",
            "SHLIB_PATH",  # HP-UX
            "LIBPATH",  # OS/2, AIX
            "LIBRARY_PATH",  # BE/OS
        ):
            if name in os.environ:
                directories.extend(os.environ[name].split(os.pathsep))

        self._get_ld_so_conf_dirs("/etc/ld.so.conf", directories)

        bitage = platform.architecture()[0]

        unix_lib_dirs_list = []
        if bitage.startswith("64"):
            # prefer 64 bit if that is our arch
            unix_lib_dirs_list += ["/lib64", "/usr/lib64"]

        # must include standard libs, since those paths are also used by 64 bit
        # installs
        unix_lib_dirs_list += ["/lib", "/usr/lib"]
        if sys.platform.startswith("linux"):
            # Try and support multiarch work in Ubuntu
            # https://wiki.ubuntu.com/MultiarchSpec
            if bitage.startswith("32"):
                # Assume Intel/AMD x86 compat
                unix_lib_dirs_list += ["/lib/i386-linux-gnu", "/usr/lib/i386-linux-gnu"]
            elif bitage.startswith("64"):
                # Assume Intel/AMD x86 compatible
                unix_lib_dirs_list += [
                    "/lib/x86_64-linux-gnu",
                    "/usr/lib/x86_64-linux-gnu",
                ]
            else:
                # guess...
                unix_lib_dirs_list += glob.glob("/lib/*linux-gnu")
        directories.extend(unix_lib_dirs_list)

        cache = {}
        lib_re = re.compile(r"lib(.*)\.s[ol]")
        # ext_re = re.compile(r"\.s[ol]$")
        for our_dir in directories.ordered():
            try:
                for path in glob.glob("%s/*.s[ol]*" % our_dir):
                    file = os.path.basename(path)

                    # Index by filename
                    cache_i = cache.setdefault(file, set())
                    cache_i.add(path)

                    # Index by library name
                    match = lib_re.match(file)
                    if match:
                        library = match.group(1)
                        cache_i = cache.setdefault(library, set())
                        cache_i.add(path)
            except OSError:
                pass

        self._ld_so_cache = cache

    def getplatformpaths(self, libname):
        if self._ld_so_cache is None:
            self._create_ld_so_cache()

        result = self._ld_so_cache.get(libname, set())
        for i in result:
            # we iterate through all found paths for library, since we may have
            # actually found multiple architectures or other library types that
            # may not load
            yield i


# Windows


class WindowsLibraryLoader(LibraryLoader):
    """Library loader for Microsoft Windows"""

    name_formats = ["%s.dll", "lib%s.dll", "%slib.dll", "%s"]

    class Lookup(LibraryLoader.Lookup):
        """Lookup class for Windows libraries..."""

        def __init__(self, path):
            super(WindowsLibraryLoader.Lookup, self).__init__(path)
            self.access["stdcall"] = ctypes.windll.LoadLibrary(path)


# Platform switching

# If your value of sys.platform does not appear in this dict, please contact
# the Ctypesgen maintainers.

loaderclass = {
    "darwin": DarwinLibraryLoader,
    "cygwin": WindowsLibraryLoader,
    "win32": WindowsLibraryLoader,
    "msys": WindowsLibraryLoader,
}

load_library = loaderclass.get(sys.platform, PosixLibraryLoader)()


def add_library_search_dirs(other_dirs):
    """
    Add libraries to search paths.
    If library paths are relative, convert them to absolute with respect to this
    file's directory
    """
    for path in other_dirs:
        if not os.path.isabs(path):
            path = os.path.abspath(path)
        load_library.other_dirs.append(path)


del loaderclass

# End loader

add_library_search_dirs([])

# No libraries

# No modules

enum_ASI_BAYER_PATTERN = c_int# /usr/include/libasi/ASICamera2.h: 49

ASI_BAYER_RG = 0# /usr/include/libasi/ASICamera2.h: 49

ASI_BAYER_BG = (ASI_BAYER_RG + 1)# /usr/include/libasi/ASICamera2.h: 49

ASI_BAYER_GR = (ASI_BAYER_BG + 1)# /usr/include/libasi/ASICamera2.h: 49

ASI_BAYER_GB = (ASI_BAYER_GR + 1)# /usr/include/libasi/ASICamera2.h: 49

ASI_BAYER_PATTERN = enum_ASI_BAYER_PATTERN# /usr/include/libasi/ASICamera2.h: 49

enum_ASI_IMG_TYPE = c_int# /usr/include/libasi/ASICamera2.h: 58

ASI_IMG_RAW8 = 0# /usr/include/libasi/ASICamera2.h: 58

ASI_IMG_RGB24 = (ASI_IMG_RAW8 + 1)# /usr/include/libasi/ASICamera2.h: 58

ASI_IMG_RAW16 = (ASI_IMG_RGB24 + 1)# /usr/include/libasi/ASICamera2.h: 58

ASI_IMG_Y8 = (ASI_IMG_RAW16 + 1)# /usr/include/libasi/ASICamera2.h: 58

ASI_IMG_END = (-1)# /usr/include/libasi/ASICamera2.h: 58

ASI_IMG_TYPE = enum_ASI_IMG_TYPE# /usr/include/libasi/ASICamera2.h: 58

enum_ASI_GUIDE_DIRECTION = c_int# /usr/include/libasi/ASICamera2.h: 65

ASI_GUIDE_NORTH = 0# /usr/include/libasi/ASICamera2.h: 65

ASI_GUIDE_SOUTH = (ASI_GUIDE_NORTH + 1)# /usr/include/libasi/ASICamera2.h: 65

ASI_GUIDE_EAST = (ASI_GUIDE_SOUTH + 1)# /usr/include/libasi/ASICamera2.h: 65

ASI_GUIDE_WEST = (ASI_GUIDE_EAST + 1)# /usr/include/libasi/ASICamera2.h: 65

ASI_GUIDE_DIRECTION = enum_ASI_GUIDE_DIRECTION# /usr/include/libasi/ASICamera2.h: 65

enum_ASI_FLIP_STATUS = c_int# /usr/include/libasi/ASICamera2.h: 75

ASI_FLIP_NONE = 0# /usr/include/libasi/ASICamera2.h: 75

ASI_FLIP_HORIZ = (ASI_FLIP_NONE + 1)# /usr/include/libasi/ASICamera2.h: 75

ASI_FLIP_VERT = (ASI_FLIP_HORIZ + 1)# /usr/include/libasi/ASICamera2.h: 75

ASI_FLIP_BOTH = (ASI_FLIP_VERT + 1)# /usr/include/libasi/ASICamera2.h: 75

ASI_FLIP_STATUS = enum_ASI_FLIP_STATUS# /usr/include/libasi/ASICamera2.h: 75

enum_ASI_CAMERA_MODE = c_int# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_NORMAL = 0# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_TRIG_SOFT_EDGE = (ASI_MODE_NORMAL + 1)# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_TRIG_RISE_EDGE = (ASI_MODE_TRIG_SOFT_EDGE + 1)# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_TRIG_FALL_EDGE = (ASI_MODE_TRIG_RISE_EDGE + 1)# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_TRIG_SOFT_LEVEL = (ASI_MODE_TRIG_FALL_EDGE + 1)# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_TRIG_HIGH_LEVEL = (ASI_MODE_TRIG_SOFT_LEVEL + 1)# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_TRIG_LOW_LEVEL = (ASI_MODE_TRIG_HIGH_LEVEL + 1)# /usr/include/libasi/ASICamera2.h: 86

ASI_MODE_END = (-1)# /usr/include/libasi/ASICamera2.h: 86

ASI_CAMERA_MODE = enum_ASI_CAMERA_MODE# /usr/include/libasi/ASICamera2.h: 86

enum_ASI_TRIG_OUTPUT = c_int# /usr/include/libasi/ASICamera2.h: 92

ASI_TRIG_OUTPUT_PINA = 0# /usr/include/libasi/ASICamera2.h: 92

ASI_TRIG_OUTPUT_PINB = (ASI_TRIG_OUTPUT_PINA + 1)# /usr/include/libasi/ASICamera2.h: 92

ASI_TRIG_OUTPUT_NONE = (-1)# /usr/include/libasi/ASICamera2.h: 92

ASI_TRIG_OUTPUT_PIN = enum_ASI_TRIG_OUTPUT# /usr/include/libasi/ASICamera2.h: 92

enum_ASI_ERROR_CODE = c_int# /usr/include/libasi/ASICamera2.h: 119

ASI_SUCCESS = 0# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_INDEX = (ASI_SUCCESS + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_ID = (ASI_ERROR_INVALID_INDEX + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_CONTROL_TYPE = (ASI_ERROR_INVALID_ID + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_CAMERA_CLOSED = (ASI_ERROR_INVALID_CONTROL_TYPE + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_CAMERA_REMOVED = (ASI_ERROR_CAMERA_CLOSED + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_PATH = (ASI_ERROR_CAMERA_REMOVED + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_FILEFORMAT = (ASI_ERROR_INVALID_PATH + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_SIZE = (ASI_ERROR_INVALID_FILEFORMAT + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_IMGTYPE = (ASI_ERROR_INVALID_SIZE + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_OUTOF_BOUNDARY = (ASI_ERROR_INVALID_IMGTYPE + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_TIMEOUT = (ASI_ERROR_OUTOF_BOUNDARY + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_SEQUENCE = (ASI_ERROR_TIMEOUT + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_BUFFER_TOO_SMALL = (ASI_ERROR_INVALID_SEQUENCE + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_VIDEO_MODE_ACTIVE = (ASI_ERROR_BUFFER_TOO_SMALL + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_EXPOSURE_IN_PROGRESS = (ASI_ERROR_VIDEO_MODE_ACTIVE + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_GENERAL_ERROR = (ASI_ERROR_EXPOSURE_IN_PROGRESS + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_INVALID_MODE = (ASI_ERROR_GENERAL_ERROR + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_GPS_NOT_SUPPORTED = (ASI_ERROR_INVALID_MODE + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_GPS_VER_ERR = (ASI_ERROR_GPS_NOT_SUPPORTED + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_GPS_FPGA_ERR = (ASI_ERROR_GPS_VER_ERR + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_GPS_PARAM_OUT_OF_RANGE = (ASI_ERROR_GPS_FPGA_ERR + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_GPS_DATA_INVALID = (ASI_ERROR_GPS_PARAM_OUT_OF_RANGE + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_END = (ASI_ERROR_GPS_DATA_INVALID + 1)# /usr/include/libasi/ASICamera2.h: 119

ASI_ERROR_CODE = enum_ASI_ERROR_CODE# /usr/include/libasi/ASICamera2.h: 119

enum_ASI_BOOL = c_int# /usr/include/libasi/ASICamera2.h: 124

ASI_FALSE = 0# /usr/include/libasi/ASICamera2.h: 124

ASI_TRUE = (ASI_FALSE + 1)# /usr/include/libasi/ASICamera2.h: 124

ASI_BOOL = enum_ASI_BOOL# /usr/include/libasi/ASICamera2.h: 124

# /usr/include/libasi/ASICamera2.h: 150
class struct__ASI_CAMERA_INFO(Structure):
    pass

struct__ASI_CAMERA_INFO.__slots__ = [
    'Name',
    'CameraID',
    'MaxHeight',
    'MaxWidth',
    'IsColorCam',
    'BayerPattern',
    'SupportedBins',
    'SupportedVideoFormat',
    'PixelSize',
    'MechanicalShutter',
    'ST4Port',
    'IsCoolerCam',
    'IsUSB3Host',
    'IsUSB3Camera',
    'ElecPerADU',
    'BitDepth',
    'IsTriggerCam',
    'Unused',
]
struct__ASI_CAMERA_INFO._fields_ = [
    ('Name', c_char * int(64)),
    ('CameraID', c_int),
    ('MaxHeight', c_long),
    ('MaxWidth', c_long),
    ('IsColorCam', ASI_BOOL),
    ('BayerPattern', ASI_BAYER_PATTERN),
    ('SupportedBins', c_int * int(16)),
    ('SupportedVideoFormat', ASI_IMG_TYPE * int(8)),
    ('PixelSize', c_double),
    ('MechanicalShutter', ASI_BOOL),
    ('ST4Port', ASI_BOOL),
    ('IsCoolerCam', ASI_BOOL),
    ('IsUSB3Host', ASI_BOOL),
    ('IsUSB3Camera', ASI_BOOL),
    ('ElecPerADU', c_float),
    ('BitDepth', c_int),
    ('IsTriggerCam', ASI_BOOL),
    ('Unused', c_char * int(16)),
]

ASI_CAMERA_INFO = struct__ASI_CAMERA_INFO# /usr/include/libasi/ASICamera2.h: 150

enum_ASI_CONTROL_TYPE = c_int# /usr/include/libasi/ASICamera2.h: 185

ASI_GAIN = 0# /usr/include/libasi/ASICamera2.h: 185

ASI_EXPOSURE = (ASI_GAIN + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_GAMMA = (ASI_EXPOSURE + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_WB_R = (ASI_GAMMA + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_WB_B = (ASI_WB_R + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_OFFSET = (ASI_WB_B + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_BANDWIDTHOVERLOAD = (ASI_OFFSET + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_OVERCLOCK = (ASI_BANDWIDTHOVERLOAD + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_TEMPERATURE = (ASI_OVERCLOCK + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_FLIP = (ASI_TEMPERATURE + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_AUTO_MAX_GAIN = (ASI_FLIP + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_AUTO_MAX_EXP = (ASI_AUTO_MAX_GAIN + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_AUTO_TARGET_BRIGHTNESS = (ASI_AUTO_MAX_EXP + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_HARDWARE_BIN = (ASI_AUTO_TARGET_BRIGHTNESS + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_HIGH_SPEED_MODE = (ASI_HARDWARE_BIN + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_COOLER_POWER_PERC = (ASI_HIGH_SPEED_MODE + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_TARGET_TEMP = (ASI_COOLER_POWER_PERC + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_COOLER_ON = (ASI_TARGET_TEMP + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_MONO_BIN = (ASI_COOLER_ON + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_FAN_ON = (ASI_MONO_BIN + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_PATTERN_ADJUST = (ASI_FAN_ON + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_ANTI_DEW_HEATER = (ASI_PATTERN_ADJUST + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_FAN_ADJUST = (ASI_ANTI_DEW_HEATER + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_PWRLED_BRIGNT = (ASI_FAN_ADJUST + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_USBHUB_RESET = (ASI_PWRLED_BRIGNT + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_GPS_SUPPORT = (ASI_USBHUB_RESET + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_GPS_START_LINE = (ASI_GPS_SUPPORT + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_GPS_END_LINE = (ASI_GPS_START_LINE + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_ROLLING_INTERVAL = (ASI_GPS_END_LINE + 1)# /usr/include/libasi/ASICamera2.h: 185

ASI_CONTROL_TYPE = enum_ASI_CONTROL_TYPE# /usr/include/libasi/ASICamera2.h: 185

# /usr/include/libasi/ASICamera2.h: 198
class struct__ASI_CONTROL_CAPS(Structure):
    pass

struct__ASI_CONTROL_CAPS.__slots__ = [
    'Name',
    'Description',
    'MaxValue',
    'MinValue',
    'DefaultValue',
    'IsAutoSupported',
    'IsWritable',
    'ControlType',
    'Unused',
]
struct__ASI_CONTROL_CAPS._fields_ = [
    ('Name', c_char * int(64)),
    ('Description', c_char * int(128)),
    ('MaxValue', c_long),
    ('MinValue', c_long),
    ('DefaultValue', c_long),
    ('IsAutoSupported', ASI_BOOL),
    ('IsWritable', ASI_BOOL),
    ('ControlType', ASI_CONTROL_TYPE),
    ('Unused', c_char * int(32)),
]

ASI_CONTROL_CAPS = struct__ASI_CONTROL_CAPS# /usr/include/libasi/ASICamera2.h: 198

enum_ASI_EXPOSURE_STATUS = c_int# /usr/include/libasi/ASICamera2.h: 206

ASI_EXP_IDLE = 0# /usr/include/libasi/ASICamera2.h: 206

ASI_EXP_WORKING = (ASI_EXP_IDLE + 1)# /usr/include/libasi/ASICamera2.h: 206

ASI_EXP_SUCCESS = (ASI_EXP_WORKING + 1)# /usr/include/libasi/ASICamera2.h: 206

ASI_EXP_FAILED = (ASI_EXP_SUCCESS + 1)# /usr/include/libasi/ASICamera2.h: 206

ASI_EXPOSURE_STATUS = enum_ASI_EXPOSURE_STATUS# /usr/include/libasi/ASICamera2.h: 206

# /usr/include/libasi/ASICamera2.h: 210
class struct__ASI_ID(Structure):
    pass

struct__ASI_ID.__slots__ = [
    'id',
]
struct__ASI_ID._fields_ = [
    ('id', c_ubyte * int(8)),
]

ASI_ID = struct__ASI_ID# /usr/include/libasi/ASICamera2.h: 210

ASI_SN = ASI_ID# /usr/include/libasi/ASICamera2.h: 212

# /usr/include/libasi/ASICamera2.h: 216
class struct__ASI_SUPPORTED_MODE(Structure):
    pass

struct__ASI_SUPPORTED_MODE.__slots__ = [
    'SupportedCameraMode',
]
struct__ASI_SUPPORTED_MODE._fields_ = [
    ('SupportedCameraMode', ASI_CAMERA_MODE * int(16)),
]

ASI_SUPPORTED_MODE = struct__ASI_SUPPORTED_MODE# /usr/include/libasi/ASICamera2.h: 216

# /usr/include/libasi/ASICamera2.h: 228
class struct__ASI_DATE_TIME(Structure):
    pass

struct__ASI_DATE_TIME.__slots__ = [
    'Year',
    'Month',
    'Day',
    'Hour',
    'Minute',
    'Second',
    'Msecond',
    'Usecond',
    'Unused',
]
struct__ASI_DATE_TIME._fields_ = [
    ('Year', c_int),
    ('Month', c_int),
    ('Day', c_int),
    ('Hour', c_int),
    ('Minute', c_int),
    ('Second', c_int),
    ('Msecond', c_int),
    ('Usecond', c_int),
    ('Unused', c_char * int(64)),
]

ASI_DATE_TIME = struct__ASI_DATE_TIME# /usr/include/libasi/ASICamera2.h: 228

# /usr/include/libasi/ASICamera2.h: 237
class struct__ASI_GPS_DATA(Structure):
    pass

struct__ASI_GPS_DATA.__slots__ = [
    'Datetime',
    'Latitude',
    'Longitude',
    'Altitude',
    'SatelliteNum',
    'Unused',
]
struct__ASI_GPS_DATA._fields_ = [
    ('Datetime', ASI_DATE_TIME),
    ('Latitude', c_double),
    ('Longitude', c_double),
    ('Altitude', c_int),
    ('SatelliteNum', c_int),
    ('Unused', c_char * int(64)),
]

ASI_GPS_DATA = struct__ASI_GPS_DATA# /usr/include/libasi/ASICamera2.h: 237

# /usr/include/libasi/ASICamera2.h: 262
for _lib in _libs.values():
    if not _lib.has("ASIGetNumOfConnectedCameras", "cdecl"):
        continue
    ASIGetNumOfConnectedCameras = _lib.get("ASIGetNumOfConnectedCameras", "cdecl")
    ASIGetNumOfConnectedCameras.argtypes = []
    ASIGetNumOfConnectedCameras.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 275
for _lib in _libs.values():
    if not _lib.has("ASIGetProductIDs", "cdecl"):
        continue
    ASIGetProductIDs = _lib.get("ASIGetProductIDs", "cdecl")
    ASIGetProductIDs.argtypes = [POINTER(c_int)]
    ASIGetProductIDs.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 287
for _lib in _libs.values():
    if not _lib.has("ASICameraCheck", "cdecl"):
        continue
    ASICameraCheck = _lib.get("ASICameraCheck", "cdecl")
    ASICameraCheck.argtypes = [c_int, c_int]
    ASICameraCheck.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 312
for _lib in _libs.values():
    if not _lib.has("ASIGetCameraProperty", "cdecl"):
        continue
    ASIGetCameraProperty = _lib.get("ASIGetCameraProperty", "cdecl")
    ASIGetCameraProperty.argtypes = [POINTER(ASI_CAMERA_INFO), c_int]
    ASIGetCameraProperty.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 330
for _lib in _libs.values():
    if not _lib.has("ASIGetCameraPropertyByID", "cdecl"):
        continue
    ASIGetCameraPropertyByID = _lib.get("ASIGetCameraPropertyByID", "cdecl")
    ASIGetCameraPropertyByID.argtypes = [c_int, POINTER(ASI_CAMERA_INFO)]
    ASIGetCameraPropertyByID.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 347
for _lib in _libs.values():
    if not _lib.has("ASIOpenCamera", "cdecl"):
        continue
    ASIOpenCamera = _lib.get("ASIOpenCamera", "cdecl")
    ASIOpenCamera.argtypes = [c_int]
    ASIOpenCamera.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 362
for _lib in _libs.values():
    if not _lib.has("ASIInitCamera", "cdecl"):
        continue
    ASIInitCamera = _lib.get("ASIInitCamera", "cdecl")
    ASIInitCamera.argtypes = [c_int]
    ASIInitCamera.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 377
for _lib in _libs.values():
    if not _lib.has("ASICloseCamera", "cdecl"):
        continue
    ASICloseCamera = _lib.get("ASICloseCamera", "cdecl")
    ASICloseCamera.argtypes = [c_int]
    ASICloseCamera.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 397
for _lib in _libs.values():
    if not _lib.has("ASIGetNumOfControls", "cdecl"):
        continue
    ASIGetNumOfControls = _lib.get("ASIGetNumOfControls", "cdecl")
    ASIGetNumOfControls.argtypes = [c_int, POINTER(c_int)]
    ASIGetNumOfControls.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 418
for _lib in _libs.values():
    if not _lib.has("ASIGetControlCaps", "cdecl"):
        continue
    ASIGetControlCaps = _lib.get("ASIGetControlCaps", "cdecl")
    ASIGetControlCaps.argtypes = [c_int, c_int, POINTER(ASI_CONTROL_CAPS)]
    ASIGetControlCaps.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 438
for _lib in _libs.values():
    if not _lib.has("ASIGetControlValue", "cdecl"):
        continue
    ASIGetControlValue = _lib.get("ASIGetControlValue", "cdecl")
    ASIGetControlValue.argtypes = [c_int, c_int, POINTER(c_long), POINTER(c_int)]
    ASIGetControlValue.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 459
for _lib in _libs.values():
    if not _lib.has("ASISetControlValue", "cdecl"):
        continue
    ASISetControlValue = _lib.get("ASISetControlValue", "cdecl")
    ASISetControlValue.argtypes = [c_int, c_int, c_long, c_int]
    ASISetControlValue.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 484
for _lib in _libs.values():
    if not _lib.has("ASISetROIFormat", "cdecl"):
        continue
    ASISetROIFormat = _lib.get("ASISetROIFormat", "cdecl")
    ASISetROIFormat.argtypes = [c_int, c_int, c_int, c_int, c_int]
    ASISetROIFormat.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 504
for _lib in _libs.values():
    if not _lib.has("ASIGetROIFormat", "cdecl"):
        continue
    ASIGetROIFormat = _lib.get("ASIGetROIFormat", "cdecl")
    ASIGetROIFormat.argtypes = [c_int, POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_int)]
    ASIGetROIFormat.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 527
for _lib in _libs.values():
    if not _lib.has("ASISetStartPos", "cdecl"):
        continue
    ASISetStartPos = _lib.get("ASISetStartPos", "cdecl")
    ASISetStartPos.argtypes = [c_int, c_int, c_int]
    ASISetStartPos.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 544
for _lib in _libs.values():
    if not _lib.has("ASIGetStartPos", "cdecl"):
        continue
    ASIGetStartPos = _lib.get("ASIGetStartPos", "cdecl")
    ASIGetStartPos.argtypes = [c_int, POINTER(c_int), POINTER(c_int)]
    ASIGetStartPos.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 562
for _lib in _libs.values():
    if not _lib.has("ASIGetDroppedFrames", "cdecl"):
        continue
    ASIGetDroppedFrames = _lib.get("ASIGetDroppedFrames", "cdecl")
    ASIGetDroppedFrames.argtypes = [c_int, POINTER(c_int)]
    ASIGetDroppedFrames.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 588
for _lib in _libs.values():
    if not _lib.has("ASIEnableDarkSubtract", "cdecl"):
        continue
    ASIEnableDarkSubtract = _lib.get("ASIEnableDarkSubtract", "cdecl")
    ASIEnableDarkSubtract.argtypes = [c_int, String]
    ASIEnableDarkSubtract.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 605
for _lib in _libs.values():
    if not _lib.has("ASIDisableDarkSubtract", "cdecl"):
        continue
    ASIDisableDarkSubtract = _lib.get("ASIDisableDarkSubtract", "cdecl")
    ASIDisableDarkSubtract.argtypes = [c_int]
    ASIDisableDarkSubtract.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 622
for _lib in _libs.values():
    if not _lib.has("ASIStartVideoCapture", "cdecl"):
        continue
    ASIStartVideoCapture = _lib.get("ASIStartVideoCapture", "cdecl")
    ASIStartVideoCapture.argtypes = [c_int]
    ASIStartVideoCapture.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 638
for _lib in _libs.values():
    if not _lib.has("ASIStopVideoCapture", "cdecl"):
        continue
    ASIStopVideoCapture = _lib.get("ASIStopVideoCapture", "cdecl")
    ASIStopVideoCapture.argtypes = [c_int]
    ASIStopVideoCapture.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 666
for _lib in _libs.values():
    if not _lib.has("ASIGetVideoData", "cdecl"):
        continue
    ASIGetVideoData = _lib.get("ASIGetVideoData", "cdecl")
    ASIGetVideoData.argtypes = [c_int, POINTER(c_ubyte), c_long, c_int]
    ASIGetVideoData.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 698
for _lib in _libs.values():
    if not _lib.has("ASIGetVideoDataGPS", "cdecl"):
        continue
    ASIGetVideoDataGPS = _lib.get("ASIGetVideoDataGPS", "cdecl")
    ASIGetVideoDataGPS.argtypes = [c_int, POINTER(c_ubyte), c_long, c_int, POINTER(ASI_GPS_DATA)]
    ASIGetVideoDataGPS.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 716
for _lib in _libs.values():
    if not _lib.has("ASIPulseGuideOn", "cdecl"):
        continue
    ASIPulseGuideOn = _lib.get("ASIPulseGuideOn", "cdecl")
    ASIPulseGuideOn.argtypes = [c_int, c_int]
    ASIPulseGuideOn.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 733
for _lib in _libs.values():
    if not _lib.has("ASIPulseGuideOff", "cdecl"):
        continue
    ASIPulseGuideOff = _lib.get("ASIPulseGuideOff", "cdecl")
    ASIPulseGuideOff.argtypes = [c_int, c_int]
    ASIPulseGuideOff.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 752
for _lib in _libs.values():
    if not _lib.has("ASIStartExposure", "cdecl"):
        continue
    ASIStartExposure = _lib.get("ASIStartExposure", "cdecl")
    ASIStartExposure.argtypes = [c_int, c_int]
    ASIStartExposure.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 769
for _lib in _libs.values():
    if not _lib.has("ASIStopExposure", "cdecl"):
        continue
    ASIStopExposure = _lib.get("ASIStopExposure", "cdecl")
    ASIStopExposure.argtypes = [c_int]
    ASIStopExposure.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 789
for _lib in _libs.values():
    if not _lib.has("ASIGetExpStatus", "cdecl"):
        continue
    ASIGetExpStatus = _lib.get("ASIGetExpStatus", "cdecl")
    ASIGetExpStatus.argtypes = [c_int, POINTER(ASI_EXPOSURE_STATUS)]
    ASIGetExpStatus.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 813
for _lib in _libs.values():
    if not _lib.has("ASIGetDataAfterExp", "cdecl"):
        continue
    ASIGetDataAfterExp = _lib.get("ASIGetDataAfterExp", "cdecl")
    ASIGetDataAfterExp.argtypes = [c_int, POINTER(c_ubyte), c_long]
    ASIGetDataAfterExp.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 840
for _lib in _libs.values():
    if not _lib.has("ASIGetDataAfterExpGPS", "cdecl"):
        continue
    ASIGetDataAfterExpGPS = _lib.get("ASIGetDataAfterExpGPS", "cdecl")
    ASIGetDataAfterExpGPS.argtypes = [c_int, POINTER(c_ubyte), c_long, POINTER(ASI_GPS_DATA)]
    ASIGetDataAfterExpGPS.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 855
for _lib in _libs.values():
    if not _lib.has("ASIGetID", "cdecl"):
        continue
    ASIGetID = _lib.get("ASIGetID", "cdecl")
    ASIGetID.argtypes = [c_int, POINTER(ASI_ID)]
    ASIGetID.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 870
for _lib in _libs.values():
    if not _lib.has("ASISetID", "cdecl"):
        continue
    ASISetID = _lib.get("ASISetID", "cdecl")
    ASISetID.argtypes = [c_int, ASI_ID]
    ASISetID.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 886
for _lib in _libs.values():
    if not _lib.has("ASIGetGainOffset", "cdecl"):
        continue
    ASIGetGainOffset = _lib.get("ASIGetGainOffset", "cdecl")
    ASIGetGainOffset.argtypes = [c_int, POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_int)]
    ASIGetGainOffset.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 903
for _lib in _libs.values():
    if not _lib.has("ASIGetLMHGainOffset", "cdecl"):
        continue
    ASIGetLMHGainOffset = _lib.get("ASIGetLMHGainOffset", "cdecl")
    ASIGetLMHGainOffset.argtypes = [c_int, POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_int)]
    ASIGetLMHGainOffset.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 909
for _lib in _libs.values():
    if not _lib.has("ASIGetSDKVersion", "cdecl"):
        continue
    ASIGetSDKVersion = _lib.get("ASIGetSDKVersion", "cdecl")
    ASIGetSDKVersion.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        ASIGetSDKVersion.restype = ReturnString
    else:
        ASIGetSDKVersion.restype = String
        ASIGetSDKVersion.errcheck = ReturnString
    break

# /usr/include/libasi/ASICamera2.h: 923
for _lib in _libs.values():
    if not _lib.has("ASIGetCameraSupportMode", "cdecl"):
        continue
    ASIGetCameraSupportMode = _lib.get("ASIGetCameraSupportMode", "cdecl")
    ASIGetCameraSupportMode.argtypes = [c_int, POINTER(ASI_SUPPORTED_MODE)]
    ASIGetCameraSupportMode.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 937
for _lib in _libs.values():
    if not _lib.has("ASIGetCameraMode", "cdecl"):
        continue
    ASIGetCameraMode = _lib.get("ASIGetCameraMode", "cdecl")
    ASIGetCameraMode.argtypes = [c_int, POINTER(ASI_CAMERA_MODE)]
    ASIGetCameraMode.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 952
for _lib in _libs.values():
    if not _lib.has("ASISetCameraMode", "cdecl"):
        continue
    ASISetCameraMode = _lib.get("ASISetCameraMode", "cdecl")
    ASISetCameraMode.argtypes = [c_int, ASI_CAMERA_MODE]
    ASISetCameraMode.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 968
for _lib in _libs.values():
    if not _lib.has("ASISendSoftTrigger", "cdecl"):
        continue
    ASISendSoftTrigger = _lib.get("ASISendSoftTrigger", "cdecl")
    ASISendSoftTrigger.argtypes = [c_int, c_int]
    ASISendSoftTrigger.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 983
for _lib in _libs.values():
    if not _lib.has("ASIGetSerialNumber", "cdecl"):
        continue
    ASIGetSerialNumber = _lib.get("ASIGetSerialNumber", "cdecl")
    ASIGetSerialNumber.argtypes = [c_int, POINTER(ASI_SN)]
    ASIGetSerialNumber.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 1005
for _lib in _libs.values():
    if not _lib.has("ASISetTriggerOutputIOConf", "cdecl"):
        continue
    ASISetTriggerOutputIOConf = _lib.get("ASISetTriggerOutputIOConf", "cdecl")
    ASISetTriggerOutputIOConf.argtypes = [c_int, ASI_TRIG_OUTPUT_PIN, c_int, c_long, c_long]
    ASISetTriggerOutputIOConf.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 1024
for _lib in _libs.values():
    if not _lib.has("ASIGetTriggerOutputIOConf", "cdecl"):
        continue
    ASIGetTriggerOutputIOConf = _lib.get("ASIGetTriggerOutputIOConf", "cdecl")
    ASIGetTriggerOutputIOConf.argtypes = [c_int, ASI_TRIG_OUTPUT_PIN, POINTER(c_int), POINTER(c_long), POINTER(c_long)]
    ASIGetTriggerOutputIOConf.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 1043
for _lib in _libs.values():
    if not _lib.has("ASIGPSGetData", "cdecl"):
        continue
    ASIGPSGetData = _lib.get("ASIGPSGetData", "cdecl")
    ASIGPSGetData.argtypes = [c_int, POINTER(ASI_GPS_DATA), POINTER(ASI_GPS_DATA)]
    ASIGPSGetData.restype = c_int
    break

# /usr/include/libasi/ASICamera2.h: 42
try:
    ASICAMERA_ID_MAX = 256
except:
    pass

# /usr/include/libasi/ASICamera2.h: 152
try:
    ASI_BRIGHTNESS = ASI_OFFSET
except:
    pass

# /usr/include/libasi/ASICamera2.h: 153
try:
    ASI_AUTO_MAX_BRIGHTNESS = ASI_AUTO_TARGET_BRIGHTNESS
except:
    pass

ASI_CONTROL_TYPE = c_int# /usr/include/libasi/ASICamera2.h: 240

ASI_BOOL = c_int# /usr/include/libasi/ASICamera2.h: 241

ASI_ERROR_CODE = c_int# /usr/include/libasi/ASICamera2.h: 242

ASI_FLIP_STATUS = c_int# /usr/include/libasi/ASICamera2.h: 243

ASI_IMG_TYPE = c_int# /usr/include/libasi/ASICamera2.h: 244

ASI_GUIDE_DIRECTION = c_int# /usr/include/libasi/ASICamera2.h: 245

ASI_BAYER_PATTERN = c_int# /usr/include/libasi/ASICamera2.h: 246

_ASI_CAMERA_INFO = struct__ASI_CAMERA_INFO# /usr/include/libasi/ASICamera2.h: 150

_ASI_CONTROL_CAPS = struct__ASI_CONTROL_CAPS# /usr/include/libasi/ASICamera2.h: 198

_ASI_ID = struct__ASI_ID# /usr/include/libasi/ASICamera2.h: 210

_ASI_SUPPORTED_MODE = struct__ASI_SUPPORTED_MODE# /usr/include/libasi/ASICamera2.h: 216

_ASI_DATE_TIME = struct__ASI_DATE_TIME# /usr/include/libasi/ASICamera2.h: 228

_ASI_GPS_DATA = struct__ASI_GPS_DATA# /usr/include/libasi/ASICamera2.h: 237

# No inserted files

# No prefix-stripping

