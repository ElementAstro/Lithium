/*
 * StarDto.hpp
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

Date: 2023-7-25

Description: Star Data Transform Object

**************************************************/

#ifndef STARDTO_HPP
#define STARDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class StarDto : public oatpp::DTO
{
    DTO_INIT(StarDto, DTO)

    DTO_FIELD_INFO(id)
    {
        info->description = "ID of the star";
    }
    DTO_FIELD(String, id);

    DTO_FIELD_INFO(name)
    {
        info->description = "Name of the star";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(type)
    {
        info->description = "Type of the star";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(ra)
    {
        info->description = "Right Ascension of the star";
    }
    DTO_FIELD(String, ra);

    DTO_FIELD_INFO(dec)
    {
        info->description = "Declination of the star";
    }
    DTO_FIELD(String, dec);

    DTO_FIELD_INFO(consts)
    {
        info->description = "Constellation of the star";
    }
    DTO_FIELD(String, consts);

    // These properties may be empty

    DTO_FIELD_INFO(majax)
    {
        info->description = "Major axis of the star";
    }
    DTO_FIELD(String, majax);

    DTO_FIELD_INFO(minax)
    {
        info->description = "Minor axis of the star";
    }
    DTO_FIELD(String, minax);

    DTO_FIELD_INFO(pa)
    {
        info->description = "Position angle of the star";
    }
    DTO_FIELD(String, pa);

    DTO_FIELD_INFO(bmag)
    {
        info->description = "B magnitude of the star";
    }
    DTO_FIELD(String, bmag);

    DTO_FIELD_INFO(vmag)
    {
        info->description = "V magnitude of the star";
    }
    DTO_FIELD(String, vmag);

    DTO_FIELD_INFO(jmag)
    {
        info->description = "J magnitude of the star";
    }
    DTO_FIELD(String, jmag);

    DTO_FIELD_INFO(hmag)
    {
        info->description = "H magnitude of the star";
    }
    DTO_FIELD(String, hmag);

    DTO_FIELD_INFO(kmag)
    {
        info->description = "K magnitude of the star";
    }
    DTO_FIELD(String, kmag);

    DTO_FIELD_INFO(sbrightn)
    {
        info->description = "Surface brightness of the star";
    }
    DTO_FIELD(String, sbrightn);

    DTO_FIELD_INFO(hubble)
    {
        info->description = "Hubble classification of the star";
    }
    DTO_FIELD(String, hubble);

    DTO_FIELD_INFO(parallax)
    {
        info->description = "Parallax of the star";
    }
    DTO_FIELD(String, parallax);

    DTO_FIELD_INFO(pmra)
    {
        info->description = "Proper motion in Right Ascension of the star";
    }
    DTO_FIELD(String, pmra);

    DTO_FIELD_INFO(pmdec)
    {
        info->description = "Proper motion in Declination of the star";
    }
    DTO_FIELD(String, pmdec);

    DTO_FIELD_INFO(radvel)
    {
        info->description = "Radial velocity of the star";
    }
    DTO_FIELD(String, radvel);

    DTO_FIELD_INFO(redshift)
    {
        info->description = "Redshift of the star";
    }
    DTO_FIELD(String, redshift);

    DTO_FIELD_INFO(cstarumag)
    {
        info->description = "C-star U magnitude of the star";
    }
    DTO_FIELD(String, cstarumag);

    DTO_FIELD_INFO(cstarbmag)
    {
        info->description = "C-star B magnitude of the star";
    }
    DTO_FIELD(String, cstarbmag);

    DTO_FIELD_INFO(cstarvmag)
    {
        info->description = "C-star V magnitude of the star";
    }
    DTO_FIELD(String, cstarvmag);

    DTO_FIELD_INFO(messier)
    {
        info->description = "Messier number of the star";
    }
    DTO_FIELD(String, messier);

    DTO_FIELD_INFO(ngc)
    {
        info->description = "NGC number of the star";
    }
    DTO_FIELD(String, ngc);

    DTO_FIELD_INFO(ic)
    {
        info->description = "IC number of the star";
    }
    DTO_FIELD(String, ic);

    DTO_FIELD_INFO(cstarnames)
    {
        info->description = "C-star names of the star";
    }
    DTO_FIELD(String, cstarnames);

    DTO_FIELD_INFO(identifiers)
    {
        info->description = "Identifiers of the star";
    }
    DTO_FIELD(String, identifiers);

    DTO_FIELD_INFO(commonnames)
    {
        info->description = "Common names of the star";
    }
    DTO_FIELD(String, commonnames);

    DTO_FIELD_INFO(nednotes)
    {
        info->description = "Notes from NED (NASA/IPAC Extragalactic Database)";
    }
    DTO_FIELD(String, nednotes);

    DTO_FIELD_INFO(ongcnotes)
    {
        info->description = "Notes from ONGC (Observatoire de la Côte d'Azur)";
    }
    DTO_FIELD(String, ongcnotes);
};

#include OATPP_CODEGEN_END(DTO)

#endif // STARDTO_HPP
