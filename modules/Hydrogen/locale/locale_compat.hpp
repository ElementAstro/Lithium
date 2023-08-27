/*
 * locale/locale_compat.hpppp
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

Date: 2023-4-9

Description: Locale Support (Copy from HYDROGEN)

**************************************************/

#pragma once

#include <locale.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // C interface
    //
    // usage:
    //
    //     locale_char_t *save = hydrogen_locale_C_numeric_push();
    //     ...
    //     hydrogen_locale_C_numeric_pop(save);
    //

#if defined(_MSC_VER)

#include <string.h>
#include <malloc.h>

    typedef wchar_t locale_char_t;
#define HYDROGEN_LOCALE(s) L"" #s

    __inline static locale_char_t *hydrogen_setlocale(int category, const locale_char_t *locale)
    {
        return _wcsdup(_wsetlocale(category, locale));
    }

    __inline static void hydrogen_restore_locale(int category, locale_char_t *prev)
    {
        _wsetlocale(category, prev);
        free(prev);
    }

#define _HYDROGEN_C_INLINE __inline

#else // _MSC_VER

typedef char locale_char_t;
#define HYDROGEN_LOCALE(s) s

inline static locale_char_t *hydrogen_setlocale(int category, const locale_char_t *locale)
{
    return setlocale(category, locale);
}

inline static void hydrogen_restore_locale(int category, locale_char_t *prev)
{
    setlocale(category, prev);
}

#define _HYDROGEN_C_INLINE inline

#endif // _MSC_VER

    _HYDROGEN_C_INLINE static locale_char_t *hydrogen_locale_C_numeric_push()
    {
        return hydrogen_setlocale(LC_NUMERIC, HYDROGEN_LOCALE("C"));
    }

    _HYDROGEN_C_INLINE static void hydrogen_locale_C_numeric_pop(locale_char_t *prev)
    {
        hydrogen_restore_locale(LC_NUMERIC, prev);
    }

#undef _HYDROGEN_C_INLINE

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// C++ interface
//
// usage:
//
//     AutoCNumeric locale; // LC_NUMERIC locale set to "C" for object scope
//     ...
//

class AutoLocale
{
    int m_category;
    locale_char_t *m_orig;

public:
    AutoLocale(int category, const locale_char_t *locale)
        : m_category(category)
    {
        m_orig = hydrogen_setlocale(category, locale);
    }

    // method Restore can be used to restore the original locale
    // before the object goes out of scope
    void Restore()
    {
        if (m_orig)
        {
            hydrogen_restore_locale(m_category, m_orig);
            m_orig = nullptr;
        }
    }

    ~AutoLocale()
    {
        Restore();
    }
};

class AutoCNumeric : public AutoLocale
{
public:
    AutoCNumeric() : AutoLocale(LC_NUMERIC, HYDROGEN_LOCALE("C")) {}
};

#endif // __cplusplus
