/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <algorithm>
#include <limits>
#include <map>
#include <sstream>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define JPS_OLD_VERSION "0.5" // this version is still supported

// precision error
#define J_EPS 0.001 // 0.001
#define J_EPS_EVENT 0.00001 // zum pruefen des aktuellen Zeitschrittes auf events
#define J_EPS_DIST 0.05 // [m]

#define J_EPS_GOAL 0.005 /// [m] Abstand zum Ziel, damit Fußgänger immer zu einem Raum gehört
#define J_TOLERANZ 0.03 /// [m] Toleranz beim erstellen der Linien
#define J_EPS_V 0.1 /// [m/s] wenn  v<EPS_V wird mit 0 gerechnet

// default final destination for the pedestrians
#define FINAL_DEST_OUT -1

// Not-a-Number (NaN)
#define J_NAN std::numeric_limits<double>::quiet_NaN()

enum AgentColorMode { BY_VELOCITY = 1, BY_ROUTER, BY_GROUP, BY_FINAL_GOAL, BY_INTERMEDIATE_GOAL };

enum LineIntersectType {
    NO_INTERSECTION = 0,
    INTERSECTION,
    OVERLAP // overlap, parallel, no intersection
};

enum GridCode { // used in floor fields
    WALL = -10,
    INSIDE = -11,
    OUTSIDE = -12,
    // instead of constants, GridCode stores UID of doors
    OPEN_CROSSING = -13,
    OPEN_TRANSITION = -14,
    CLOSED_CROSSING =
        -15, // closed crossings and transitions are marked as walls in "parseBuilding"
    CLOSED_TRANSITION = -16
};

enum FloorfieldMode {
    FF_CENTRALPOINT,
    FF_FULL_LINE,
    FF_WALL_AVOID,
    FF_HOMO_SPEED,
    FF_PED_SPEED,
    FF_ROOM_SCALE,
    FF_SUBROOM_SCALE
};

enum MagicNumbers { UNKNOWN_DISTANCE, UNKNOWN_COST, WALL_ON_COSTARRAY, TARGET_REGION };

enum SPEEDFIELD { // this enum is used as index in _speedFieldSelector
    INITIAL_SPEED = 0, // homogen speed in walkable area, nealy zero in walls
    REDU_WALL_SPEED = 1, // reduced wall speed
    PED_SPEED = 2 // standing agents reduce speed, so that jams will be considered in ff
};

enum TARGETMODE { LINESEGMENT = 0, CENTERPOINT };

enum USERMODE { DISTANCE_MEASUREMENTS_ONLY, DISTANCE_AND_DIRECTIONS_USED };

enum class OptionalOutput {
    speed = 0,
    velocity = 1,
    final_goal = 2,
    intermediate_goal = 3,
    desired_direction = 4,
    router = 6,
    group = 7,
}; // enum class

constexpr double magicnum(int i)
{
    return (i == UNKNOWN_DISTANCE)  ? -3.0 :
           (i == UNKNOWN_COST)      ? -2.0 :
           (i == WALL_ON_COSTARRAY) ? -7.0 :
           (i == TARGET_REGION)     ? 0.0 :
                                      J_NAN;
}

inline char xmltob(const char* t, char v = 0)
{
    if(t && (*t))
        return (char) atoi(t);
    return v;
}
inline int xmltoi(const char* t, int v = 0)
{
    if(t && (*t))
        return atoi(t);
    return v;
}
inline long xmltol(const char* t, long v = 0)
{
    if(t && (*t))
        return atol(t);
    return v;
}
inline double xmltof(const char* t, double v = 0.0)
{
    if(t && (*t))
        return atof(t);
    return v;
}
inline const char* xmltoa(const char* t, const char* v = "")
{
    if(t)
        return t;
    return v;
}
inline char xmltoc(const char* t, const char v = '\0')
{
    if(t && (*t))
        return *t;
    return v;
}

/**
 * @return true if the element is present in the vector
 */
template <typename A>
inline bool IsElementInVector(const std::vector<A>& vec, const A& el)
{
    typename std::vector<A>::const_iterator it;
    it = std::find(vec.begin(), vec.end(), el);
    if(it == vec.end()) {
        return false;
    } else {
        return true;
    }
}

/**
 * Implementation of a map with a default value.
 * @return the default value if the element was not found in the map
 */
template <typename K, typename V>
inline V GetWithDef(const std::map<K, V>& m, const K& key, const V& defval)
{
    typename std::map<K, V>::const_iterator it = m.find(key);
    if(it == m.end()) {
        return defval;
    } else {
        return it->second;
    }
}

inline std::string concatenate(std::string const& name, int i)
{
    std::stringstream s;
    s << name << i;
    return s.str();
}

inline void
ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace)
{
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}
