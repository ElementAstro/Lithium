/*
 * macro.hpp
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

Description: HYDROGEN Macro (Copy from HYDROGEN)

**************************************************/

#pragma once

#ifndef HYDROGEN_UNUSED
#define HYDROGEN_UNUSED(x) (void)x
#endif

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#ifndef HYDROGEN_HAS_CPP_ATTRIBUTE
#define HYDROGEN_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#endif

#ifndef HYDROGEN_HAS_ATTRIBUTE
#define HYDROGEN_HAS_ATTRIBUTE(x) __has_attribute(x)
#endif

#ifndef HYDROGEN_FALLTHROUGH
#if defined(__cpp_attributes) && __cpp_attributes >= 201603
#define HYDROGEN_FALLTHROUGH [[fallthrough]]
#elif defined(__GNUC__) && (__GNUC__ > 7 || (__GNUC__ == 7 && __GNUC_MINOR__ >= 1))
#define HYDROGEN_FALLTHROUGH __attribute__((fallthrough))
#else
#define HYDROGEN_FALLTHROUGH \
    do                      \
    {                       \
    } while (0)
#endif
#endif

#if defined(__cplusplus)

#include <memory>

template <typename T>
static inline std::shared_ptr<T> make_shared_weak(T *object)
{
    return std::shared_ptr<T>(object, [](T *) {});
}

template <typename T>
static inline T *getPtrHelper(T *ptr)
{
    return ptr;
}

template <typename Wrapper>
static inline typename Wrapper::element_type *getPtrHelper(const Wrapper &p)
{
    return p.get();
}

#define DECLARE_PRIVATE(Class)                                                                                                  \
    inline Class##Private *d_func() { return reinterpret_cast<Class##Private *>(getPtrHelper(this->d_ptr)); }                   \
    inline const Class##Private *d_func() const { return reinterpret_cast<const Class##Private *>(getPtrHelper(this->d_ptr)); } \
    friend Class##Private;

#define DECLARE_PRIVATE_D(Dptr, Class)                                                                                   \
    inline Class##Private *d_func() { return reinterpret_cast<Class##Private *>(getPtrHelper(Dptr)); }                   \
    inline const Class##Private *d_func() const { return reinterpret_cast<const Class##Private *>(getPtrHelper(Dptr)); } \
    friend Class##Private;

#define D_PTR(Class) Class##Private *const d = d_func()

#endif

#ifndef ATTRIBUTE_FORMAT_PRINTF
#if defined(__GNUC__) || defined(__clang__)
#define ATTRIBUTE_FORMAT_PRINTF(A, B) __attribute__((format(printf, (A), (B))))
#else
#define ATTRIBUTE_FORMAT_PRINTF(A, B)
#endif
#endif

#ifdef SWIG
#define HYDROGEN_DEPRECATED(message)
#elif __cpp_attributes >= 201309L
#define HYDROGEN_DEPRECATED(message) [[deprecated(message)]]
#else
#define HYDROGEN_DEPRECATED(message) [[deprecated]]
#endif
