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
    LITHIUM_NUMBER, /*!< INumberVectorProperty. */
    LITHIUM_SWITCH, /*!< ISwitchVectorProperty. */
    LITHIUM_TEXT,   /*!< ITextVectorProperty. */
    LITHIUM_LIGHT,  /*!< ILightVectorProperty. */
    LITHIUM_BLOB,   /*!< IBLOBVectorProperty. */
    LITHIUM_UNKNOWN
} LITHIUM_PROPERTY_TYPE;

/*! INDI Equatorial Axis type */
typedef enum
{
    AXIS_RA, /*!< Right Ascension Axis. */
    AXIS_DE  /*!< Declination Axis. */
} LITHIUM_EQ_AXIS;

/*! INDI Horizontal Axis type */
typedef enum
{
    AXIS_AZ, /*!< Azimuth Axis. */
    AXIS_ALT /*!< Altitude Axis. */
} LITHIUM_HO_AXIS;

/*! North/South Direction type */
typedef enum
{
    DIRECTION_NORTH = 0, /*!< North direction */
    DIRECTION_SOUTH      /*!< South direction */
} LITHIUM_DIR_NS;

/*! West/East Direction type */
typedef enum
{
    DIRECTION_WEST = 0, /*!< West direction */
    DIRECTION_EAST      /*!< East direction */
} LITHIUM_DIR_WE;

/*! INDI Error Types */
typedef enum
{
    LITHIUM_DEVICE_NOT_FOUND = -1,    /*!< Device not found error */
    LITHIUM_PROPERTY_INVALID = -2,    /*!< Property invalid error */
    LITHIUM_PROPERTY_DUPLICATED = -3, /*!< Property duplicated error */
    LITHIUM_DISPATCH_ERROR = -4       /*!< Dispatch error */
} LITHIUM_ERROR_TYPE;

typedef enum
{
    LITHIUM_MONO = 0,
    LITHIUM_BAYER_RGGB = 8,
    LITHIUM_BAYER_GRBG = 9,
    LITHIUM_BAYER_GBRG = 10,
    LITHIUM_BAYER_BGGR = 11,
    LITHIUM_BAYER_CYYM = 16,
    LITHIUM_BAYER_YCMY = 17,
    LITHIUM_BAYER_YMCY = 18,
    LITHIUM_BAYER_MYYC = 19,
    LITHIUM_RGB = 100,
    LITHIUM_BGR = 101,
    LITHIUM_JPG = 200,
} LITHIUM_PIXEL_FORMAT;
