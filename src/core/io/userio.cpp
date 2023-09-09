/*
    Copyright (C) 2021 by Pawel Soja <kernel32.pl@gmail.com>

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
#include "userio.hpp"

#include <cstdio>
#include <string_view>

static ssize_t s_file_write(void *user, const void *ptr, size_t count)
{
    return std::fwrite(ptr, 1, count, reinterpret_cast<FILE *>(user));
}

static int s_file_printf(void *user, const char *format, va_list arg)
{
    return std::vfprintf(reinterpret_cast<FILE *>(user), format, arg);
}

static const struct userio s_userio_file = {
    .write = s_file_write,
    .vprintf = s_file_printf,
    .joinbuff = nullptr,
};

const struct userio *userio_file()
{
    return &s_userio_file;
}

ssize_t userio_printf(const struct userio *io, void *user, const char *format, ...)
{
    int ret;
    va_list ap;
    va_start(ap, format);
    ret = io->vprintf(user, format, ap);
    va_end(ap);
    return ret;
}

ssize_t userio_vprintf(const struct userio *io, void *user, const char *format, va_list arg)
{
    return io->vprintf(user, format, arg);
}

ssize_t userio_write(const struct userio *io, void *user, const void *ptr, size_t count)
{
    return io->write(user, ptr, count);
}

ssize_t userio_prints(const struct userio *io, void *user, std::string_view str)
{
    return io->write(user, str.data(), str.size());
}

ssize_t userio_putc(const struct userio *io, void *user, int ch)
{
    char c = ch;
    return io->write(user, &c, sizeof(c));
}

size_t userio_xml_escape(const struct userio *io, void *user, std::string_view src)
{
    size_t total = 0;
    const char *ptr = src.data();
    const char *replacement;

    for (; ptr < src.data() + src.size(); ++ptr)
    {
        switch (*ptr)
        {
        case '&':
            replacement = "&amp;";
            break;
        case '\'':
            replacement = "&apos;";
            break;
        case '"':
            replacement = "&quot;";
            break;
        case '<':
            replacement = "&lt;";
            break;
        case '>':
            replacement = "&gt;";
            break;
        default:
            replacement = nullptr;
        }

        if (replacement != nullptr)
        {
            total += userio_write(io, user, src.data(), ptr - src.data());
            src = {ptr + 1, src.size() - (ptr - src.data() + 1)};
            total += userio_write(io, user, replacement, std::strlen(replacement));
        }
    }
    total += userio_write(io, user, src.data(), ptr - src.data());
    return total;
}

void userio_xmlv1(const userio *io, void *user)
{
    userio_prints(io, user, "<?xml version='1.0'?>\n");
}

ssize_t userio_json_write_string(const struct userio *io, void *user, std::string_view str)
{
    ssize_t ret;
    ret = userio_putc(io, user, '\"');
    if (ret < 0)
        return ret;

    for (const auto c : str)
    {
        switch (c)
        {
        case '\"':
            ret = userio_prints(io, user, "\\\"");
            break;
        case '\\':
            ret = userio_prints(io, user, "\\\\");
            break;
        case '\b':
            ret = userio_prints(io, user, "\\b");
            break;
        case '\f':
            ret = userio_prints(io, user, "\\f");
            break;
        case '\n':
            ret = userio_prints(io, user, "\\n");
            break;
        case '\r':
            ret = userio_prints(io, user, "\\r");
            break;
        case '\t':
            ret = userio_prints(io, user, "\\t");
            break;
        default:
            if (c < ' ')
            {
                ret = userio_printf(io, user, "\\u%04x", c);
            }
            else
            {
                ret = userio_putc(io, user, c);
            }
            break;
        }

        if (ret < 0)
            return ret;
    }

    ret = userio_putc(io, user, '\"');
    return ret >= 0 ? ret + 2 : ret;
}

ssize_t userio_json_write_number(const struct userio *io, void *user, double number)
{
    return userio_printf(io, user, "%g", number);
}

ssize_t userio_json_write_boolean(const struct userio *io, void *user, bool value)
{
    if (value)
        return userio_prints(io, user, "true");
    else
        return userio_prints(io, user, "false");
}

ssize_t userio_json_write_null(const struct userio *io, void *user)
{
    return userio_prints(io, user, "null");
}