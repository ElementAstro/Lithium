/*******************************************************************************
  Copyright(c) 2011 Jasem Mutlaq. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#pragma once

/*! INDI property type */
typedef enum
{
    HYDROGEN_NUMBER, /*!< INumberVectorProperty. */
    HYDROGEN_SWITCH, /*!< ISwitchVectorProperty. */
    HYDROGEN_TEXT,   /*!< ITextVectorProperty. */
    HYDROGEN_LIGHT,  /*!< ILightVectorProperty. */
    HYDROGEN_BLOB,   /*!< IBLOBVectorProperty. */
    HYDROGEN_UNKNOWN
} HYDROGEN_PROPERTY_TYPE;

/*! INDI Equatorial Axis type */
typedef enum
{
    AXIS_RA, /*!< Right Ascension Axis. */
    AXIS_DE  /*!< Declination Axis. */
} HYDROGEN_EQ_AXIS;

/*! INDI Horizontal Axis type */
typedef enum
{
    AXIS_AZ, /*!< Azimuth Axis. */
    AXIS_ALT /*!< Altitude Axis. */
} HYDROGEN_HO_AXIS;

/*! North/South Direction type */
typedef enum
{
    DIRECTION_NORTH = 0, /*!< North direction */
    DIRECTION_SOUTH      /*!< South direction */
} HYDROGEN_DIR_NS;

/*! West/East Direction type */
typedef enum
{
    DIRECTION_WEST = 0, /*!< West direction */
    DIRECTION_EAST      /*!< East direction */
} HYDROGEN_DIR_WE;

/*! INDI Error Types */
typedef enum
{
    HYDROGEN_DEVICE_NOT_FOUND = -1,    /*!< Device not found error */
    HYDROGEN_PROPERTY_INVALID = -2,    /*!< Property invalid error */
    HYDROGEN_PROPERTY_DUPLICATED = -3, /*!< Property duplicated error */
    HYDROGEN_DISPATCH_ERROR = -4       /*!< Dispatch error */
} HYDROGEN_ERROR_TYPE;

typedef enum
{
    HYDROGEN_MONO = 0,
    HYDROGEN_BAYER_RGGB = 8,
    HYDROGEN_BAYER_GRBG = 9,
    HYDROGEN_BAYER_GBRG = 10,
    HYDROGEN_BAYER_BGGR = 11,
    HYDROGEN_BAYER_CYYM = 16,
    HYDROGEN_BAYER_YCMY = 17,
    HYDROGEN_BAYER_YMCY = 18,
    HYDROGEN_BAYER_MYYC = 19,
    HYDROGEN_RGB = 100,
    HYDROGEN_BGR = 101,
    HYDROGEN_JPG = 200,
} HYDROGEN_PIXEL_FORMAT;
