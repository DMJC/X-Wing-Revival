#pragma once
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <cmath>

enum class Faction {
    Rebel = 0, Empire, DefectEmpire, HostileRebel, Neutral, Nature, Other
};

enum class ShipSize {
    Fighter = 0,  // 16px icon
    Medium,       // 32px icon
    Capital       // 64px icon
};

enum class ObjectType {
    Ship = 0, NavPoint, AsteroidField
};

struct ShipTypeInfo {
    std::string shortName;
    std::string displayName;
    ShipSize    size;
};

static const std::vector<ShipTypeInfo> SHIP_TYPES = {
    {"X/W",   "X-Wing",                   ShipSize::Fighter},
    {"Y/W",   "Y-Wing",                   ShipSize::Fighter},
    {"A/W",   "A-Wing",                   ShipSize::Fighter},
    {"B/W",   "B-Wing",                   ShipSize::Fighter},
    {"T/F",   "TIE Fighter",              ShipSize::Fighter},
    {"T/B",   "TIE Bomber",               ShipSize::Fighter},
    {"T/A",   "TIE Advanced",             ShipSize::Fighter},
    {"T/I",   "TIE Interceptor",          ShipSize::Fighter},
    {"T/D",   "TIE Defender",             ShipSize::Fighter},
    {"Z-95",  "Z-95 Headhunter",          ShipSize::Fighter},
    {"GUN",   "Gunboat",                  ShipSize::Medium},
    {"SHU",   "Lambda Shuttle",           ShipSize::Medium},
    {"YT1300","YT-1300 Freighter",        ShipSize::Medium},
    {"FRT",   "Freighter",                ShipSize::Medium},
    {"COR",   "Corvette (CR90)",          ShipSize::Medium},
    {"FRG",   "Nebulon-B Frigate",        ShipSize::Medium},
    {"ISD",   "Imperial Star Destroyer",  ShipSize::Capital},
    {"VSD",   "Victory Star Destroyer",   ShipSize::Capital},
    {"MC80",  "Mon Cal Cruiser",          ShipSize::Capital},
    {"MC40",  "MC40 Light Cruiser",       ShipSize::Capital},
    {"DRD",   "Dreadnaught",              ShipSize::Capital},
    {"DSII",  "Death Star II",            ShipSize::Capital},
};

static const std::vector<std::string> BACKGROUNDS = {
    "stars", "nebula", "hoth", "endor", "bespin", "tatooine", "yavin", "space"
};

static const std::vector<std::string> GAMETYPES = {
    "team_elim", "ffa_elim", "team_dm", "ffa_dm",
    "hunt", "fleet", "race", "objectives"
};

static const std::vector<std::string> FACTION_NAMES = {
    "Rebel", "Empire", "Defect Empire", "Hostile Rebel", "Neutral", "Nature", "Other"
};

inline std::string factionTeamString(Faction f) {
    switch (f) {
        case Faction::Rebel:        return "rebel";
        case Faction::DefectEmpire: return "rebel";
        case Faction::Empire:       return "empire";
        case Faction::HostileRebel: return "empire";
        default:                    return "";
    }
}

// Returns 0=friendly(green), 1=hostile(red), 2=neutral(grey)
inline int colorRole(ObjectType t, Faction f, const std::string& playerTeam) {
    if (t == ObjectType::NavPoint || t == ObjectType::AsteroidField) return 2;
    switch (f) {
        case Faction::Rebel:        return (playerTeam == "rebel")   ? 0 : 1;
        case Faction::DefectEmpire: return (playerTeam == "rebel")   ? 0 : 1;
        case Faction::Empire:       return (playerTeam == "empire")  ? 0 : 1;
        case Faction::HostileRebel: return (playerTeam == "empire")  ? 0 : 1;
        default:                    return 2;
    }
}

inline ShipSize shipSizeFor(const std::string& sc) {
    for (const auto& s : SHIP_TYPES)
        if (s.shortName == sc) return s.size;
    return ShipSize::Fighter;
}

struct MissionObject {
    ObjectType  objType  = ObjectType::Ship;
    std::string shipClass= "X/W";
    ShipSize    size     = ShipSize::Fighter;
    Faction     faction  = Faction::Rebel;
    std::string name;
    double x = 0, y = 0, z = 0;

    // NavPoint
    bool        navVisible     = true;
    int         navTargetSystem= 1;
    std::string navVarName;

    // AsteroidField
    int asteroidCount = 16;

    bool isLandingShip = false;
    bool selected = false;
};

struct MissionSystem {
    int         number     = 1;
    std::string background = "stars";
    double spawnX = 0, spawnY = 0, spawnZ = 0;
    std::vector<MissionObject> objects;

    std::string displayName() const {
        return "System " + std::to_string(number);
    }
};

struct MissionData {
    // Global properties
    std::string missionName  = "New Mission";
    std::string missionDesc;
    std::string gametype     = "team_elim";
    std::string playerTeam   = "rebel";
    std::string playerShip   = "X/W";
    bool allowShipChange     = false;

    std::vector<MissionSystem> systems;

    std::string briefingText;
    std::string winText;
    std::string lossText;

    MissionData() {
        systems.emplace_back();
        systems[0].number = 1;
    }

    bool isMultiSystem() const { return systems.size() > 1; }

    std::string exportText() const {
        std::ostringstream o;
        if (isMultiSystem()) {
            o << "multisystem\n";
            o << "systems " << systems.size() << "\n";
        }
        o << "mission_name " << missionName << "\n";
        if (!missionDesc.empty())
            o << "mission_desc " << missionDesc << "\n";
        o << "gametype " << gametype << "\n";
        o << "player_team " << playerTeam << "\n";
        o << "player_ship " << playerShip << "\n";
        if (allowShipChange)
            o << "allow_ship_change true\n";
        o << "\n";

        for (const auto& sys : systems) {
            if (isMultiSystem())
                o << "system " << sys.number << "\n";
            o << "bg " << sys.background << "\n";
            o << "player_spawn " << sys.spawnX << " " << sys.spawnY << " " << sys.spawnZ << "\n";

            for (const auto& obj : sys.objects) {
                if (obj.objType == ObjectType::AsteroidField) {
                    o << "asteroids " << obj.asteroidCount << "\n";
                } else if (obj.objType == ObjectType::NavPoint) {
                    o << "navpoint \"" << obj.name << "\" "
                      << obj.x << " " << obj.y << " " << obj.z << " "
                      << (obj.navVisible ? "true" : "false") << " "
                      << obj.navTargetSystem;
                    if (!obj.navVarName.empty())
                        o << " var " << obj.navVarName;
                    o << "\n";
                } else {
                    // Ship — output as a spawn action at mission start
                    std::string team = factionTeamString(obj.faction);
                    o << "spawn";
                    if (!team.empty()) o << " " << team;
                    if (obj.isLandingShip) o << " landing";
                    o << " " << obj.shipClass;
                    if (!obj.name.empty())
                        o << " \"" << obj.name << "\"";
                    o << " at " << obj.x << " " << obj.y << " " << obj.z << "\n";
                }
            }

            if (isMultiSystem())
                o << "system " << sys.number << "\n";
            o << "\n";
        }

        if (!winText.empty()) {
            o << "on victory\n";
            o << "message " << winText << "\n\n";
        }
        if (!lossText.empty()) {
            o << "on defeat\n";
            o << "message " << lossText << "\n\n";
        }

        return o.str();
    }

    bool saveToFile(const std::string& path) const {
        std::ofstream f(path);
        if (!f) return false;
        f << exportText();
        return f.good();
    }
};
