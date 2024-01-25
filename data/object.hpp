#pragma once

#include <string>

#include "atom/type/json.hpp"
using json = nlohmann::json;

struct Object
{
    std::string name;
    std::string type;
    double ra;
    double dec;
    std::string constellation;
    double majorAxis;
    double minorAxis;
    int positionAngle;
    double bMagnitude;
    double vMagnitude;
    double jMagnitude;
    double hMagnitude;
    double kMagnitude;
    double surfaceBrightness;
    std::string hubbleType;
    double parallax;
    double properMotionRA;
    double properMotionDec;
    int radialVelocity;
    double redshift;
    double cstarUMagnitude;
    double cstarBMagnitude;
    double cstarVMagnitude;
    std::string messier;
    std::string ngc;
    std::string ic;
    std::string cstarNames;
    std::string identifiers;
    std::string commonNames;
    std::string nedNotes;
    std::string ongcNotes;
    bool notNGC;

    json toJson() const
    {
        json j;
        j["name"] = name;
        j["type"] = type;
        j["ra"] = ra;
        j["dec"] = dec;
        j["constellation"] = constellation;
        j["majorAxis"] = majorAxis;
        j["minorAxis"] = minorAxis;
        j["positionAngle"] = positionAngle;
        j["bMagnitude"] = bMagnitude;
        j["vMagnitude"] = vMagnitude;
        j["jMagnitude"] = jMagnitude;
        j["hMagnitude"] = hMagnitude;
        j["kMagnitude"] = kMagnitude;
        j["surfaceBrightness"] = surfaceBrightness;
        j["hubbleType"] = hubbleType;
        j["parallax"] = parallax;
        j["properMotionRA"] = properMotionRA;
        j["properMotionDec"] = properMotionDec;
        j["radialVelocity"] = radialVelocity;
        j["redshift"] = redshift;
        j["cstarUMagnitude"] = cstarUMagnitude;
        j["cstarBMagnitude"] = cstarBMagnitude;
        j["cstarVMagnitude"] = cstarVMagnitude;
        j["messier"] = messier;
        j["ngc"] = ngc;
        j["ic"] = ic;
        j["cstarNames"] = cstarNames;
        j["identifiers"] = identifiers;
        j["commonNames"] = commonNames;
        j["nedNotes"] = nedNotes;
        j["ongcNotes"] = ongcNotes;
        j["notNGC"] = notNGC;
        return j;
    }

    static Object fromJson(const json &j)
    {
        Object obj;
        obj.name = j["name"];
        obj.type = j["type"];
        obj.ra = j["ra"];
        obj.dec = j["dec"];
        obj.constellation = j["constellation"];
        obj.majorAxis = j["majorAxis"];
        obj.minorAxis = j["minorAxis"];
        obj.positionAngle = j["positionAngle"];
        obj.bMagnitude = j["bMagnitude"];
        obj.vMagnitude = j["vMagnitude"];
        obj.jMagnitude = j["jMagnitude"];
        obj.hMagnitude = j["hMagnitude"];
        obj.kMagnitude = j["kMagnitude"];
        obj.surfaceBrightness = j["surfaceBrightness"];
        obj.hubbleType = j["hubbleType"];
        obj.parallax = j["parallax"];
        obj.properMotionRA = j["properMotionRA"];
        obj.properMotionDec = j["properMotionDec"];
        obj.radialVelocity = j["radialVelocity"];
        obj.redshift = j["redshift"];
        obj.cstarUMagnitude = j["cstarUMagnitude"];
        obj.cstarBMagnitude = j["cstarBMagnitude"];
        obj.cstarVMagnitude = j["cstarVMagnitude"];
        obj.messier = j["messier"];
        obj.ngc = j["ngc"];
        obj.ic = j["ic"];
        obj.cstarNames = j["cstarNames"];
        obj.identifiers = j["identifiers"];
        obj.commonNames = j["commonNames"];
        obj.nedNotes = j["nedNotes"];
        obj.ongcNotes = j["ongcNotes"];
        obj.notNGC = j["notNGC"];
        return obj;
    }
};