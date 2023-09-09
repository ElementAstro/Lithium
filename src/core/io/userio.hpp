/*
    Copyright (C) 2021 by Pawel Soja <kernel32.pl@gmail.com>
                  2022 by Ludovic Pollet

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string_view>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct userio
    {
        ssize_t (*write)(void *user, const void *ptr, size_t count);
        int (*vprintf)(void *user, const char *format, va_list arg);

        // join the given shared buffer as ancillary data. xml must be at least one char - optional
        void (*joinbuff)(void *user, const char *xml, void *buffer, size_t bloblen);
    } userio;

    const struct userio *userio_file();

    ssize_t userio_printf(const struct userio *io, void *user, const char *format, ...);
    ssize_t userio_vprintf(const struct userio *io, void *user, const char *format, va_list arg);

    ssize_t userio_write(const struct userio *io, void *user, const void *ptr, size_t count);

    ssize_t userio_putc(const struct userio *io, void *user, int ch);

    // extras
    ssize_t userio_prints(const struct userio *io, void *user, std::string_view str);
    size_t userio_xml_escape(const struct userio *io, void *user, std::string_view src);
    void userio_xmlv1(const userio *io, void *user);

    // JSON support
    ssize_t userio_json_write_string(const struct userio *io, void *user, std::string_view str);
    ssize_t userio_json_write_number(const struct userio *io, void *user, double number);
    ssize_t userio_json_write_boolean(const struct userio *io, void *user, bool value);
    ssize_t userio_json_write_null(const struct userio *io, void *user);

#ifdef __cplusplus
}
#endif