/*
 * hydrogendriverio.h
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-5

Description: Hydrogen Driver

**************************************************/

/* A driverio struct is valid only for sending one xml message */
typedef struct driverio
{
    struct userio userio;
    void *user;
    void **joins;
    size_t *joinSizes;
    int joinCount;
    int locked;
    char *outBuff;
    unsigned int outPos;
} driverio;

void driverio_init(driverio *dio);
void driverio_finish(driverio *dio);
