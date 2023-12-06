#include "dso.hpp"

#include <stdexcept>
#include <sstream>

DsoObject::DsoObject(const std::string &name)
{
    if (name.empty())
    {
        throw std::invalid_argument("Name parameter cannot be empty.");
    }

    m_id = 0;
    m_name = name;
    m_type = "Unknown";
    m_ra = 0.0;
    m_dec = 0.0;
    m_const = "Unknown";
    m_notngc = "Unknown";
    m_majax = 0.0;
    m_minax = 0.0;
    m_pa = 0;
    m_bmag = 0.0;
    m_vmag = 0.0;
    m_jmag = 0.0;
    m_hmag = 0.0;
    m_kmag = 0.0;
    m_sbrightn = 0.0;
    m_parallax = 0.0;
    m_pmra = 0.0;
    m_pmdec = 0.0;
    m_radvel = 0.0;
    m_redshift = 0.0;
    m_cstarumag = 0.0;
    m_cstarbmag = 0.0;
    m_cstarvmag = 0.0;

    std::string catalog, objectname;
    std::string tables = "objects JOIN objTypes ON objects.type = objTypes.type ";
    tables += "JOIN objIdentifiers ON objects.name = objIdentifiers.name";
    recognize_name(name, catalog, objectname);
    std::stringstream params;

    if (catalog == "Messier")
    {
        params << "messier=\"" << objectname << "\"";
    }
    else
    {
        params << "objIdentifiers.identifier=\"" << objectname << "\"";
    }

    // Perform query and retrieve object data
    std::vector<std::string> objectData = _queryFetchOne(catalog, tables, params.str());

    if (objectData.empty())
    {
        throw std::runtime_error("Object not found: " + objectname);
    }

    // If object is a duplicate and returndup is false, get the main object data
    if (objectData[2] == "Dup" && !returndup)
    {
        if (!objectData[26].empty())
        {
            objectname = "NGC" + objectData[26];
        }
        else
        {
            objectname = "IC" + objectData[27];
        }
        objectData = _queryFetchOne(catalog, tables, params.str());
    }

    // Assign object properties
    _id = std::stoi(objectData[0]);
    _name = objectData[1];
    _type = objectData[3];
    _ra = std::stod(objectData[4]);
    _dec = std::stod(objectData[5]);
    _const = objectData[6];
    _notngc = objectData[33];

    // Assign optional properties
    if (!objectData[7].empty())
    {
        _majax = std::stod(objectData[7]);
    }
    if (!objectData[8].empty())
    {
        _minax = std::stod(objectData[8]);
    }
    if (!objectData[9].empty())
    {
        _pa = std::stoi(objectData[9]);
    }
    if (!objectData[10].empty())
    {
        _bmag = std::stod(objectData[10]);
    }
    if (!objectData[11].empty())
    {
        _vmag = std::stod(objectData[11]);
    }
    if (!objectData[12].empty())
    {
        _jmag = std::stod(objectData[12]);
    }
    if (!objectData[13].empty())
    {
        _hmag = std::stod(objectData[13]);
    }
    if (!objectData[14].empty())
    {
        _kmag = std::stod(objectData[14]);
    }
    if (!objectData[15].empty())
    {
        _sbrightn = std::stod(objectData[15]);
    }
    _hubble = objectData[16];
    if (!objectData[17].empty())
    {
        _parallax = std::stod(objectData[17]);
    }
    if (!objectData[18].empty())
    {
        _pmra = std::stod(objectData[18]);
    }
    if (!objectData[19].empty())
    {
        _pmdec = std::stod(objectData[19]);
    }
    if (!objectData[20].empty())
    {
        _radvel = std::stod(objectData[20]);
    }
    if (!objectData[21].empty())
    {
        _redshift = std::stod(objectData[21]);
    }
    if (!objectData[22].empty())
    {
        _cstarumag = std::stod(objectData[22]);
    }
    if (!objectData[23].empty())
    {
        _cstarbmag = std::stod(objectData[23]);
    }
    if (!objectData[24].empty())
    {
        _cstarvmag = std::stod(objectData[24]);
    }
    _messier = objectData[25];
    _ngc = objectData[26];
    _ic = objectData[27];
    _cstarnames = objectData[28];
    _identifiers = objectData[29];
    _commonnames = objectData[30];
    _nednotes = objectData[31];
    _ongcnotes = objectData[32];
}

std::string Dso::to_string() const
{
    return _name + ", " + _type + " in " + _const;
}

std::string Dso::get_constellation() const
{
    return _const;
}

std::vector<double> Dso::get_coords() const
{
    std::vector<double> coords;
    coords.push_back(std::trunc(rad2deg(_ra) / 15));
    double ms = (rad2deg(_ra) / 15 - coords[0]) * 60;
    coords.push_back(std::trunc(ms));
    coords.push_back((ms - coords[1]) * 60);

    double dec = std::trunc(rad2deg(std::abs(_dec)));
    ms = (rad2deg(std::abs(_dec)) - dec) * 60;
    coords.push_back(dec * (_dec < 0 ? -1 : 1));
    coords.push_back(std::trunc(ms));
    coords.push_back((ms - coords[4]) * 60);

    return coords;
}

std::string Dso::get_dec() const
{
    std::vector<double> coords = get_coords();
    return std::to_string(coords[3]) + ":" + std::to_string(coords[4]) + ":" + std::to_string(coords[5]);
}

std::vector<double> Dso::get_dimensions() const
{
    return {_majax, _minax, static_cast<double>(_pa)};
}

std::string Dso::get_hubble() const
{
    return _hubble;
}

int Dso::get_id() const
{
    return _id;
}

std::tuple<std::string, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>> Dso::get_identifiers() const
{
    std::string messier = (!_messier.empty()) ? "M" + _messier : "";
    std::vector<std::string> ngc, ic, commonNames, other;

    if (!_ngc.empty())
    {
        for (const auto &number : split(_ngc, ','))
        {
            ngc.push_back("NGC" + number);
        }
    }

    if (!_ic.empty())
    {
        for (const auto &number : split(_ic, ','))
        {
            ic.push_back("IC" + number);
        }
    }

    if (!_commonnames.empty())
    {
        commonNames = split(_commonnames, ',');
    }

    if (!_identifiers.empty())
    {
        other = split(_identifiers, ',');
    }

    return std::make_tuple(messier, ngc, ic, commonNames, other);
}